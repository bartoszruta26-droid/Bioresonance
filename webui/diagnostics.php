<?php
declare(strict_types=1);

/**
 * Read-only ResoNet Protocol v1 HTTP bridge.
 * Only HELLO_REQ, PING and STATUS_REQ are emitted.
 */
header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: no-store');
header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: DENY');
header('Referrer-Policy: no-referrer');

final class DiagnosticException extends RuntimeException {
    public function __construct(
        public readonly string $errorCode,
        string $message,
        public readonly bool $retryable = false
    ) {
        parent::__construct($message);
    }
}

function respond(int $status, array $body): void {
    if (($body['success'] ?? true) === false && $body['error'] instanceof DiagnosticException) {
        $error = $body['error'];
        $body['error'] = [
            'code' => $error->errorCode,
            'message' => $error->getMessage(),
            'retryable' => $error->retryable,
        ];
    } elseif (($body['success'] ?? true) === false && is_string($body['error'] ?? null)) {
        if ($status === 400) $code = 'INVALID_REQUEST';
        elseif ($status === 401) $code = 'AUTH_REQUIRED';
        elseif ($status === 405) $code = 'METHOD_NOT_ALLOWED';
        else $code = 'INTERNAL_ERROR';
        $body['error'] = ['code' => $code, 'message' => $body['error'], 'retryable' => false];
    }
    http_response_code($status);
    echo json_encode($body, JSON_UNESCAPED_UNICODE | JSON_INVALID_UTF8_SUBSTITUTE);
    exit;
}

function crc16_ccitt_false(string $data): int {
    $crc = 0xFFFF;
    $length = strlen($data);
    for ($i = 0; $i < $length; $i++) {
        $crc ^= ord($data[$i]) << 8;
        for ($bit = 0; $bit < 8; $bit++) {
            $crc = (($crc & 0x8000) !== 0)
                ? (($crc << 1) ^ 0x1021) & 0xFFFF
                : ($crc << 1) & 0xFFFF;
        }
    }
    return $crc;
}

function encode_frame(int $type, int $sequence): string {
    $header = "RN" . chr(1) . chr($type) . chr(0) . pack('n', $sequence) . pack('n', 0);
    return $header . pack('n', crc16_ccitt_false($header));
}

function read_exact($socket, int $length): string {
    $data = '';
    while (strlen($data) < $length) {
        $chunk = '';
        $received = @socket_recv($socket, $chunk, $length - strlen($data), 0);
        if ($received === false || $received === 0) {
            $error = socket_last_error($socket);
            $timeoutCodes = array_filter([
                defined('SOCKET_EAGAIN') ? SOCKET_EAGAIN : null,
                defined('SOCKET_EWOULDBLOCK') ? SOCKET_EWOULDBLOCK : null,
                defined('SOCKET_ETIMEDOUT') ? SOCKET_ETIMEDOUT : null,
            ], static function ($value) { return $value !== null; });
            if ($received === false && in_array($error, $timeoutCodes, true)) {
                throw new DiagnosticException('TRANSPORT_TIMEOUT', 'Przekroczono limit czasu odpowiedzi TCP.', true);
            }
            if ($received === 0) {
                throw new DiagnosticException('CONNECTION_CLOSED', 'Urządzenie zamknęło połączenie przed końcem odpowiedzi.', true);
            }
            throw new DiagnosticException('TRANSPORT_UNAVAILABLE', 'Nie udało się odebrać odpowiedzi TCP.', true);
        }
        $data .= $chunk;
    }
    return $data;
}

function exchange(Socket $socket, int $type, int $expectedType, int $sequence): string {
    $frame = encode_frame($type, $sequence);
    $offset = 0;
    while ($offset < strlen($frame)) {
        $sent = @socket_write($socket, substr($frame, $offset));
        if ($sent === false || $sent === 0) {
            throw new DiagnosticException('TRANSPORT_UNAVAILABLE', 'Nie udało się wysłać pełnej ramki.', true);
        }
        $offset += $sent;
    }

    $header = read_exact($socket, 9);
    if (substr($header, 0, 2) !== 'RN' || ord($header[2]) !== 1 || ord($header[4]) !== 0) {
        throw new DiagnosticException('PROTOCOL_INVALID_HEADER', 'Odpowiedź ma nieprawidłowy nagłówek protokołu v1.');
    }
    $responseType = ord($header[3]);
    $responseSequence = unpack('nvalue', substr($header, 5, 2))['value'];
    $length = unpack('nvalue', substr($header, 7, 2))['value'];
    if ($length > 192) {
        throw new DiagnosticException('PROTOCOL_INVALID_LENGTH', 'Długość odpowiedzi przekracza limit protokołu.');
    }
    if ($responseSequence !== $sequence) {
        throw new DiagnosticException('PROTOCOL_SEQUENCE_MISMATCH', 'Odpowiedź ma niezgodny numer sequence.');
    }
    $payload = read_exact($socket, $length);
    $receivedCrc = unpack('nvalue', read_exact($socket, 2))['value'];
    if ($receivedCrc !== crc16_ccitt_false($header . $payload)) {
        throw new DiagnosticException('PROTOCOL_CRC_MISMATCH', 'Suma kontrolna odpowiedzi jest nieprawidłowa.');
    }
    if ($responseType === 0x7D || $responseType === 0x7E) {
        throw new DiagnosticException('DEVICE_REJECTED', 'Urządzenie odrzuciło żądanie protokołu v1.');
    }
    if ($responseType !== $expectedType) {
        throw new DiagnosticException('PROTOCOL_UNEXPECTED_TYPE', sprintf('Nieoczekiwany typ odpowiedzi: 0x%02X', $responseType));
    }
    return $payload;
}

function is_private_ipv4(string $ip): bool {
    if (filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4) === false) return false;
    $long = ip2long($ip);
    if ($long === false) return false;
    $unsigned = (int)sprintf('%u', $long);
    $ranges = [
        [ip2long('10.0.0.0'), 8],
        [ip2long('172.16.0.0'), 12],
        [ip2long('192.168.0.0'), 16],
    ];
    foreach ($ranges as [$base, $bits]) {
        $mask = (0xFFFFFFFF << (32 - $bits)) & 0xFFFFFFFF;
        if (($unsigned & $mask) === (((int)sprintf('%u', $base)) & $mask)) return true;
    }
    return false;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    header('Allow: POST');
    respond(405, ['success' => false, 'error' => 'Dozwolona jest wyłącznie metoda POST']);
}

if (empty($_SERVER['HTTPS']) || $_SERVER['HTTPS'] === 'off') {
    respond(400, ['success' => false, 'error' => 'Panel diagnostyczny wymaga HTTPS']);
}

$expectedToken = getenv('BIORESONANCE_API_TOKEN') ?: '';
$providedToken = $_SERVER['HTTP_X_BIORESONANCE_TOKEN'] ?? '';
if (strlen($expectedToken) < 32 || !hash_equals($expectedToken, $providedToken)) {
    respond(401, ['success' => false, 'error' => 'Brak prawidłowego tokenu API']);
}

$input = json_decode(file_get_contents('php://input'), true);
if (!is_array($input)) respond(400, ['success' => false, 'error' => 'Nieprawidłowy JSON']);
$ip = $input['ip'] ?? '';
$action = $input['action'] ?? 'status';
if (!is_string($ip) || !is_private_ipv4($ip)) {
    respond(400, ['success' => false, 'error' => 'Dozwolony jest wyłącznie adres IPv4 z prywatnej sieci RFC1918']);
}
if (!in_array($action, ['status', 'ping'], true)) {
    respond(400, ['success' => false, 'error' => 'Niedozwolona akcja diagnostyczna']);
}

$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if ($socket === false) respond(502, ['success' => false, 'error' => 'Nie można utworzyć gniazda TCP']);
try {
    if (!socket_set_nonblock($socket)) {
        throw new RuntimeException('Nie można ustawić nieblokującego trybu połączenia');
    }
    $connected = @socket_connect($socket, $ip, 5001);
    if (!$connected) {
        $connectError = socket_last_error($socket);
        $pendingErrors = array_filter([
            defined('SOCKET_EINPROGRESS') ? SOCKET_EINPROGRESS : null,
            defined('SOCKET_EALREADY') ? SOCKET_EALREADY : null,
            defined('SOCKET_EWOULDBLOCK') ? SOCKET_EWOULDBLOCK : null,
            defined('SOCKET_EAGAIN') ? SOCKET_EAGAIN : null,
        ], static function ($value) { return $value !== null; });
        if (!in_array($connectError, $pendingErrors, true)) {
            throw new RuntimeException('Nie można połączyć z urządzeniem na porcie 5001');
        }
        $readSockets = [];
        $writeSockets = [$socket];
        $exceptSockets = [$socket];
        $ready = @socket_select($readSockets, $writeSockets, $exceptSockets, 2, 0);
        if ($ready === false || $ready === 0 || !empty($exceptSockets) || empty($writeSockets)) {
            throw new DiagnosticException('TRANSPORT_TIMEOUT', 'Przekroczono limit czasu połączenia TCP.', true);
        }
        $socketError = socket_get_option($socket, SOL_SOCKET, SO_ERROR);
        if ($socketError !== 0) {
            throw new RuntimeException('Połączenie TCP nie powiodło się (kod ' . $socketError . ')');
        }
    }
    if (!socket_set_block($socket)) {
        throw new RuntimeException('Nie można przywrócić blokującego trybu gniazda');
    }
    if (!socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, ['sec' => 2, 'usec' => 0]) ||
        !socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, ['sec' => 2, 'usec' => 0])) {
        throw new RuntimeException('Nie można ustawić limitów czasu gniazda');
    }
    $sequence = 1;
    $hello = exchange($socket, 0x01, 0x02, $sequence++);
    if (strlen($hello) !== 13) throw new RuntimeException('Nieprawidłowy HELLO_RESP');
    $deviceId = bin2hex(substr($hello, 0, 8));
    $firmware = [ord($hello[8]), ord($hello[9]), ord($hello[10])];
    $capabilities = unpack('nvalue', substr($hello, 11, 2))['value'];
    if ($capabilities !== 0) throw new DiagnosticException('PROTOCOL_UNSUPPORTED_CAPABILITY', 'Urządzenie zgłosiło nieobsługiwaną funkcję protokołu v1.');

    if ($action === 'ping') {
        $pong = exchange($socket, 0x03, 0x04, $sequence++);
        if ($pong !== '') throw new RuntimeException('Nieprawidłowy PONG');
        respond(200, ['success' => true, 'device_id' => $deviceId,
            'firmware' => implode('.', $firmware), 'capabilities' => $capabilities,
            'diagnostic' => 'PONG']);
    }

    $data = exchange($socket, 0x05, 0x06, $sequence++);
    if (strlen($data) !== 14) throw new RuntimeException('Nieprawidłowy STATUS_RESP');
    $temperature = unpack('nvalue', substr($data, 9, 2))['value'];
    if ($temperature >= 0x8000) $temperature -= 0x10000;
    $temperatureValue = $temperature === 0x7FFF ? null : $temperature / 100;
    $uptime = unpack('Nvalue', substr($data, 3, 4))['value'];
    $freeMemory = unpack('nvalue', substr($data, 7, 2))['value'];
    $faultFlags = unpack('nvalue', substr($data, 1, 2))['value'];
    $sensorFlags = unpack('nvalue', substr($data, 11, 2))['value'];
    $state = ord($data[0]);
    $outputFlag = ord($data[13]);
    if ($state > 5 || ($faultFlags & ~0x0003) !== 0 || !in_array($outputFlag, [0, 1], true)) {
        throw new DiagnosticException('DEVICE_STATUS_INVALID', 'Odpowiedź STATUS zawiera nieobsługiwane wartości.');
    }
    $outputInhibited = $outputFlag === 1 ? true : null;

    respond(200, ['success' => true, 'device_id' => $deviceId,
        'firmware' => implode('.', $firmware), 'capabilities' => $capabilities,
        'status' => ['state' => $state, 'fault_flags' => $faultFlags,
            'uptime_seconds' => $uptime, 'free_memory_bytes' => $freeMemory,
            'temperature_c' => $temperatureValue, 'sensor_valid_flags' => $sensorFlags,
            'output_inhibited' => $outputInhibited]]);
} catch (DiagnosticException $error) {
    respond(502, ['success' => false, 'error' => $error]);
} catch (Throwable $error) {
    respond(502, ['success' => false, 'error' => new DiagnosticException(
        'INTERNAL_ERROR', 'Wystąpił nieoczekiwany błąd serwera diagnostycznego.'
    )]);
} finally {
    socket_close($socket);
}
