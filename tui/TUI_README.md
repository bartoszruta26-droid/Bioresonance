# 🖥️ Bioresonance TUI - Terminalowy Interfejs Użytkownika

## 📋 Opis

Profesjonalny terminalowy interfejs użytkownika (TUI) do sterowania systemem biorezonansu **ResoNet-Nano** z Arduino Nano i Ethernet HAT. Aplikacja umożliwia konfigurację i obsługę licznych końcówek biorezonansowych w trybie pojedynczym i wielokanałowym.

## 🔧 Wymagania

### Systemowe
- **System operacyjny**: Linux (Ubuntu, Debian, Fedora, Arch)
- **Kompilator**: g++ z obsługą C++17
- **Biblioteki**: ncurses

### Instalacja zależności

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install libncurses5-dev libncursesw5-dev build-essential

# Fedora/RHEL
sudo dnf install ncurses-devel gcc-c++

# Arch Linux
sudo pacman -S ncurses base-devel
```

## 📦 Kompilacja

```bash
cd /workspace/tui
g++ -std=c++17 -o bioresonance_tui bioresonance_tui.cpp -lncurses
```

### Szczegóły Implementacji
- **Plik źródłowy**: `bioresonance_tui.cpp` (1061 linii)
- **Standard**: C++17
- **Biblioteki**: ncurses (terminal UI)
- **Kompilacja**: Single-file build

## 🚀 Uruchomienie

```bash
# Podstawowe użycie (domyślny IP i port)
sudo ./bioresonance_tui

# Z podaniem adresu IP urządzenia
sudo ./bioresonance_tui 192.168.1.100

# Z podaniem IP i portu
sudo ./bioresonance_tui 192.168.1.100 5001
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
| `1` | Konfiguracja wybranej końcówki |
| `2` | Wybór trybu pracy |
| `3` | Start terapii |
| `4` | Stop terapii |
| `5` | Odśwież status systemu |
| `E` | Włącz/Wyłącz wybraną końcówkę |
| `F` | Edycja częstotliwości |
| `I` | Edycja intensywności |
| `M` | Zmiana modulacji |

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

## 🛠️ Dodatkowe Końcówki - Propozycje Rozszerzeń

### Potencjalne nowe typy końcówek:

1. **Cewka Helmholtsa**
   - Pole jednorodne dla badań naukowych
   - Precyzyjna kalibracja natężenia pola

2. **Aplikator Uszny (Otic)**
   - Małe cewki do aplikacji usznych
   - Wysokie częstotliwości (>100 kHz)

3. **Elektrody Kontaktowe**
   - Bezpośredni kontakt ze skórą
   - Niskie impedancje

4. **Aplikator Okrężny**
   - Elastyczna cewka do owijania
   - Kończyny, głowa

5. **Matrix LED + EMF**
   - Połączenie światła i pola EMF
   - Terapia fotoniczno-elektromagnetyczna

6. **Termo-EMF Probe**
   - Zintegrowany element grzewczy
   - Terapia cieplno-elektromagnetyczna

7. **Wibracyjna Końcówka**
   - Mechaniczne wibracje + EMF
   - Terapia multisensoryczna

8. **Biofeedback Sensor**
   - Pomiar GSR, HRV podczas terapii
   - Automatyczna adaptacja parametrów

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
sudo ./bioresonance_tui 192.168.1.100

# 2. Wybierz kanał 1 (cewka płaska)
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

## 🐛 Rozwiązywanie Problemów

### Błąd połączenia
```
Rozwiązanie: Sprawdź czy Arduino jest podłączone do sieci
i ma poprawny adres IP. Pinguj urządzenie: ping 192.168.1.100
```

### Brak odpowiedzi
```
Rozwiązanie: Restart Arduino, sprawdź kabel Ethernet,
zweryfikuj konfigurację sieciową
```

### Błąd kompilacji
```
Rozwiązanie: Zainstaluj brakujące biblioteki:
sudo apt-get install libncurses5-dev libncursesw5-dev
```

## 📄 Licencja

MIT License - patrz główny plik LICENSE projektu ResoNet-Nano.

## 👨‍💻 Autor

ResoNet Development Team - Profesjonalne Systemy Biorezonansu

---

**Wersja**: 1.0  
**Data**: 2024  
**Kompatybilność**: ResoNet-Nano Firmware v4.0+
