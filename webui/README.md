# ResoNet-Nano Web UI

Interfejs webowy do obsługi efektora Arduino Nano z Ethernet HAT.

## Struktura plików

```
webui/
├── index.html    # Główny plik HTML z interfejsem użytkownika (CSS + JavaScript)
├── api.php       # Backend PHP obsługujący komunikację TCP z Arduino
└── README.md     # Ten plik
```

## Wymagania

- Serwer Apache2 z PHP 7.4 lub nowszym
- Włączona obsługa sesji PHP (`session_start()`)
- Włączona funkcja `fsockopen()` w PHP (domyślnie włączona)
- Arduino Nano z Ethernet HAT działające w sieci lokalnej

## Instalacja

### 1. Skopiuj pliki do katalogu Apache2

```bash
sudo cp -r /workspace/webui/* /var/www/html/
```

Lub jeśli chcesz użyć podkatalogu:

```bash
sudo mkdir -p /var/www/html/resonet
sudo cp -r /workspace/webui/* /var/www/html/resonet/
```

### 2. Ustaw uprawnienia

```bash
sudo chown -R www-data:www-data /var/www/html/webui
sudo chmod -R 755 /var/www/html/webui
```

### 3. Upewnij się, że sesje PHP są włączone

Sprawdź plik konfiguracyjny PHP (zwykle `/etc/php/8.x/apache2/php.ini`):

```ini
session.auto_start = 0
session.save_path = "/var/lib/php/sessions"
```

### 4. Restart Apache2

```bash
sudo systemctl restart apache2
```

## Użycie

1. Otwórz przeglądarkę internetową
2. Przejdź do adresu: `http://twoj-serwer/webui/` lub `http://twoj-serwer/resonet/`
3. Wprowadź adres IP urządzenia Arduino Nano (np. `192.168.1.100`)
4. Port domyślny: `5001`
5. Kliknij **Połącz**

## Funkcjonalności

### Konfiguracja Końcówek (8 kanałów)

Dla każdego kanału możesz skonfigurować:
- **Nazwa**: Dowolna nazwa opisowa
- **Typ**: 
  - Cewka Płaska (FLAT_COIL)
  - Cewka Ferrytowa (FERRITE_ROD)
  - Płyta Kapacytacyjna (CAPACITIVE_PLATE)
  - Aplikator Punktowy (PEN_APPLICATOR)
  - Mata EMF (MAT_APPLICATOR)
  - Podkładka Lokalna (LOCAL_PAD)
  - Pierścień (RING_APPLICATOR)
  - Niestandardowa (CUSTOM)
- **Częstotliwość**: Wartość w Hz (np. 727.00)
- **Duty Cycle**: Współczynnik wypełnienia 0-100%
- **Intensywność**: Wartość 0-4095
- **Modulacja**: Brak, AM, FM, Burst, Sweep
- **Status**: Włącz/Wyłącz kanał

### Tryby Pracy

- **Pojedynczy**: Praca na jednym kanale
- **Wielokanałowy**: Praca na wielu kanałach jednocześnie

### Sterowanie Terapią

- **Start Terapii**: Rozpoczyna generowanie sygnałów na aktywnych kanałach
- **Stop Terapii**: Zatrzymuje generowanie sygnałów
- **Wyślij Wszystkie**: Wysyła konfiguracje wszystkich aktywnych kanałów do urządzenia

### Monitorowanie Statusu

Panel statusu wyświetla:
- Temperaturę urządzenia
- Wolną pamięć RAM
- Czas pracy (uptime)
- Stan PWM (ACTIVE/STOPPED)
- Aktualną częstotliwość
- Stan systemu bezpieczeństwa

### Dziennik Zdarzeń

Automatycznie rejestruje wszystkie zdarzenia:
- Połączenia/rozłączenia
- Błędy
- Zmiany konfiguracji
- Komendy wysyłane do urządzenia

## API JSON

Backend udostępnia API REST poprzez `api.php?action=<action>`

### Dostępne akcje:

| Akcja | Metoda | Opis |
|-------|--------|------|
| `connect` | POST | Połączenie z urządzeniem |
| `disconnect` | GET | Rozłączenie z urządzeniem |
| `get_probes` | GET | Pobranie konfiguracji końcówek |
| `set_probe` | POST | Ustawienie pola konfiguracji |
| `send_config` | POST | Wysłanie konfiguracji kanału |
| `send_all_configs` | POST | Wysłanie wszystkich konfiguracji |
| `start_therapy` | POST | Start terapii |
| `stop_therapy` | POST | Stop terapii |
| `get_status` | GET | Pobranie statusu urządzenia |
| `get_logs` | GET | Pobranie dziennika zdarzeń |
| `set_probe_mode` | POST | Ustawienie trybu pracy |
| `get_connection` | GET | Sprawdzenie stanu połączenia |

### Przykłady żądań:

```javascript
// Połączenie z urządzeniem
fetch('api.php?action=connect', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ip: '192.168.1.100', port: 5001})
})

// Pobranie konfiguracji
fetch('api.php?action=get_probes')

// Ustawienie częstotliwości
fetch('api.php?action=set_probe', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({channel: 1, field: 'freq', value: 727.00})
})

// Start terapii
fetch('api.php?action=start_therapy', {method: 'POST'})
```

## Format Komend TCP do Arduino

Aplikacja wysyła komendy w formacie zrozumiałym dla Arduino Nano:

- `CONFIG:channel,freq_x100,duty,intensity,modulation` - konfiguracja kanału
- `s` - pobranie statusu
- `START` - start terapii
- `STOP` - stop terapii

Przykład:
```
CONFIG:1,72700,50,2048,NONE
```

## Bezpieczeństwo

- Walidacja danych wejściowych (zakresy wartości)
- Sesje PHP do przechowywania stanu
- Timeout połączenia (2 sekundy)
- Obsługa błędów sieciowych

## Rozwiązywanie Problemów

### Nie można połączyć z urządzeniem

1. Sprawdź czy Arduino jest włączone i podłączone do sieci
2. Zweryfikuj adres IP i port
3. Upewnij się, że firewall nie blokuje połączenia
4. Sprawdź czy Arduino nasłuchuje na porcie 5001

### Błąd sesji PHP

1. Sprawdź uprawnienia do katalogu sesji
2. Upewnij się, że `session.save_path` istnieje
3. Restart Apache2

### Puste logi

1. Sprawdź czy funkcja `fsockopen()` jest włączona
2. Sprawdź logi błędów Apache2: `/var/log/apache2/error.log`

## Autor

ResoNet-Nano Team - na bazie funkcji z `bioresonance_tui.sh`

## Licencja

Open Source
