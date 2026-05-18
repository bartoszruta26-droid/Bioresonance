<?php
/**
 * ResoNet-Nano Web UI - Backend PHP
 * Interfejs webowy do obsługi efektora Arduino Nano
 * 
 * Ten plik komunikuje się z urządzeniem Arduino poprzez TCP/IP
 * i udostępnia API JSON dla frontendu JavaScript.
 * 
 * @version 1.0
 * @author ResoNet-Nano Team
 * 
 * ============================================================================
 * DEBUG MODE - Tryb Debugowania
 * ============================================================================
 * Ustaw na true aby włączyć szczegółowe logowanie wszystkich operacji
 * W trybie debug:
 * - Wszystkie operacje są logowane do pliku debug.log
 * - Komunikaty błędów są bardziej szczegółowe
 * - Czas wykonania operacji jest mierzony
 * - Surowe dane z urządzenia są zapisywane
 */
define('DEBUG_MODE', true);
define('DEBUG_LOG_FILE', __DIR__ . '/debug.log');

// Funkcja pomocnicza do logowania debug
function debug_log($message, $level = 'INFO') {
    if (!DEBUG_MODE) {
        return;
    }
    
    $timestamp = date('Y-m-d H:i:s.u');
    $backtrace = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS, 2);
    $caller = isset($backtrace[1]) ? $backtrace[1]['function'] : 'unknown';
    $line = isset($backtrace[0]) ? $backtrace[0]['line'] : 'unknown';
    
    $log_message = sprintf(
        "[%s] [%s] [%s:%d] %s",
        $timestamp,
        $level,
        $caller,
        $line,
        $message
    );
    
    // Loguj do pliku
    file_put_contents(DEBUG_LOG_FILE, $log_message . PHP_EOL, FILE_APPEND | LOCK_EX);
    
    // Jeśli to błąd, loguj też do error_log PHP
    if ($level === 'ERROR' || $level === 'CRITICAL') {
        error_log($log_message);
    }
}

// Rozpocznij mierzenie czasu wykonania skryptu
$script_start_time = microtime(true);
debug_log("=== URUCHOMIENIE SKRYPTU api.php ===", "START");
debug_log("REQUEST_METHOD: " . $_SERVER['REQUEST_METHOD']);
debug_log("REQUEST_URI: " . $_SERVER['REQUEST_URI']);
debug_log("REMOTE_ADDR: " . ($_SERVER['REMOTE_ADDR'] ?? 'unknown'));

// ============================================================================
// KONFIGURACJA SYSTEMU
// ============================================================================

$DEFAULT_PORT = 5001;
$TIMEOUT_SEC = 2;

// Domyślne ustawienia końcówek (probe)
$DEFAULT_PROBES = [
    1 => ["name" => "Cewka Płaska Główna", "type" => "FLAT_COIL", "freq" => 727.00, "duty" => 50, "intensity" => 2048, "modulation" => "NONE", "enabled" => false],
    2 => ["name" => "Cewka Ferrytowa Punktowa", "type" => "FERRITE_ROD", "freq" => 10000.00, "duty" => 50, "intensity" => 2048, "modulation" => "NONE", "enabled" => false],
    3 => ["name" => "Płyta Kapacytacyjna", "type" => "CAPACITIVE_PLATE", "freq" => 5000.00, "duty" => 50, "intensity" => 2048, "modulation" => "NONE", "enabled" => false],
    4 => ["name" => "Aplikator Punktowy", "type" => "PEN_APPLICATOR", "freq" => 25000.00, "duty" => 50, "intensity" => 2048, "modulation" => "NONE", "enabled" => false],
    5 => ["name" => "Mata EMF Całe Ciało", "type" => "MAT_APPLICATOR", "freq" => 78.30, "duty" => 50, "intensity" => 2048, "modulation" => "NONE", "enabled" => false],
    6 => ["name" => "Podkładka Lokalna", "type" => "LOCAL_PAD", "freq" => 1000.00, "duty" => 50, "intensity" => 2048, "modulation" => "NONE", "enabled" => false],
    7 => ["name" => "Pierścień Kończyna", "type" => "RING_APPLICATOR", "freq" => 500.00, "duty" => 50, "intensity" => 2048, "modulation" => "NONE", "enabled" => false],
    8 => ["name" => "Konfiguracja Niestandardowa", "type" => "CUSTOM", "freq" => 10.00, "duty" => 50, "intensity" => 2048, "modulation" => "NONE", "enabled" => false]
];

// Nazwy typów końcówek
$TYPE_NAMES = [
    "FLAT_COIL" => "Cewka Płaska",
    "FERRITE_ROD" => "Cewka Ferrytowa",
    "CAPACITIVE_PLATE" => "Płyta Kapacytacyjna",
    "PEN_APPLICATOR" => "Aplikator Punktowy",
    "MAT_APPLICATOR" => "Mata EMF",
    "LOCAL_PAD" => "Podkładka Lokalna",
    "RING_APPLICATOR" => "Pierścień",
    "CUSTOM" => "Niestandardowa"
];

// Nazwy modulacji
$MODULATION_NAMES = [
    "NONE" => "Brak",
    "AM" => "AM",
    "FM" => "FM",
    "BURST" => "Burst",
    "SWEEP" => "Sweep"
];

// ============================================================================
// ZMIENNE SESJI I STANU
// ============================================================================

session_start();

// Inicjalizacja stanu w sesji jeśli nie istnieje
if (!isset($_SESSION['probes'])) {
    $_SESSION['probes'] = $DEFAULT_PROBES;
}
if (!isset($_SESSION['device_ip'])) {
    $_SESSION['device_ip'] = '';
}
if (!isset($_SESSION['device_port'])) {
    $_SESSION['device_port'] = $DEFAULT_PORT;
}
if (!isset($_SESSION['connected'])) {
    $_SESSION['connected'] = false;
}
if (!isset($_SESSION['status'])) {
    $_SESSION['status'] = [
        'temp' => '0.0',
        'memory' => '0',
        'uptime' => '0',
        'pwm' => false,
        'freq' => '0',
        'safety' => 'UNKNOWN'
    ];
}
if (!isset($_SESSION['logs'])) {
    $_SESSION['logs'] = [];
}
if (!isset($_SESSION['probe_mode'])) {
    $_SESSION['probe_mode'] = 'SINGLE';
}

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

/**
 * Dodaj wpis do dziennika zdarzeń
 * 
 * @param string $message Wiadomość do zalogowania
 * @return void
 */
function log_message($message) {
    debug_log("log_message called: $message");
    
    $timestamp = date('H:i:s');
    $log_entry = "[$timestamp] $message";
    
    if (!isset($_SESSION['logs'])) {
        $_SESSION['logs'] = [];
        debug_log("Zainicjalizowano pustą listę logów w sesji", "DEBUG");
    }
    
    array_push($_SESSION['logs'], $log_entry);
    debug_log("Dodano wpis do logu: $log_entry", "DEBUG");
    
    // Ogranicz liczbę wpisów do 50
    if (count($_SESSION['logs']) > 50) {
        debug_log("Ograniczanie liczby logów (usunięcie najstarszego wpisu)", "DEBUG");
        array_shift($_SESSION['logs']);
    }
    
    debug_log("Aktualna liczba wpisów w logu: " . count($_SESSION['logs']), "DEBUG");
}

/**
 * Połącz z urządzeniem Arduino
 * 
 * @param string $ip Adres IP urządzenia
 * @param int $port Port TCP urządzenia
 * @return bool true jeśli połączenie成功了，false w przeciwnym razie
 */
function connect_to_device($ip, $port) {
    debug_log("connect_to_device called with IP: $ip, Port: $port", "INFO");
    
    log_message("Łączenie z urządzeniem $ip:$port...");
    
    // Sprawdź czy socket jest dostępny
    if (!function_exists('fsockopen')) {
        debug_log("Funkcja fsockopen niedostępna w tej instalacji PHP", "ERROR");
        log_message("ERROR: Funkcja fsockopen niedostępna!");
        return false;
    }
    
    // Walidacja adresu IP
    if (!filter_var($ip, FILTER_VALIDATE_IP)) {
        debug_log("Nieprawidłowy format adresu IP: $ip", "ERROR");
        log_message("ERROR: Nieprawidłowy adres IP: $ip");
        return false;
    }
    
    // Walidacja portu
    if (!is_numeric($port) || $port < 1 || $port > 65535) {
        debug_log("Nieprawidłowy numer portu: $port", "ERROR");
        log_message("ERROR: Nieprawidłowy port: $port");
        return false;
    }
    
    // Spróbuj połączyć się z urządzeniem
    $errno = 0;
    $errstr = '';
    debug_log("Próba połączenia przez fsockopen...", "DEBUG");
    $socket = @fsockopen($ip, $port, $errno, $errstr, 2);
    
    if ($socket) {
        debug_log("Połączenie nawiązane pomyślnie", "INFO");
        fclose($socket);
        $_SESSION['connected'] = true;
        $_SESSION['device_ip'] = $ip;
        $_SESSION['device_port'] = $port;
        log_message("Połączono z $ip:$port");
        return true;
    } else {
        debug_log("Błąd połączenia: errno=$errno, errstr=$errstr", "ERROR");
        $_SESSION['connected'] = false;
        log_message("ERROR: Błąd połączenia z urządzeniem $ip:$port - $errstr (errno: $errno)");
        return false;
    }
}

/**
 * Rozłącz z urządzeniem
 * 
 * @return void
 */
function disconnect_from_device() {
    debug_log("disconnect_from_device called", "INFO");
    
    if (isset($_SESSION['connected']) && $_SESSION['connected']) {
        debug_log("Aktywne połączenie zostanie zamknięte", "DEBUG");
    }
    
    $_SESSION['connected'] = false;
    log_message("Rozłączono z urządzeniem");
}

/**
 * Wyślij komendę do urządzenia
 * 
 * @param string $cmd Komenda do wysłania
 * @return string|false Odpowiedź z urządzenia lub false w przypadku błędu
 */
function send_command($cmd) {
    debug_log("send_command called with command: '$cmd'", "INFO");
    
    if (!$_SESSION['connected']) {
        debug_log("Brak aktywnego połączenia - nie można wysłać komendy", "ERROR");
        log_message("ERROR: Nie można wysłać komendy - brak połączenia: $cmd");
        return false;
    }
    
    $ip = $_SESSION['device_ip'];
    $port = $_SESSION['device_port'];
    
    debug_log("Połączenie aktywne z {$ip}:{$port}", "DEBUG");
    log_message("Wysyłanie komendy: $cmd");
    
    // Sprawdź czy socket jest dostępny
    if (!function_exists('fsockopen')) {
        debug_log("Funkcja fsockopen niedostępna", "ERROR");
        log_message("ERROR: Funkcja fsockopen niedostępna!");
        return false;
    }
    
    $errno = 0;
    $errstr = '';
    debug_log("Otwieranie socketu do wysłania komendy...", "DEBUG");
    $socket = @fsockopen($ip, $port, $errno, $errstr, 2);
    
    if ($socket) {
        debug_log("Socket otwarty pomyślnie", "DEBUG");
        
        // Dodaj znak nowej linii na końcu komendy
        $full_cmd = $cmd . "\n";
        debug_log("Wysyłanie danych: " . trim($full_cmd), "DEBUG");
        $bytes_written = fwrite($socket, $full_cmd);
        debug_log("Wysłano bajtów: $bytes_written", "DEBUG");
        
        // Ustaw timeout na odczyt
        stream_set_timeout($socket, 2);
        
        // Odczytaj odpowiedź
        debug_log("Oczekiwanie na odpowiedź...", "DEBUG");
        $response = fread($socket, 1024);
        debug_log("Otrzymano odpowiedź: " . trim($response ?? '(brak)'), "DEBUG");
        
        // Sprawdź czy nie wystąpił timeout
        $meta = stream_get_meta_data($socket);
        if ($meta['timed_out']) {
            debug_log("Timeout podczas oczekiwania na odpowiedź", "WARNING");
            log_message("WARNING: Timeout odpowiedzi z urządzenia");
        }
        
        fclose($socket);
        debug_log("Socket zamknięty", "DEBUG");
        
        return $response;
    } else {
        debug_log("Nie udało się otworzyć socketu: errno=$errno, errstr=$errstr", "ERROR");
        log_message("ERROR: Nie udało się wysłać komendy: $cmd (errno: $errno)");
        return false;
    }
}

/**
 * Wyślij konfigurację końcówki do urządzenia
 * 
 * @param int $channel Numer kanału (1-8)
 * @return string|false Odpowiedź z urządzenia lub false w przypadku błędu
 */
function send_probe_config($channel) {
    debug_log("send_probe_config called for channel: $channel", "INFO");
    
    if (!$_SESSION['connected']) {
        debug_log("Brak połączenia - nie można wysłać konfiguracji", "ERROR");
        return false;
    }
    
    if (!isset($_SESSION['probes'][$channel])) {
        debug_log("Nieprawidłowy numer kanału: $channel", "ERROR");
        log_message("ERROR: Nieprawidłowy kanał: $channel");
        return false;
    }
    
    $probe = $_SESSION['probes'][$channel];
    debug_log("Konfiguracja kanału $channel: " . json_encode($probe, JSON_UNESCAPED_UNICODE), "DEBUG");
    
    // Konwertuj częstotliwość do formatu Hz * 100
    $freq_x100 = intval(floatval($probe['freq']) * 100);
    debug_log("Częstotliwość {$probe['freq']} Hz -> {$freq_x100} (x100)", "DEBUG");
    
    // Format pakietu: CONFIG:channel,freq_x100,duty,intensity,modulation
    $cmd = sprintf("CONFIG:%d,%d,%d,%d,%s", 
        $channel, 
        $freq_x100, 
        $probe['duty'], 
        $probe['intensity'], 
        $probe['modulation']
    );
    
    debug_log("Generowanie komendy: $cmd", "DEBUG");
    $response = send_command($cmd);
    
    if ($response !== false) {
        log_message("Wysłano konfigurację kanału $channel: {$probe['name']}");
        debug_log("Odpowiedź z urządzenia: " . trim($response ?? '(brak)'), "DEBUG");
    } else {
        log_message("ERROR: Nie udało się wysłać konfiguracji kanału $channel");
    }
    
    return $response;
}

/**
 * Pobierz status urządzenia
 * 
 * @return string|false Odpowiedź z urządzenia lub false w przypadku błędu
 */
function request_status() {
    debug_log("request_status called", "INFO");
    
    if (!$_SESSION['connected']) {
        debug_log("Brak połączenia - nie można pobrać statusu", "WARNING");
        return false;
    }
    
    debug_log("Wysyłanie komendy statusu 's'", "DEBUG");
    $response = send_command("s");
    
    if ($response !== false) {
        debug_log("Otrzymano odpowiedź statusu, parsowanie...", "DEBUG");
        parse_status_response($response);
    } else {
        debug_log("Brak odpowiedzi na komendę statusu", "WARNING");
    }
    
    return $response;
}

/**
 * Parsuj odpowiedź statusu i aktualizuj sesję
 * 
 * @param string $response Surowa odpowiedź z urządzenia
 * @return void
 */
function parse_status_response($response) {
    debug_log("parse_status_response called", "DEBUG");
    
    if (!$response) {
        debug_log("Pusta odpowiedź - pomijam parsowanie", "DEBUG");
        return;
    }
    
    debug_log("Surowa odpowiedź: " . trim($response), "DEBUG");
    
    // Parsuj temperaturę
    if (preg_match('/Temperature:\s*([0-9.]+)/', $response, $matches)) {
        $_SESSION['status']['temp'] = $matches[1];
        debug_log("Znaleziono temperaturę: {$matches[1]}°C", "DEBUG");
    } else {
        debug_log("Nie znaleziono temperatury w odpowiedzi", "DEBUG");
    }
    
    // Parsuj pamięć
    if (preg_match('/Free Memory:\s*([0-9]+)/', $response, $matches)) {
        $_SESSION['status']['memory'] = $matches[1];
        debug_log("Znaleziono pamięć: {$matches[1]} bytes", "DEBUG");
    } else {
        debug_log("Nie znaleziono pamięci w odpowiedzi", "DEBUG");
    }
    
    // Parsuj uptime
    if (preg_match('/Uptime:\s*([0-9]+)/', $response, $matches)) {
        $_SESSION['status']['uptime'] = $matches[1];
        debug_log("Znaleziono uptime: {$matches[1]} s", "DEBUG");
    } else {
        debug_log("Nie znaleziono uptime w odpowiedzi", "DEBUG");
    }
    
    // Parsuj stan PWM
    if (preg_match('/PWM Running:\s*(YES|NO)/', $response, $matches)) {
        $_SESSION['status']['pwm'] = ($matches[1] === 'YES');
        debug_log("Znaleziono stan PWM: " . ($matches[1] === 'YES' ? 'TAK' : 'NIE'), "DEBUG");
    } else {
        debug_log("Nie znaleziono stanu PWM w odpowiedzi", "DEBUG");
    }
    
    // Parsuj częstotliwość
    if (preg_match('/Frequency:\s*([0-9]+)/', $response, $matches)) {
        $_SESSION['status']['freq'] = $matches[1];
        debug_log("Znaleziono częstotliwość: {$matches[1]} Hz", "DEBUG");
    } else {
        debug_log("Nie znaleziono częstotliwości w odpowiedzi", "DEBUG");
    }
    
    // Parsuj stan bezpieczeństwa
    if (preg_match('/Safety State:\s*([A-Z_]+)/', $response, $matches)) {
        $_SESSION['status']['safety'] = $matches[1];
        debug_log("Znaleziono stan bezpieczeństwa: {$matches[1]}", "DEBUG");
    } else {
        debug_log("Nie znaleziono stanu bezpieczeństwa w odpowiedzi", "DEBUG");
    }
    
    debug_log("Aktualny status po parsowaniu: " . json_encode($_SESSION['status']), "DEBUG");
}

/**
 * Aktualizuj pole konfiguracji końcówki
 */
function set_probe_field($channel, $field, $value) {
    if (!isset($_SESSION['probes'][$channel])) {
        log_message("ERROR: Nieprawidłowy kanał: $channel");
        return false;
    }
    
    $probe = &$_SESSION['probes'][$channel];
    
    switch ($field) {
        case 'name':
            $probe['name'] = $value;
            break;
        case 'type':
            $probe['type'] = $value;
            break;
        case 'freq':
            // Walidacja częstotliwości
            if (!is_numeric($value)) {
                log_message("ERROR: Nieprawidłowy format częstotliwości: $value");
                return false;
            }
            $probe['freq'] = floatval($value);
            break;
        case 'duty':
            // Walidacja duty cycle
            if (!is_numeric($value) || $value < 0 || $value > 100) {
                log_message("ERROR: Duty cycle poza zakresem (0-100): $value");
                return false;
            }
            $probe['duty'] = intval($value);
            break;
        case 'intensity':
            // Walidacja intensywności
            if (!is_numeric($value) || $value < 0 || $value > 4095) {
                log_message("ERROR: Intensywność poza zakresem (0-4095): $value");
                return false;
            }
            $probe['intensity'] = intval($value);
            break;
        case 'modulation':
            $probe['modulation'] = $value;
            break;
        case 'enabled':
            $probe['enabled'] = ($value === true || $value === 'true');
            break;
        default:
            log_message("ERROR: Nieznane pole: $field");
            return false;
    }
    
    log_message("Zmieniono probe $channel.$field = $value");
    return true;
}

// ============================================================================
// OBSŁUGA ŻĄDAŃ API
// ============================================================================

header('Content-Type: application/json; charset=utf-8');

$action = isset($_GET['action']) ? $_GET['action'] : '';
$method = $_SERVER['REQUEST_METHOD'];

$response = ['success' => false, 'message' => ''];

try {
    switch ($action) {
        case 'connect':
            if ($method === 'POST') {
                $data = json_decode(file_get_contents('php://input'), true);
                $ip = isset($data['ip']) ? $data['ip'] : '';
                $port = isset($data['port']) ? intval($data['port']) : $DEFAULT_PORT;
                
                if (empty($ip)) {
                    throw new Exception("Podaj adres IP urządzenia");
                }
                
                if (connect_to_device($ip, $port)) {
                    $response['success'] = true;
                    $response['message'] = "Połączono z urządzeniem";
                    $response['connected'] = true;
                } else {
                    throw new Exception("Nie udało się połączyć z urządzeniem");
                }
            } else {
                throw new Exception("Metoda POST wymagana");
            }
            break;
            
        case 'disconnect':
            disconnect_from_device();
            $response['success'] = true;
            $response['message'] = "Rozłączono z urządzeniem";
            $response['connected'] = false;
            break;
            
        case 'get_probes':
            $response['success'] = true;
            $response['probes'] = $_SESSION['probes'];
            $response['type_names'] = $TYPE_NAMES;
            $response['modulation_names'] = $MODULATION_NAMES;
            $response['probe_mode'] = $_SESSION['probe_mode'];
            break;
            
        case 'set_probe':
            if ($method === 'POST') {
                $data = json_decode(file_get_contents('php://input'), true);
                $channel = isset($data['channel']) ? intval($data['channel']) : 0;
                $field = isset($data['field']) ? $data['field'] : '';
                $value = isset($data['value']) ? $data['value'] : null;
                
                if ($channel < 1 || $channel > 8) {
                    throw new Exception("Nieprawidłowy numer kanału");
                }
                
                if (set_probe_field($channel, $field, $value)) {
                    $response['success'] = true;
                    $response['message'] = "Zaktualizowano konfigurację";
                    $response['probes'] = $_SESSION['probes'];
                } else {
                    throw new Exception("Nie udało się zaktualizować konfiguracji");
                }
            } else {
                throw new Exception("Metoda POST wymagana");
            }
            break;
            
        case 'send_config':
            if ($method === 'POST') {
                $data = json_decode(file_get_contents('php://input'), true);
                $channel = isset($data['channel']) ? intval($data['channel']) : 0;
                
                if ($channel < 1 || $channel > 8) {
                    throw new Exception("Nieprawidłowy numer kanału");
                }
                
                if (!$_SESSION['connected']) {
                    throw new Exception("Brak połączenia z urządzeniem");
                }
                
                send_probe_config($channel);
                $response['success'] = true;
                $response['message'] = "Wysłano konfigurację do urządzenia";
            } else {
                throw new Exception("Metoda POST wymagana");
            }
            break;
            
        case 'send_all_configs':
            if ($method === 'POST') {
                if (!$_SESSION['connected']) {
                    throw new Exception("Brak połączenia z urządzeniem");
                }
                
                for ($i = 1; $i <= 8; $i++) {
                    if ($_SESSION['probes'][$i]['enabled']) {
                        send_probe_config($i);
                    }
                }
                
                $response['success'] = true;
                $response['message'] = "Wysłano wszystkie aktywne konfiguracje";
            } else {
                throw new Exception("Metoda POST wymagana");
            }
            break;
            
        case 'start_therapy':
            if ($method === 'POST') {
                if (!$_SESSION['connected']) {
                    throw new Exception("Brak połączenia z urządzeniem");
                }
                
                // Wyślij konfiguracje dla wszystkich aktywnych kanałów
                for ($i = 1; $i <= 8; $i++) {
                    if ($_SESSION['probes'][$i]['enabled']) {
                        send_probe_config($i);
                    }
                }
                
                // Wyślij komendę start
                send_command("START");
                
                log_message("Rozpoczęto terapię");
                $response['success'] = true;
                $response['message'] = "Terapia rozpoczęta";
            } else {
                throw new Exception("Metoda POST wymagana");
            }
            break;
            
        case 'stop_therapy':
            if ($method === 'POST') {
                if (!$_SESSION['connected']) {
                    throw new Exception("Brak połączenia z urządzeniem");
                }
                
                send_command("STOP");
                log_message("Zatrzymano terapię");
                $response['success'] = true;
                $response['message'] = "Terapia zatrzymana";
            } else {
                throw new Exception("Metoda POST wymagana");
            }
            break;
            
        case 'get_status':
            if ($_SESSION['connected']) {
                request_status();
            }
            $response['success'] = true;
            $response['status'] = $_SESSION['status'];
            $response['connected'] = $_SESSION['connected'];
            break;
            
        case 'get_logs':
            $response['success'] = true;
            $response['logs'] = array_values(array_slice($_SESSION['logs'], -20));
            break;
            
        case 'set_probe_mode':
            if ($method === 'POST') {
                $data = json_decode(file_get_contents('php://input'), true);
                $mode = isset($data['mode']) ? $data['mode'] : 'SINGLE';
                
                $_SESSION['probe_mode'] = $mode;
                log_message("Zmieniono tryb pracy na: $mode");
                
                $response['success'] = true;
                $response['message'] = "Zmieniono tryb pracy";
                $response['probe_mode'] = $mode;
            } else {
                throw new Exception("Metoda POST wymagana");
            }
            break;
            
        case 'get_connection':
            $response['success'] = true;
            $response['connected'] = $_SESSION['connected'];
            $response['ip'] = $_SESSION['device_ip'];
            $response['port'] = $_SESSION['device_port'];
            break;
            
        default:
            $response['success'] = true;
            $response['message'] = "Dostępne akcje: connect, disconnect, get_probes, set_probe, send_config, start_therapy, stop_therapy, get_status, get_logs";
            break;
    }
} catch (Exception $e) {
    $response['success'] = false;
    $response['message'] = $e->getMessage();
    log_message("ERROR: " . $e->getMessage());
}

echo json_encode($response, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE);
