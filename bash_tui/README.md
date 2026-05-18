# 🖥️ Bioresonance TUI - Terminalowy Interfejs Użytkownika (BASH)

## 📋 Opis

Profesjonalny terminalowy interfejs użytkownika (TUI) do sterowania systemem biorezonansu **ResoNet-Nano** z Arduino Nano i Ethernet HAT. Aplikacja umożliwia konfigurację i obsługę licznych końcówek biorezonansowych w trybie pojedynczym i wielokanałowym.

**Wersja Bash** - implementacja w czystym shell script bez zależności od C++, Python czy innych języków kompilowanych.

## 🔧 Wymagania

### Systemowe
- **System operacyjny**: Linux z bashem w wersji 4.0+
- **Narzędzia**: 
  - `nc` (netcat) - do komunikacji sieciowej TCP
  - `bc` - do obliczeń matematycznych
  - `tput` - do manipulacji terminalem
  - `timeout` - do kontroli czasu połączenia

### Instalacja zależności

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install netcat-openbsd bc

# Fedora/RHEL
sudo dnf install ncurses bc

# Arch Linux
sudo pacman -S gnu-netcat bc
```

## 🚀 Uruchomienie

```bash
cd /workspace/bash_tui

# Nadaj uprawnienia wykonania (jeśli jeszcze nie nadane)
chmod +x bioresonance_tui.sh

# Podstawowe użycie (domyślny IP i port)
./bioresonance_tui.sh

# Z podaniem adresu IP urządzenia
./bioresonance_tui.sh 192.168.1.100

# Z podaniem IP i portu
./bioresonance_tui.sh 192.168.1.100 5001
```

## 🎮 Sterowanie

### Nawigacja
| Klawisz | Funkcja |
|---------|---------|
| `↑` `↓` | Nawigacja w menu głównym |
| `←` `→` | Wybór kanału/końcówki |
| `Q` | Wyjście z aplikacji |

### Akcje Szybkie
| Klawisz | Funkcja |
|---------|---------|
| `1` | Edycja częstotliwości wybranej końcówki |
| `2` | Wybór trybu pracy |
| `3` | Start terapii |
| `4` | Stop terapii |
| `5` | Odśwież status systemu |
| `E` | Włącz/Wyłącz wybraną końcówkę |
| `F` | Edycja częstotliwości |
| `I` | Edycja intensywności |
| `M` | Zmiana modulacji |
| `S` | Odśwież status |
| `H` | Wyświetl pomoc |

## 🔌 Obsługiwane Końcówki Biorezonansowe

### 1. **Cewka Płaska (Flat Coil)**
- **Zastosowanie**: Terapia powierzchniowa, punkty akupunkturowe
- **Domyślna częstotliwość**: 727 Hz
- **Kanał**: 1

### 2. **Cewka Ferrytowa (Ferrite Rod)**
- **Zastosowanie**: Terapia głęboka, narządy wewnętrzne
- **Domyślna częstotliwość**: 10 kHz
- **Kanał**: 2

### 3. **Płyta Kapacytacyjna (Capacitive Plate)**
- **Zastosowanie**: Aplikacje ogólnoustrojowe
- **Domyślna częstotliwość**: 5 kHz
- **Kanał**: 3

### 4. **Aplikator Punktowy (Pen Applicator)**
- **Zastosowanie**: Terapia punktowa, precyzyjna
- **Domyślna częstotliwość**: 25 kHz
- **Kanał**: 4

### 5. **Mata EMF (EMF Mat)**
- **Zastosowanie**: Terapia całego ciała
- **Domyślna częstotliwość**: 78.3 Hz (rezonans Schumanna)
- **Kanał**: 5

### 6. **Podkładka Lokalna (Local Pad)**
- **Zastosowanie**: Aplikacje lokalne na konkretne obszary
- **Domyślna częstotliwość**: 1 kHz
- **Kanał**: 6

### 7. **Pierścień (Ring Applicator)**
- **Zastosowanie**: Kończyny, palce
- **Domyślna częstotliwość**: 500 Hz
- **Kanał**: 7

### 8. **Konfiguracja Niestandardowa (Custom)**
- **Zastosowanie**: Dowolna konfiguracja użytkownika
- **Domyślna częstotliwość**: 10 Hz
- **Kanał**: 8

## 🔄 Tryby Pracy

### Pojedyncza (SINGLE)
- Tylko jedna aktywna końcówka w danym czasie

### Dual Niezależny (DUAL_INDEPENDENT)
- Dwie niezależne końcówki pracujące jednocześnie
- Każda z własną konfiguracją częstotliwości

### Dual Sync (DUAL_SYNC)
- Dwie zsynchronizowane końcówki
- Ta sama częstotliwość, faza synchronizowana

### Wielokanałowa (MULTI_CHANNEL)
- Do 8 końcówek pracujących jednocześnie
- Każda z indywidualną konfiguracją

### Sekwencyjna (SEQUENTIAL)
- Rotacyjne włączanie końcówek w zadanej sekwencji
- Automatyczne przełączanie co określony czas

## ⚙️ Parametry Konfiguracyjne

### Częstotliwość
- Zakres: 0.1 Hz - 500 kHz
- Rozdzielczość: 0.01 Hz
- Jednostka: Hz

### Cykl Pracy (Duty Cycle)
- Zakres: 1% - 99%
- Domyślnie: 50%

### Intensywność
- Zakres: 0 - 4095 (12-bit)
- Domyślnie: 2048

### Modulacje
- **NONE**: Brak modulacji
- **AM**: Modulacja amplitudy (1 Hz)
- **FM**: Modulacja częstotliwości (±10%, 0.5 Hz)
- **BURST**: Impulsowa (500ms on/off)
- **SWEEP**: Przemiatający zakres

## 📊 Panel Statusu

Aplikacja wyświetla na bieżąco:
- Temperaturę MCU Arduino
- Wolną pamięć RAM
- Czas pracy (uptime)
- Stan PWM (ACTIVE/STOPPED)
- Aktualną częstotliwość
- Status połączenia sieciowego
- Stan systemu bezpieczeństwa

## 🔐 Bezpieczeństwo

⚠️ **WAŻNE**: Przed użyciem urządzenia medycznego:
1. Sprawdź izolację galwaniczną
2. Zweryfikuj parametry wyjściowe oscyloskopem
3. Przestrzegaj norm IEC 60601-1
4. Konsultuj się z profesjonalistą

## 📝 Przykłady Użycia

### Sesja podstawowa - cewka płaska
```bash
# 1. Uruchom TUI
./bioresonance_tui.sh 192.168.1.100

# 2. Wybierz kanał 1 (cewka płaska) strzałkami
# 3. Naciśnij 'E' aby aktywować
# 4. Naciśnij 'F' i ustaw 727 Hz
# 5. Naciśnij '3' aby rozpocząć terapię
# 6. Po zakończeniu naciśnij '4'
```

### Terapia wielokanałowa
```bash
# 1. Aktywuj kanały 1, 3, 5 (E dla każdego)
# 2. Ustaw różne częstotliwości (F)
# 3. Wybierz tryb MULTI_CHANNEL (klawisz 2)
# 4. Start terapii (klawisz 3)
```

## 🛠️ Różnice względem wersji C++

| Cecha | Wersja C++ | Wersja BASH |
|-------|-----------|-------------|
| Wydajność | Wysoka | Średnia |
| Zależności | ncurses | netcat, bc, tput |
| Kompilacja | Wymagana | Nie wymaga |
| Portowalność | Ograniczona | Wysoka |
| Modyfikacje | Trudniejsze | Łatwe |
| Debugowanie | Trudne | Łatwe |

## 🐛 Rozwiązywanie Problemów

### Błąd: nc (netcat) nie jest zainstalowany
```bash
Rozwiązanie: Zainstaluj netcat:
sudo apt-get install netcat-openbsd
```

### Błąd: bc: command not found
```bash
Rozwiązanie: Zainstaluj bc:
sudo apt-get install bc
```

### Błąd połączenia
```bash
Rozwiązanie: Sprawdź czy Arduino jest podłączone do sieci
i ma poprawny adres IP. Pinguj urządzenie: ping 192.168.1.100
```

### Brak odpowiedzi
```bash
Rozwiązanie: Restart Arduino, sprawdź kabel Ethernet,
zweryfikuj konfigurację sieciową
```

### Problem z kolorami w terminalu
```bash
Rozwiązanie: Upewnij się, że terminal obsługuje kolory ANSI.
Spróbuj ustawić: export TERM=xterm-256color
```

## 📄 Licencja

MIT License - patrz główny plik LICENSE projektu ResoNet-Nano.

## 👨‍💻 Autor

ResoNet Development Team - Profesjonalne Systemy Biorezonansu

---

**Wersja**: 1.0 (BASH)  
**Data**: 2024  
**Kompatybilność**: ResoNet-Nano Firmware v4.0+  
**Wymagany bash**: 4.0+
