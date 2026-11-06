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

function respond(int $status, array $body): void {
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
            throw new RuntimeException('Niepełna odpowiedź TCP (kod ' . $error . ')');
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
            throw new RuntimeException('Nie udało się wysłać pełnej ramki');
        }
        $offset += $sent;
    }

    $header = read_exact($socket, 9);
    if (substr($header, 0, 2) !== 'RN' || ord($header[2]) !== 1 || ord($header[4]) !== 0) {
        throw new RuntimeException('Nieprawidłowy nagłówek odpowiedzi');
    }
    $responseType = ord($header[3]);
    $responseSequence = unpack('nvalue', substr($header, 5, 2))['value'];
    $length = unpack('nvalue', substr($header, 7, 2))['value'];
    if ($length > 192 || $responseSequence !== $sequence) {
        throw new RuntimeException('Nieprawidłowa długość lub sequence odpowiedzi');
    }
    $payload = read_exact($socket, $length);
    $receivedCrc = unpack('nvalue', read_exact($socket, 2))['value'];
    if ($receivedCrc !== crc16_ccitt_false($header . $payload)) {
        throw new RuntimeException('Nieprawidłowe CRC-16/CCITT-FALSE');
    }
    if ($responseType === 0x7D || $responseType === 0x7E) {
        throw new RuntimeException('Urządzenie odrzuciło żądanie protokołu');
    }
    if ($responseType !== $expectedType) {
        throw new RuntimeException(sprintf('Nieoczekiwany typ odpowiedzi: 0x%02X', $responseType));
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
    if (!socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, ['sec' => 2, 'usec' => 0]) ||
        !socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, ['sec' => 2, 'usec' => 0])) {
        throw new RuntimeException('Nie można ustawić limitów czasu gniazda');
    }
    if (!@socket_connect($socket, $ip, 5001)) {
        throw new RuntimeException('Nie można połączyć z urządzeniem na porcie 5001');
    }
    $sequence = 1;
    $hello = exchange($socket, 0x01, 0x02, $sequence++);
    if (strlen($hello) !== 13) throw new RuntimeException('Nieprawidłowy HELLO_RESP');
    $deviceId = bin2hex(substr($hello, 0, 8));
    $firmware = [ord($hello[8]), ord($hello[9]), ord($hello[10])];
    $capabilities = unpack('nvalue', substr($hello, 11, 2))['value'];

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
    $outputInhibited = ord($data[13]) === 1 ? true : null;

    respond(200, ['success' => true, 'device_id' => $deviceId,
        'firmware' => implode('.', $firmware), 'capabilities' => $capabilities,
        'status' => ['state' => $state, 'fault_flags' => $faultFlags,
            'uptime_seconds' => $uptime, 'free_memory_bytes' => $freeMemory,
            'temperature_c' => $temperatureValue, 'sensor_valid_flags' => $sensorFlags,
            'output_inhibited' => $outputInhibited]]);
} catch (Throwable $error) {
    respond(502, ['success' => false, 'error' => $error->getMessage()]);
} finally {
    socket_close($socket);
}
