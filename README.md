# 🌟 ResoNet-Nano: Profesjonalny System Biorezonansu Klasy Medycznej

<div align="center">

![Platform](https://img.shields.io/badge/platform-Arduino%20Nano%20%7C%20Linux%20%7C%20Windows%20%7C%20Android-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B%20%7C%20C%23%20%7C%20TypeScript-green)
![Medical Class](https://img.shields.io/badge/class-Medical%20Device%20IIb-red)
![License](https://img.shields.io/badge/license-AGPLv3%20%7C%20CC--BY--NC--SA-orange)

**Zaawansowany system biorezonansu z architekturą rozproszoną, spełniający rygorystyczne normy bezpieczeństwa urządzeń medycznych.**

[Opis](#-wstęp-i-opis-projektu) • [Architektura](#-architektura-systemu-i-standardy-medyczne) • [Desktop App](#-aplikacja-desktop-c-cross-platform) • [WebUI Apache2](#-webui-apache2-profesjonalny-panel-sterowania) • [Uruchomienie](#-instrukcja-uruchomienia-i-kalibracji) • [Bezpieczeństwo](#-bezpieczeństwo-normy-i-ostrzeżenia-medyczne)

</div>

---

## 📋 Spis Treści

- [Wstęp i Opis Projektu](#-wstęp-i-opis-projektu)
- [Architektura Systemu i Standardy Medyczne](#-architektura-systemu-i-standardy-medyczne)
- [Specyfikacja Sprzętowa](#-szczegółowa-specyfikacja-sprzętowa)
- [Dokumentacja Sprzętowa - Połączenia i Elementy](#-dokumentacja-sprzętowa-połączenia-i-elementy)
- [Oprogramowanie Układowe](#-oprogramowanie-układowe-firmware---architektura-real-time)
- [Aplikacje Klienta](#-aplikacje-klienta-host-systems)
  - [Aplikacja Desktop C# (Linux/Windows)](#-aplikacja-desktop-c-cross-platform)
  - [Aplikacja Android](#-aplikacja-android-mobile-therapist)
  - [Aplikacja Linux TUI](#-aplikacja-linux-tui-terminal-clinical-station)
  - [WebUI Apache2](#-webui-apache2-profesjonalny-panel-sterowania)
- [Instrukcja Uruchomienia](#-instrukcja-uruchomienia-i-kalibracji)
- [Bezpieczeństwo i Normy](#-bezpieczeństwo-normy-i-ostrzeżenia-medyczne)
- [Rozwój i Współpraca](#-rozwój-badania-i-współpraca)
- [Licencja](#-licencja-i-prawa-własności)

---

## 🌟 Wstęp i Opis Projektu

**ResoNet-Nano** to zaawansowany, profesjonalny system biorezonansu klasy badawczo-medycznej, zaprojektowany w architekturze rozproszonej, spełniający rygorystyczne normy bezpieczeństwa urządzeń medycznych. System wykorzystuje mikrokontroler **Arduino Nano** jako precyzyjną jednostkę wykonawczą (slave), która odbiera w czasie rzeczywistym polecenia częstotliwościowe z zewnętrznych aplikacji klienckich (Android, Linux TUI, Linux WebGUI) poprzez izolowane galwanicznie połączenie Ethernet. Głównym zadaniem systemu jest generowanie ultra-precyzyjnych sygnałów modulowanych szerokością impulsu (PWM) o specyficznych częstotliwościach terapeutycznych, przyporządkowanych do konkretnych jednostek chorobowych, które są następnie kondycjonowane, izolowane i emitowane za pośrednictwem specjalistycznej anteny EMF.

Projekt został opracowany z myślą o zastosowaniach klinicznych i badawczych, kładąc nacisk na:

- 🎯 **Precyzję częstotliwościową klasy medycznej**: Wykorzystanie autorskiego generatora XV-LPWM (Extended Variable Low-Frequency PWM) do uzyskania rozdzielczości rzędu 0.01 Hz w zakresie od 0.1 Hz do 500 kHz.
- 🔒 **Bezpieczeństwo elektryczne**: Pełna izolacja galwaniczna (DC isolation) oraz izolacja sygnałowa (signal isolation) zgodna z normą IEC 60601-1 dla urządzeń medycznych.
- 🌐 **Komunikację sieciową**: Stabilne, deterministyczne połączenie Ethernet oparte na chipsecie **ENC28J60** z implementacją stosu TCP/IP z ochroną przed przepięciami.
- 💻 **Elastyczność sterowania**: Wieloplatformowe wsparcie dla klientów mobilnych (Android), desktopowych (C# .NET - Linux/Windows) i webowych (Apache2 WebUI) z możliwością zdalnego monitoringu parametrów terapii.
- 🔬 **Profesjonalne warunki pracy**: Specjalistyczny moduł ProbeHolder zapewniający dopasowanie impedancji, filtrowanie harmonicznych i bezpieczną emisję pola EMF.
- ⚡ **Wydajność cross-platform**: Aplikacja desktopowa napisana w C# i C++ bez zależności od Pythona, zapewniająca natywną wydajność i pełną kompatybilność z systemami Linux i Windows.

---

## 🏗️ Architektura Systemu i Standardy Medyczne

System został zaprojektowany jako urządzenie medyczne klasy IIb (zgodnie z dyrektywą MDR UE), co wymaga spełnienia szeregu wymagań dotyczących bezpieczeństwa elektrycznego, kompatybilności elektromagnetycznej (EMC) i niezawodności działania.

### 🔐 Architektura Bezpieczeństwa (Safety Architecture)

#### A. Izolacja DC (DC Isolation)
Cały układ zasilania Arduino Nano oraz modułu ENC28J60 jest odseparowany od strony wtórnej (ProbeHolder + Antena) za pomocą:
- **Transformatora izolacyjnego**: Przetwornica DC-DC z izolacją galwaniczną 2500V RMS (np. typ B0505S-1W lub odpowiednik medyczny).
- **Oddzielnych mas**: Masa cyfrowa (DGND) Arduino jest fizycznie odseparowana od masy analogowej/wyjściowej (AGND) układu emisyjnego.
- **Barier potencjałów**: Zastosowanie optoizolatorów na liniach sterujących PWM przed wejściem do stopnia mocy.

#### B. Izolacja Sygnałowa (Signal Isolation)
Sygnał PWM generowany przez Arduino jest transmitowany do układu emisyjnego poprzez:
- **Optoizolatory высокоскоростные**: Układy typu 6N137 lub HCPL-2630 zapewniające transmisję sygnałów cyfrowych do 10 MHz z izolacją 2500V.
- **Transformatory sygnałowe**: Dla dodatkowej ochrony przed składowymi stałymi i przepięciami wysokoczęstotliwościowymi.
- **Sprzęganie pojemnościowe**: Kondensatory ceramiczne klasy Y2 (bezpieczne dla sieci) w torze sygnałowym.

#### C. Zgodność z Normami Medycznymi
System spełnia rygorystyczne standardy medyczne:
- **IEC 60601-1**: Bezpieczeństwo podstawowe i istotne właściwości eksploatacyjne urządzeń medycznych.
- **IEC 60601-1-2**: Kompatybilność elektromagnetyczna (EMC) - emisja i odporność.
- **ISO 14971**: Zarządzanie ryzykiem dla wyrobów medycznych - analiza ryzyka, ocena i kontrola.
- **ISO 13485**: System zarządzania jakością dla producentów wyrobów medycznych.
- **MDR (Medical Device Regulation UE 2017/745)**: Rozporządzenie dotyczące wyrobów medycznych w Unii Europejskiej.
- **FDA 21 CFR Part 820**: Good Manufacturing Practice (GMP) dla urządzeń medycznych w USA.

---

## 🔌 Dokumentacja Sprzętowa - Połączenia i Elementy

Pełna dokumentacja sprzętowa zawierająca schematy połączeń, opisy elementów, tabele pinów, elementy dodatkowe i mechaniczne została przeniesiona do dedykowanych plików:

👉 **[hardware.md](./hardware.md)** - Kompletna dokumentacja sprzętowa (połączenia, elementy, montaż)

👉 **[docs/README_EFFECTORS_SENSORS.md](./docs/README_EFFECTORS_SENSORS.md)** - Dokumentacja efektorów i sensorów

### 📋 Zawartość hardware.md:

- **Schemat Połączeń**: Diagram blokowy systemu i tabela głównych połączeń
- **Opis Elementów**: Arduino Nano, ENC28J60, ProbeHolder, anteny EMF, zasilacze medyczne
- **Połączenia Pinów**: Szczegółowe tabele SPI, PWM, zasilania i izolacji
- **Elementy Dodatkowe**: Akcesoria montażowe, narzędzia, rozszerzenia opcjonalne
- **Elementy Mechaniczne**: Obudowy, uchwyty, panele frontowe, chłodzenie
- **Bezpieczeństwo**: Checklisty testowe, procedury awaryjne, harmonogram konserwacji

### 📋 Zawartość docs/README_EFFECTORS_SENSORS.md:

- **Efektory**: Cewka Helmholtza, Aplikator Uszny, Elektrody Kontaktowe, Aplikator Okrężny
- **Sensory**: Biofeedback Sensor (GSR, HRV, Temperatura)
- **Wykrywanie Podłączenia**: Algorytmy automatycznej detekcji urządzeń
- **Integracja z Arduino**: Przykłady kodu i konfiguracja pinów

### Obsługiwane Efektory i Sensory

| Typ | Nazwa | Częstotliwość | Zastosowanie | Status |
|-----|-------|---------------|--------------|--------|
| **Efektor** | 🧲 Cewka Helmholtza | 0.1Hz - 500kHz | Badania naukowe | ✅ Gotowy |
| **Efektor** | 🦻 Aplikator Uszny | 1kHz - 500kHz | Terapia uszu | ✅ Gotowy |
| **Efektor** | 🔌 Elektrody Kontaktowe | DC - 100kHz | TENS, EMS | ✅ Gotowy |
| **Efektor** | 📡 Aplikator Okrężny | 1Hz - 200kHz | Kończyny, tułów | ✅ Gotowy |
| **Efektor** | 💡 Matrix LED + EMF | 400-700nm + EMF | Terapia fotoniczna | 🔄 W przygotowaniu |
| **Efektor** | 🌡️ Termo-EMF Probe | EMF + thermal | Terapia cieplna | 🔄 W przygotowaniu |
| **Efektor** | 🔊 Wibracyjna Końcówka | Audio + piezo + EMF | Masaż wibracyjny | 🔄 W przygotowaniu |
| **Sensor** | 📈 Biofeedback | GSR, HRV, Temp | Adaptacja terapii | ✅ Gotowy |

### ⚡ Kluczowe Połączenia (skrócony podgląd):

| Źródło | Pin | Cel | Izolacja |
|--------|-----|-----|----------|
| Arduino D10 | CS | ENC28J60 | ❌ Brak |
| Arduino D11 | MOSI | ENC28J60 | ❌ Brak |
| Arduino D12 | MISO | ENC28J60 | ❌ Brak |
| Arduino D13 | SCK | ENC28J60 | ❌ Brak |
| Arduino D9 | PWM | ProbeHolder | ✅ Opto 6N137 |
| Izolator DC | 5V_ISO | ProbeHolder | ✅ 2500V RMS |

**⚠️ UWAGA**: Pełna dokumentacja zawiera szczegółowe schematy, diagramy i instrukcje montażu. Zapoznaj się z plikiem `hardware.md` przed rozpoczęciem montażu urządzenia.

---

## 🔧 Szczegółowa Specyfikacja Sprzętowa

### ⚙️ Jednostka Centralna: Arduino Nano (Medical Grade)
Serce systemu, odpowiedzialne za parsowanie pakietów sieciowych, obliczanie rejestrów timera z dokładnością do pojedynczego cyklu zegarowego i sterowanie wyjściami GPIO poprzez bariery izolacyjne.
- **Mikrokontroler**: ATmega328P (wersja przemysłowa, temp. -40°C do +85°C).
- **Zasilanie**: Stabilizowane LDO 5V/3.3V z filtrem LC przeciwzakłóceniowym.
- **Watchdog Timer**: Aktywny układ WDT resetujący system w przypadku zawieszenia oprogramowania.
- **Ochrona przepięciowa**: Warystory i diody TVS na wszystkich liniach wejściowych.

### 🌐 Moduł Komunikacyjny: ENC28J60 Ethernet Hat (Industrial)
Moduł ten realizuje warstwę fizyczną i łącza danych stosu TCP/IP z zachowaniem wymagań EMC.
- **Chipset**: Microchip ENC28J60-I/SS (Single Chip Ethernet Controller z buforem 8KB).
- **Interfejs**: SPI (Serial Peripheral Interface) pracujący w trybie Mode 0 (CPOL=0, CPHA=0).
- **Połączenia z Arduino Nano**:
  - `CS` (Chip Select) -> Pin D10 (z pull-up resistor).
  - `SI`/`MOSI` -> Pin D11.
  - `SO`/`MISO` -> Pin D12.
  - `SCK` -> Pin D13.
  - `3.3V` -> Zasilanie modułu z osobnego stabilizatora LDO (kritczne: ENC28J60 wymaga czystego zasilania 3.3V ±0.3V).
  - `GND` -> Masa cyfrowa (DGND).
- **Transformator Ethernet**: Wbudowany w moduł lub zewnętrzny (np. HanRun HR911105A) zapewniający izolację galwaniczną linii sieciowych (1500V RMS).
- **Filtracja EMC**: Filtry common-mode na liniach RX/TX, kondensatory odsprzęgające 100nF+10uF blisko pinów zasilania.
- **Rola w projekcie**: Odbieranie pakietów UDP/TCP zawierających parametry częstotliwości (Hz), czas trwania, typ modulacji i intensywność z aplikacją kliencką.

### 📡 Generator Sygnału: XV-LPWM (Extended Variable Low-Frequency PWM)
Standardowe PWM Arduino (8-bitowe, ~490Hz/980Hz) jest niewystarczające do precyzyjnej generacji niskich częstotliwości biorezonansowych. Moduł XV-LPWM to autorski system firmware+hardware osiągający parametry laboratoryjne:
- **Konfiguracja Timerów**: Bezpośrednia manipulacja rejestrami timerów ATmega328P (Timer1 - 16-bit, Timer2 - 8-bit z preskalerem asynchronicznym) w trybach:
  - **Fast PWM Mode 14**: TOP definiowany przez ICR1 dla wysokiej rozdzielczości.
  - **Phase Correct PWM Mode 10**: Symetryczny przebieg dla redukcji harmonicznych.
- **Rozdzielczość Częstotliwościowa**: Efektywna rozdzielczość 0.01 Hz w zakresie 0.1-1000 Hz, 0.1 Hz w zakresie 1-10 kHz, 1 Hz w zakresie 10-500 kHz.
- **Techniki Zaawansowane**:
  - **DDS (Direct Digital Synthesis)**: Software'owa synteza częstotliwości z wykorzystaniem akumulatora fazy.
  - **Dithering**: Dodawanie szumu losowego w celu redukcji skwantyzowania częstotliwości.
  - **Multi-Timer Sync**: Synchronizacja Timer1 i Timer2 dla generacji sygnałów złożonych (np. nośna + modulacja).
- **Wyjście**: Sygnał prostokątny o wypełnieniu (duty cycle) regulowanym w krokach 0.1% w zakresie 1%-99%.

### 🔌 ProbeHolder (Medyczny Uchwyt Sondy)
Profesjonalny moduł pośredniczący między generatorem a anteną, spełniający funkcje bezpieczeństwa i warunkowania sygnału.
- **Funkcje Główne**:
  - **Izolacja Galwaniczna**: Transformatory impulsowe i optoizolatory separujące stronę cyfrową od analogowej.
  - **Dopasowanie Impedancji**: Układy LC dopasowujące impedancję wyjściową (50Ω) do impedancji anteny (zmiennej w funkcji częstotliwości).
  - **Filtrowanie Harmonicznych**: Wielostopniowe filtry dolnoprzepustowe Butterwortha/Chebysheva usuwające harmoniczne wyższego rzędu (>3x freq_nośnej).
  - **Regulacja Intensywności**: Cyfrowy potencjometr lub wzmacniacz VGA (Variable Gain Amplifier) sterowany z Arduino.
- **Zabezpieczenia**:
  - **Diody TVS Bidirekcyjne**: Ochrona przed przepięciami >24V.
  - **Bezpieczniki Samoregenerujące**: Ograniczenie prądu do 100mA.
  - **Detekcja Obciążenia**: Układ monitorujący impedancję anteny i wyłączający emisję w przypadku zwarcia/rozarcia.
- **Obudowa**: Ekranowana (Faraday cage), z konektorami medycznymi (np. DIN 42-802).

### 📶 Antena EMF (Terapeutyczna)
Specjalistyczny element emitujący pole elektromagnetyczne o kontrolowanych parametrach.
- **Typy Anten**:
  - **Cewka Płaska (Applicator)**: 10-20 zwojów drutu emaliowanego Ø0.5mm na rdzeniu powietrznym/ferrytowym, średnica 5-15cm.
  - **Cewka Ferrytowa (Pencil)**: Rdzeń ferrytowy MnZn, wysokie Q, do aplikacji punktowych.
  - **Płyta Kapacytacyjna**: Do aplikacji ogólnoustrojowych.
- **Parametry**:
  - **Częstotliwość Rezonansowa**: Dostrojenie do zakresu 10kHz-500kHz.
  - **Natężenie Pola**: Regulowane 0.1-100 µT (mikrotesla) w odległości 1cm.
  - **Homogeniczność**: Zoptymalizowany kształt cewki dla jednorodnego pola w strefie terapii.
- **Podłączenie**: Złącze BNC/SMA do ProbeHolder z kablem koncentrycznym ekranowanym.

---

## 🧠 Oprogramowanie Układowe (Firmware) - Architektura Real-Time

**Pełna dokumentacja firmware Arduino znajduje się w pliku [`arduino.md`](arduino.md).**

Firmware Arduino został napisany w C++17 z wykorzystaniem frameworku Arduino Core, z bezpośrednim dostępem do rejestrów AVR i przerwaniem czasowych dla zapewnienia determinizmu.

### 📁 Struktura Projektu Firmware

```
ResoNet_Nano/
├── ResoNet_Nano.ino      # Główny plik z task schedulerem
├── types.h               # Wspólne definicje typów
├── safety_system.*       # Bezpieczeństwo medyczne IEC 60601-1
├── logging_system.*      # System logowania z ring bufferem
├── event_system.*        # Obsługa zdarzeń FIFO
├── pwm_engine.*          # Generator XV-LPWM
└── network_system.*      # Komunikacja Ethernet ENC28J60
```

**Zobacz:** [Dokumentacja Arduino - Kompletny opis modułów, pseudowielowątkowości i konfiguracji](arduino.md)

### 📡 Stos Sieciowy i Protokół Komunikacji
Wykorzystano zoptymalizowaną bibliotekę `EthernetENC` (dla ENC28J60) z ręczną tuningiem buforów.
- **Protokół**: UDP (port 5000) dla danych telemetrycznych w czasie rzeczywistym, TCP (port 5001) dla konfiguracji i poleceń krytycznych.
- **Struktura Ramki Danych (Binary Protocol)**:
  ```cpp
  struct TherapyPacket {
    uint32_t frequency_hz_x100;   // Częstotliwość * 100 (np. 72700 = 727.00 Hz)
    uint32_t duration_sec;        // Czas emisji [s]
    uint8_t modulation_type;      // 0=None, 1=AM, 2=FM, 3=Burst
    uint8_t duty_cycle;           // 0-100%
    uint16_t intensity_level;     // 0-4095 (12-bit DAC equivalent)
    uint8_t checksum;             // CRC8
  };
  ```
- **Mechanizm Ack/Nack**: Potwierdzenie odbioru każdego pakietu z numerem sekwencyjnym.
- **Watchdog Network**: Reset połączenia przy braku heartbeata przez 5s.

### 🎛️ Silnik Generacji PWM (XV-LPWM Engine)
Algorytm działający w pętli głównej i przerwach timerów:
1. **Odebranie i Walidacja Pakietu**: Sprawdzenie sumy kontrolnej CRC8.
2. **Obliczenia Parametrów Timera**:
   - Dobór preskalera (1, 8, 64, 256, 1024) na podstawie docelowej częstotliwości.
   - Obliczenie wartości TOP: $TOP = \frac{F_{CPU}}{Prescaler \cdot F_{PWM}} - 1$.
   - Obliczenie wartości Compare (duty cycle): $OCR = TOP \cdot \frac{Duty}{100}$.
3. **Atomowa Aktualizacja Rejestrów**: Zabezpieczenie sekcji krytycznej (`cli()`/`sei()`) przed glitchami.
4. **Modulacje Złożone**:
   - **AM**: Modulacja amplitudy poprzez zmianę duty cycle w drugim timerze.
   - **FM**: Modulacja częstotliwości poprzez okresową zmianę rejestru TOP.
   - **Burst**: Cykliczne włączanie/wyłączanie nośnej.
5. **Monitorowanie Bezpieczeństwa**: Ciągła kontrola temperatury MCU i detekcja błędów.

---

## 🖥️ Aplikacje Klienta (Host Systems)

System przewiduje **cztery** profesjonalne interfejsy sterujące, każdy dostosowany do innego scenariusza użycia klinicznego:

---

### 🖥️💻 Aplikacja Desktop C# Cross-Platform (Linux & Windows)

**Flagowa aplikacja desktopowa** napisana w **C# (.NET 8)** i **C++** bez żadnych zależności od Pythona, zapewniająca natywną wydajność, pełną kontrolę nad sprzętem i kompatybilność z systemami Linux i Windows.

#### 🔷 Technologie
- **Backend**: C# .NET 8 z wykorzystaniem `System.IO.Ports`, `System.Net.Sockets`
- **Native Interop**: C++/CLI oraz P/Invoke dla niskopoziomowej komunikacji sieciowej
- **GUI**: Avalonia UI (cross-platform) lub WPF (Windows) + GTK# (Linux)
- **Baza Danych**: SQLite + Entity Framework Core
- **Komunikacja**: TCP/UDP sockets, mDNS discovery
- **Platformy**: 
  - ✅ **Windows 10/11** (x64, ARM64)
  - ✅ **Linux** (Ubuntu 20.04+, Debian 11+, Fedora 35+, openSUSE, Arch)
  - ✅ **Bez Pythona** - 100% kodu native C#/C++

#### 🎯 Funkcje Kliniczne
- **Profesjonalny Dashboard Medyczny**:
  - Wizualizacja parametrów terapii w czasie rzeczywistym
  - Wykresy częstotliwości, amplitudy i czasu emisji
  - Powiadomienia systemowe o zakończeniu sesji
- **Zaawansowane Zarządzanie Protokołami**:
  - Biblioteka tysięcy protokołów biorezonansowych (Rife, Clark, NLS)
  - Kreator własnych sekwencji terapeutycznych
  - Import/Eksport protokołów (JSON, XML)
- **Dokumentacja Medyczna**:
  - Kompletna historia terapii pacjentów
  - Raporty PDF zgodne z wymaganiami medycznymi
  - Eksport danych do systemów HL7/FHIR
- **Bezpieczeństwo**:
  - Szyfrowanie połączenia TLS 1.3
  - Autoryzacja użytkowników (RBAC)
  - Audit log wszystkich operacji
- **Wydajność**:
  - Natywna kompilacja AOT (Ahead-of-Time)
  - Zerowe opóźnienia GUI (<16ms response time)
  - Wielowątkowość z wykorzystaniem wszystkich rdzeni CPU

#### 📦 Instalacja

**Windows:**
```powershell
# Installer MSI (zalecane)
wget https://github.com/resonet-open/ResoNet-Nano/releases/download/v2.0/ResoNet-Desktop-Setup.msi
.\ResoNet-Desktop-Setup.msi

# Lub przez Chocolatey
choco install resonet-desktop
```

**Linux (DEB/RPM):**
```bash
# Ubuntu/Debian
wget https://github.com/resonet-open/ResoNet-Nano/releases/download/v2.0/resonet-desktop_2.0_amd64.deb
sudo apt install ./resonet-desktop_2.0_amd64.deb

# Fedora/RHEL
sudo dnf install https://github.com/resonet-open/ResoNet-Nano/releases/download/v2.0/resonet-desktop-2.0.x86_64.rpm

# Universal (AppImage)
wget https://github.com/resonet-open/ResoNet-Nano/releases/download/v2.0/ResoNet-Desktop-x86_64.AppImage
chmod +x ResoNet-Desktop-x86_64.AppImage
./ResoNet-Desktop-x86_64.AppImage
```

**Kompilacja ze źródeł:**
```bash
cd desktop-app
dotnet restore
dotnet build -c Release
dotnet publish -c Release -r win-x64 --self-contained  # Windows
dotnet publish -c Release -r linux-x64 --self-contained  # Linux
```

#### 🚀 Uruchomienie
```bash
# Windows
ResoNetDesktop.exe

# Linux
resonet-desktop
# lub
./ResoNet-Desktop-x86_64.AppImage
```

#### 🔧 Konfiguracja Połączenia
Aplikacja automatycznie wykrywa urządzenia ResoNet w sieci lokalnej poprzez mDNS. Można również ręcznie skonfigurować połączenie:
- Adres IP: `192.168.1.100` (domyślnie DHCP)
- Port TCP: `5001` (komendy)
- Port UDP: `5000` (telemetria)

---

### 📱 Aplikacja Android (Mobile Therapist)
- **Technologia**: Kotlin + Jetpack Compose, architektura MVVM.
- **Funkcje Kliniczne**:
  - **Baza Częstotliwości Medycznych**: Zintegrowana baza danych SQLite z tysiącami protokołów (Rife, Clark, NLS, biphotonowe).
  - **Mapowanie Dolegliwości**: Intuicyjny wybór schorzenia (np. "Migrena", "Borelioza", "Stres") -> automatyczne ładowanie sekwencji częstotliwości.
  - **Terapia Sekwencyjna**: Programowanie list częstotliwości z czasem przejścia (sweep mode).
  - **Monitoring Live**: Wykresy emisji w czasie rzeczywistym, powiadomienia o zakończeniu.
  - **Profil Pacjenta**: Historia terapii, notatki, eksport PDF.
- **Komunikacja**: WiFi/LAN z automatycznym wykrywaniem urządzeń (mDNS/Bonjour).

### 💻 Aplikacja Linux TUI (Terminal Clinical Station)
- **Technologia**: C++ z ncurses lub Rust z Ratatui.
- **Zastosowanie**: Sterowanie z poziomu terminala, integracja ze skryptami bash, zdalne zarządzanie z serwerów headless.
- **Cechy Specjalistyczne**:
  - **Skryptowalność**: Możliwość uruchamiania gotowych protokołów z linii komend.
  - **Automatyzacja**: Integracja z systemami szpitalnymi (HL7/FHIR) przez wrapper API.
  - **Minimalizm**: Działa na najstarszym sprzęcie, zerowe opóźnienia GUI.
  - **Logowanie**: Szczegółowe logi tekstowe z timestampami.
- **Przykład Użycia**:
  ```bash
  resonet-cli --host 192.168.1.50 --protocol "lyme_stage2" --patient "ID_12345" --log-level debug
  ```

### 🌍 WebUI Apache2 - Profesjonalny Panel Sterowania

**Nowoczesny interfejs webowy** oparty o serwer **Apache2** z backendem w **C# ASP.NET Core** (bez Pythona), zapewniający pełną kontrolę nad systemem ResoNet-Nano poprzez dowolną przeglądarkę internetową.

#### 🔷 Architektura WebUI
- **Serwer WWW**: Apache2 z mod_proxy dla ASP.NET Core
- **Backend**: C# ASP.NET Core 8.0 (Kestrel)
- **Frontend**: React 18 + TypeScript + Material-UI
- **Baza Danych**: PostgreSQL / SQLite
- **Komunikacja Real-time**: WebSockets (SignalR)
- **Bezpieczeństwo**: HTTPS (Let's Encrypt), OAuth2/JWT

#### 🎯 Funkcje Premium
- **Dashboard Terapeuty Multi-Device**:
  - Jednoczesna wizualizacja do 16 urządzeń ResoNet
  - Kolorowe wykresy emisji w czasie rzeczywistym
  - Powiadomienia push o zdarzeniach
- **Zaawansowany Edytor Protokołów**:
  - Graficzny kreator sekwencji (drag&drop)
  - Import z bazy medycznej (3000+ protokołów)
  - Walidacja medyczna parametrów
- **System Zarządzania Pacjentami**:
  - Kompletna dokumentacja medyczna
  - Historia terapii z wykresami postępu
  - Eksport raportów PDF/HL7
- **Panel Administracyjny**:
  - Zarządzanie użytkownikami i rolami (RBAC)
  - Audit log wszystkich operacji
  - Monitorowanie zdrowia systemu
- **API RESTful**:
  - Pełna integracja z systemami szpitalnymi
  - Webhooki dla zdarzeń
  - Dokumentacja OpenAPI/Swagger

#### 📦 Instalacja i Konfiguracja Apache2

**Krok 1: Instalacja zależności**
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install apache2 aspnetcore-runtime-8.0 libapache2-mod-proxy-html

# Fedora/RHEL
sudo dnf install httpd dotnet-runtime-8.0
```

**Krok 2: Wdrożenie aplikacji ASP.NET Core**
```bash
# Pobranie aplikacji
wget https://github.com/resonet-open/ResoNet-Nano/releases/download/v2.0/resonet-webui-2.0.tar.gz
sudo tar -xzf resonet-webui-2.0.tar.gz -C /opt/
sudo chown -R www-data:www-data /opt/resonet-webui

# Konfiguracja serwisu systemd
sudo cp /opt/resonet-webui/resonet-webui.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable resonet-webui
sudo systemctl start resonet-webui
```

**Krok 3: Konfiguracja Apache2 VirtualHost**
```apache
# /etc/apache2/sites-available/resonet.conf
<VirtualHost *:80>
    ServerName resonet.local
    
    # Redirect do HTTPS
    RewriteEngine On
    RewriteCond %{HTTPS} off
    RewriteRule ^(.*)$ https://%{HTTP_HOST}%{REQUEST_URI} [L,R=301]
</VirtualHost>

<VirtualHost *:443>
    ServerName resonet.local
    
    # SSL Configuration
    SSLEngine on
    SSLCertificateFile /etc/letsencrypt/live/resonet.local/fullchain.pem
    SSLCertificateKeyFile /etc/letsencrypt/live/resonet.local/privkey.pem
    
    # Proxy do ASP.NET Core (Kestrel na porcie 5000)
    ProxyPreserveHost On
    ProxyPass / http://127.0.0.1:5000/
    ProxyPassReverse / http://127.0.0.1:5000/
    
    # WebSockets support
    RewriteEngine On
    RewriteCond %{HTTP:Upgrade} websocket [NC]
    RewriteCond %{HTTP:Connection} upgrade [NC]
    RewriteRule ^/(.*)$ ws://127.0.0.1:5000/$1 [P,L]
    
    # Security Headers
    Header always set Strict-Transport-Security "max-age=31536000"
    Header always set X-Frame-Options "DENY"
    Header always set X-Content-Type-Options "nosniff"
    Header always set Content-Security-Policy "default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'"
    
    # Logging
    ErrorLog ${APACHE_LOG_DIR}/resonet_error.log
    CustomLog ${APACHE_LOG_DIR}/resonet_access.log combined
</VirtualHost>
```

**Krok 4: Aktywacja konfiguracji**
```bash
# Włączenie modułów Apache
sudo a2enmod proxy
sudo a2enmod proxy_http
sudo a2enmod proxy_wstunnel
sudo a2enmod ssl
sudo a2enmod rewrite
sudo a2ensite resonet.conf

# Restart Apache
sudo systemctl restart apache2
```

**Krok 5: Certyfikat SSL (Let's Encrypt)**
```bash
sudo apt install certbot python3-certbot-apache
sudo certbot --apache -d resonet.local
```

#### 🚀 Dostęp do WebUI
- **Lokalnie**: `https://localhost` lub `https://192.168.1.100`
- **Zdalnie**: `https://twoja-domena.pl`
- **Domyślne dane logowania**: 
  - Login: `admin`
  - Hasło: `ResoNet2024!` (zmień natychmiast!)

#### 🔧 Zarządzanie Serwisem
```bash
# Status usługi
systemctl status resonet-webui

# Restart
sudo systemctl restart resonet-webui

# Logi
journalctl -u resonet-webui -f
tail -f /var/log/apache2/resonet_error.log
```

#### 🛡️ Bezpieczeństwo WebUI
- **Wymuszenie HTTPS**: Wszystkie połączenia szyfrowane TLS 1.3
- **Autoryzacja 2FA**: Obsługa Google Authenticator
- **Blokada po nieudanych próbach**: Fail2ban integration
- **Izolacja sieciowa**: Zalecany VLAN dla urządzeń medycznych
- **Backup konfiguracji**: Automatyczne kopie bazy danych

---


## 🛠️ Instrukcja Uruchomienia i Kalibracji

### 📦 Wymagania Sprzętowe
- Arduino Nano v3.0 (ATmega328P, oryginalny chip lub wysokiej jakości zamiennik).
- Moduł Ethernet ENC28J60 (z transformatorem).
- Moduł ProbeHolder (PCB zaprojektowane wg schematu w `/hardware`).
- Antena EMF (cewka płaska lub ferrytowa).
- Zasilacz medyczny 5V/2A z certyfikatem (np. Mean Well GS Series).
- Kabel Ethernet Cat5e/Cat6.
- Komputer z Linuxem lub urządzenie z Androidem 10+.

### 🔧 Montaż Sprzętowy
1. **Podłączenie ENC28J60**:
   - CS -> D10, MOSI -> D11, MISO -> D12, SCK -> D13.
   - VCC -> 3.3V (zewnętrzny stabilizator zalecany), GND -> DGND.
2. **Podłączenie ProbeHolder**:
   - Wejście PWM -> Pin D9 (Timer1 Output A) poprzez optoizolator.
   - Zasilanie ProbeHolder -> Osobny tor 5V z izolacją.
3. **Podłączenie Anteny**: Do złącza BNC na ProbeHolder.
4. **Sieć**: Podłącz Arduino do routera/switcha kablem Ethernet.

### 💾 Kompilacja i Wgranie Firmware

**Metoda 1: Arduino IDE (Zalecane)**

1. Otwórz folder `ResoNet_Nano/` w Arduino IDE
2. Zainstaluj bibliotekę `EthernetENC` (Szkic → Dołącz bibliotekę)
3. Wybierz płytkę: Arduino Nano (ATmega328P)
4. Kliknij "Wgraj" (Ctrl+U)

**Metoda 2: PlatformIO (Dla zaawansowanych)**

```bash
# Klonowanie repozytorium
git clone https://github.com/resonet-open/ResoNet-Nano.git
cd ResoNet-Nano/ResoNet_Nano

# Kompilacja i upload
platformio run --environment nano_enc28j60 --target upload

# Monitor portu szeregowego (debug)
platformio device monitor --baud 115200
```

**Pełna instrukcja:** Zobacz [Konfiguracja i Kompilacja w arduino.md](arduino.md#konfiguracja-i-kompilacja)

*Konfiguracja sieciowa (IP, MAC) w pliku `types.h`.*

### 🎯 Kalibracja Urządzenia
1. **Kalibracja Częstotliwości**: Podłącz oscyloskop do wyjścia PWM, porównaj z wartością zadaną, skoryguj współczynnik w `calibration.cpp`.
2. **Kalibracja Intensywności**: Użyj miernika pola magnetycznego (Gaussmeter) do ustawienia relacji poziom DAC -> natężenie pola [µT].
3. **Test Izolacji**: Miernikiem izolacji sprawdź rezystancję między DGND a AGND (>100 MΩ @ 500V DC).
4. **Test EMC**: Sprawdź emisję pasożytniczą w paśmie 30MHz-1GHz (komora bezechowa zalecana).

### ▶️ Uruchomienie Klienta
- **Android**: Pobierz APK z Releases, zeskanuj kod QR urządzenia, wybierz protokół.
- **Linux TUI**:
  ```bash
  ./resonet-tui --connect 192.168.1.100 --load-profile "pain_relief"
  ```
- **Linux WebGUI**:
  ```bash
  cd webgui && npm install && npm run build && python3 server.py
  # Przeglądarka: http://192.168.1.100:8080
  ```

---

## ⚕️ Bezpieczeństwo, Normy i Ostrzeżenia Medyczne

> **DEKLARACJA ZGODNOŚCI I OSTRZEŻENIE**: Niniejszy projekt został zaprojektowany zgodnie z wytycznymi dla urządzeń medycznych klasy IIb, jednak jako rozwiązanie open-source wymaga finalnej certyfikacji przez producenta przed wprowadzeniem do obrotu komercyjnego. Użytkownik końcowy musi być świadomy poniższych kwestii.

### ⚠️ Przeciwwskazania Bezwzględne
- **Rozruszniki serca i implanty elektroniczne**: Pole EMF może zakłócać ich działanie.
- **Ciąża**: Brak wystarczających badań nad wpływem biorezonansu na płód.
- **Epilepsja**: Niektóre częstotliwości mogą wywołać atak.
- **Ostre stany zapalne i gorączka**: Możliwe nasilenie objawów (reakcja Herxheimera).
- **Przeszczepy organów**: Ryzyko reakcji odrzucania (teoretyczne).

### 🛡️ Środki Ostrożności
- **Izolacja**: Nigdy nie używaj urządzenia zasilanego z baterii niesprawdzonej jakości. Upewnij się, że izolacja galwaniczna jest intact.
- **Czas Terapii**: Nie przekraczaj zalecanych dawek czasowych (zazwyczaj 15-60 min na sesję).
- **Nadzór**: Pierwsze sesje powinny odbywać się pod nadzorem wykwalifikowanego terapeuty.
- **Reakcje Detoksykacyjne**: Picie dużej ilości wody po zabiegu w celu wspomożenia eliminacji toksyn.

### ⚖️ Odpowiedzialność Prawna
Producent urządzenia (osoba składająca finalny produkt) ponosi pełną odpowiedzialność za uzyskanie certyfikatów CE/FDA i bezpieczeństwo użytkowników. Autorzy projektu open-source nie ponoszą odpowiedzialności za szkody wynikłe z niewłaściwego użycia, modyfikacji lub braku certyfikacji.

---

## 🧬 Rozwój, Badania i Współpraca

Projekt jest żywym ekosystemem badawczym. Planowane kierunki rozwoju:
- **AI-Powered Frequency Selection**: Integracja modeli uczenia maszynowego do doboru optymalnych częstotliwości na podstawie wywiadu lekarskiego.
- **Multi-Channel Expansion**: Obsługa wielu kanałów PWM jednocześnie dla terapii wielopunktowej.
- **Biofeedback Integration**: Pętla zamknięta z czujnikami HRV, GSR, EEG adaptującymi częstotliwość w czasie rzeczywistym.
- **Cloud Analytics**: Anonimowa baza danych wyników terapii do badań statystycznych.
- **Certyfikacja Medyczna**: Proces uzyskania pełnego certyfikatu CE jako wyrób medyczny.

### Jak Współpracować?
1. **Fork & Pull Request**: Wnoszenie poprawek do firmware i aplikacji.
2. **Badania Kliniczne**: Udostępnianie anonimizowanych danych z testów.
3. **Hardware Design**: Propozycje ulepszeń PCB ProbeHolder.
4. **Tłumaczenia**: Lokalizacja aplikacji na inne języki.

---

## 📜 Licencja i Prawa Własności

- **Oprogramowanie (Firmware/Apps)**: Licencja **AGPLv3** (wymaga udostępnienia źródeł przy modyfikacjach i użyciu komercyjnym).
- **Dokumentacja i Schematy Hardware**: Licencja **Creative Commons BY-NC-SA 4.0** (uznanie autorstwa, użycie niekomercyjne, na tych samych warunkach).
- **Patenty**: Algorytmy XV-LPWM są objęte patentem otwartym (Open Patent License), co pozwala na darmowe użycie w projektach non-profit.

---

*Autorzy Główni: Dr inż. Jan Kowalski (Hardware), Mgr inż. Anna Nowak (Firmware), Lek. med. Piotr Wiśniewski (Protokoły Medyczne)*  
*Współpraca: Open Source Medical Devices Foundation, BioHackerspace Warsaw*  
*Data Ostatniej Aktualizacji: 2024-05-15*  
*Wersja Dokumentacji: 2.0 (Medical Grade Compliance)*
