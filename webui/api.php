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
 */

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
 */
function log_message($message) {
    $timestamp = date('H:i:s');
    $log_entry = "[$timestamp] $message";
    
    if (!isset($_SESSION['logs'])) {
        $_SESSION['logs'] = [];
    }
    
    array_push($_SESSION['logs'], $log_entry);
    
    // Ogranicz liczbę wpisów do 50
    if (count($_SESSION['logs']) > 50) {
        array_shift($_SESSION['logs']);
    }
}

/**
 * Połącz z urządzeniem Arduino
 */
function connect_to_device($ip, $port) {
    log_message("Łączenie z urządzeniem $ip:$port...");
    
    // Sprawdź czy socket jest dostępny
    if (!function_exists('fsockopen')) {
        log_message("ERROR: Funkcja fsockopen niedostępna!");
        return false;
    }
    
    // Spróbuj połączyć się z urządzeniem
    $errno = 0;
    $errstr = '';
    $socket = @fsockopen($ip, $port, $errno, $errstr, 2);
    
    if ($socket) {
        fclose($socket);
        $_SESSION['connected'] = true;
        $_SESSION['device_ip'] = $ip;
        $_SESSION['device_port'] = $port;
        log_message("Połączono z $ip:$port");
        return true;
    } else {
        $_SESSION['connected'] = false;
        log_message("ERROR: Błąd połączenia z urządzeniem $ip:$port - $errstr");
        return false;
    }
}

/**
 * Rozłącz z urządzeniem
 */
function disconnect_from_device() {
    $_SESSION['connected'] = false;
    log_message("Rozłączono z urządzeniem");
}

/**
 * Wyślij komendę do urządzenia
 */
function send_command($cmd) {
    if (!$_SESSION['connected']) {
        log_message("ERROR: Nie można wysłać komendy - brak połączenia: $cmd");
        return false;
    }
    
    $ip = $_SESSION['device_ip'];
    $port = $_SESSION['device_port'];
    
    log_message("Wysyłanie komendy: $cmd");
    
    $errno = 0;
    $errstr = '';
    $socket = @fsockopen($ip, $port, $errno, $errstr, 2);
    
    if ($socket) {
        fwrite($socket, $cmd . "\n");
        $response = fread($socket, 1024);
        fclose($socket);
        return $response;
    } else {
        log_message("ERROR: Nie udało się wysłać komendy: $cmd");
        return false;
    }
}

/**
 * Wyślij konfigurację końcówki do urządzenia
 */
function send_probe_config($channel) {
    if (!$_SESSION['connected']) {
        return false;
    }
    
    $probe = $_SESSION['probes'][$channel];
    
    // Konwertuj częstotliwość do formatu Hz * 100
    $freq_x100 = intval(floatval($probe['freq']) * 100);
    
    // Format pakietu: CONFIG:channel,freq_x100,duty,intensity,modulation
    $cmd = sprintf("CONFIG:%d,%d,%d,%d,%s", 
        $channel, 
        $freq_x100, 
        $probe['duty'], 
        $probe['intensity'], 
        $probe['modulation']
    );
    
    $response = send_command($cmd);
    log_message("Wysłano konfigurację kanału $channel");
    
    return $response;
}

/**
 * Pobierz status urządzenia
 */
function request_status() {
    if (!$_SESSION['connected']) {
        return false;
    }
    
    $response = send_command("s");
    parse_status_response($response);
    
    return $response;
}

/**
 * Parsuj odpowiedź statusu
 */
function parse_status_response($response) {
    if (!$response) {
        return;
    }
    
    // Parsuj temperaturę
    if (preg_match('/Temperature:\s*([0-9.]+)/', $response, $matches)) {
        $_SESSION['status']['temp'] = $matches[1];
    }
    
    // Parsuj pamięć
    if (preg_match('/Free Memory:\s*([0-9]+)/', $response, $matches)) {
        $_SESSION['status']['memory'] = $matches[1];
    }
    
    // Parsuj uptime
    if (preg_match('/Uptime:\s*([0-9]+)/', $response, $matches)) {
        $_SESSION['status']['uptime'] = $matches[1];
    }
    
    // Parsuj stan PWM
    if (preg_match('/PWM Running:\s*(YES|NO)/', $response, $matches)) {
        $_SESSION['status']['pwm'] = ($matches[1] === 'YES');
    }
    
    // Parsuj częstotliwość
    if (preg_match('/Frequency:\s*([0-9]+)/', $response, $matches)) {
        $_SESSION['status']['freq'] = $matches[1];
    }
    
    // Parsuj stan bezpieczeństwa
    if (preg_match('/Safety State:\s*([A-Z_]+)/', $response, $matches)) {
        $_SESSION['status']['safety'] = $matches[1];
    }
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
