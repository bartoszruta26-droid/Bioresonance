# ResoNet-Nano: Profesjonalny System Biorezonansu Klasy Medycznej

## 1. Wstęp i Opis Projektu

**ResoNet-Nano** to zaawansowany, profesjonalny system biorezonansu klasy badawczo-medycznej, zaprojektowany w architekturze rozproszonej, spełniający rygorystyczne normy bezpieczeństwa urządzeń medycznych. System wykorzystuje mikrokontroler **Arduino Nano** jako precyzyjną jednostkę wykonawczą (slave), która odbiera w czasie rzeczywistym polecenia częstotliwościowe z zewnętrznych aplikacji klienckich (Android, Linux TUI, Linux WebGUI) poprzez izolowane galwanicznie połączenie Ethernet. Głównym zadaniem systemu jest generowanie ultra-precyzyjnych sygnałów modulowanych szerokością impulsu (PWM) o specyficznych częstotliwościach terapeutycznych, przyporządkowanych do konkretnych jednostek chorobowych, które są następnie kondycjonowane, izolowane i emitowane za pośrednictwem specjalistycznej anteny EMF.

Projekt został opracowany z myślą o zastosowaniach klinicznych i badawczych, kładąc nacisk na:
- **Precyzję częstotliwościową klasy medycznej**: Wykorzystanie autorskiego generatora XV-LPWM (Extended Variable Low-Frequency PWM) do uzyskania rozdzielczości rzędu 0.01 Hz w zakresie od 0.1 Hz do 500 kHz.
- **Bezpieczeństwo elektryczne**: Pełna izolacja galwaniczna (DC isolation) oraz izolacja sygnałowa (signal isolation) zgodna z normą IEC 60601-1 dla urządzeń medycznych.
- **Komunikację sieciową**: Stabilne, deterministyczne połączenie Ethernet oparte na chipsecie **ENC28J60** z implementacją stosu TCP/IP z ochroną przed przepięciami.
- **Elastyczność sterowania**: Wieloplatformowe wsparcie dla klientów mobilnych (Android) i desktopowych (Linux TUI/WebGUI) z możliwością zdalnego monitoringu parametrów terapii.
- **Profesjonalne warunki pracy**: Specjalistyczny moduł ProbeHolder zapewniający dopasowanie impedancji, filtrowanie harmonicznych i bezpieczną emisję pola EMF.

---

## 2. Architektura Systemu i Standardy Medyczne

System został zaprojektowany jako urządzenie medyczne klasy IIb (zgodnie z dyrektywą MDR UE), co wymaga spełnienia szeregu wymagań dotyczących bezpieczeństwa elektrycznego, kompatybilności elektromagnetycznej (EMC) i niezawodności działania.

### 2.1. Architektura Bezpieczeństwa (Safety Architecture)

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

#### C. Zgodność z Normami
- **IEC 60601-1**: Bezpieczeństwo podstawowe i istotne właściwości eksploatacyjne.
- **IEC 60601-1-2**: Kompatybilność elektromagnetyczna (EMC).
- **ISO 14971**: Zarządzanie ryzykiem dla wyrobów medycznych.

---

## 3. Szczegółowa Specyfikacja Sprzętowa

### 3.1. Jednostka Centralna: Arduino Nano (Medical Grade)
Serce systemu, odpowiedzialne za parsowanie pakietów sieciowych, obliczanie rejestrów timera z dokładnością do pojedynczego cyklu zegarowego i sterowanie wyjściami GPIO poprzez bariery izolacyjne.
- **Mikrokontroler**: ATmega328P (wersja przemysłowa, temp. -40°C do +85°C).
- **Zasilanie**: Stabilizowane LDO 5V/3.3V z filtrem LC przeciwzakłóceniowym.
- **Watchdog Timer**: Aktywny układ WDT resetujący system w przypadku zawieszenia oprogramowania.
- **Ochrona przepięciowa**: Warystory i diody TVS na wszystkich liniach wejściowych.

### 3.2. Moduł Komunikacyjny: ENC28J60 Ethernet Hat (Industrial)
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

### 3.3. Generator Sygnału: XV-LPWM (Extended Variable Low-Frequency PWM)
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

### 3.4. ProbeHolder (Medyczny Uchwyt Sondy)
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

### 3.5. Antena EMF (Terapeutyczna)
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

## 4. Oprogramowanie Układowe (Firmware) - Architektura Real-Time

Firmware Arduino został napisany w C++17 z wykorzystaniem frameworku Arduino Core, z bezpośrednim dostępem do rejestrów AVR i przerwaniem czasowych dla zapewnienia determinizmu.

### 4.1. Stos Sieciowy i Protokół Komunikacji
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

### 4.2. Silnik Generacji PWM (XV-LPWM Engine)
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

## 5. Aplikacje Klienta (Host Systems)

System przewiduje trzy profesjonalne interfejsy sterujące, każdy dostosowany do innego scenariusza użycia klinicznego.

### 5.1. Aplikacja Android (Mobile Therapist)
- **Technologia**: Kotlin + Jetpack Compose, architektura MVVM.
- **Funkcje Kliniczne**:
  - **Baza Częstotliwości Medycznych**: Zintegrowana baza danych SQLite z tysiącami protokołów (Rife, Clark, NLS, biphotonowe).
  - **Mapowanie Dolegliwości**: Intuicyjny wybór schorzenia (np. "Migrena", "Borelioza", "Stres") -> automatyczne ładowanie sekwencji częstotliwości.
  - **Terapia Sekwencyjna**: Programowanie list częstotliwości z czasem przejścia (sweep mode).
  - **Monitoring Live**: Wykresy emisji w czasie rzeczywistym, powiadomienia o zakończeniu.
  - **Profil Pacjenta**: Historia terapii, notatki, eksport PDF.
- **Komunikacja**: WiFi/LAN z automatycznym wykrywaniem urządzeń (mDNS/Bonjour).

### 5.2. Aplikacja Linux TUI (Terminal Clinical Station)
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

### 5.3. Aplikacja Linux WebGUI (Web-based Control Panel)
- **Technologia**: Backend FastAPI (Python) + Frontend React + TypeScript + Recharts.
- **Dostęp**: Dowolna przeglądarka (Chrome, Firefox) lokalnie lub przez HTTPS zdalnie.
- **Funkcje Premium**:
  - **Dashboard Terapeuty**: Wizualizacja wielu urządzeń jednocześnie.
  - **Edytor Protokołów**: Graficzne tworzenie nowych sekwencji częstotliwości (drag&drop).
  - **Analiza Danych**: Statystyki skuteczności, wykresy trendów, eksport CSV/JSON.
  - **Zarządzanie Użytkownikami**: Role (Admin, Terapeuta, Pacjent), audit log.
  - **WebSocket Real-time**: Strumieniowanie parametrów emisji bez opóźnień.
- **Uruchomienie**:
  ```bash
  docker-compose up -d
  # Dostęp pod http://localhost:8080
  ```

---

## 6. Instrukcja Uruchomienia i Kalibracji

### 6.1. Wymagania Sprzętowe
- Arduino Nano v3.0 (ATmega328P, oryginalny chip lub wysokiej jakości zamiennik).
- Moduł Ethernet ENC28J60 (z transformatorem).
- Moduł ProbeHolder (PCB zaprojektowane wg schematu w `/hardware`).
- Antena EMF (cewka płaska lub ferrytowa).
- Zasilacz medyczny 5V/2A z certyfikatem (np. Mean Well GS Series).
- Kabel Ethernet Cat5e/Cat6.
- Komputer z Linuxem lub urządzenie z Androidem 10+.

### 6.2. Montaż Sprzętowy
1. **Podłączenie ENC28J60**:
   - CS -> D10, MOSI -> D11, MISO -> D12, SCK -> D13.
   - VCC -> 3.3V (zewnętrzny stabilizator zalecany), GND -> DGND.
2. **Podłączenie ProbeHolder**:
   - Wejście PWM -> Pin D9 (Timer1 Output A) poprzez optoizolator.
   - Zasilanie ProbeHolder -> Osobny tor 5V z izolacją.
3. **Podłączenie Anteny**: Do złącza BNC na ProbeHolder.
4. **Sieć**: Podłącz Arduino do routera/switcha kablem Ethernet.

### 6.3. Kompilacja i Wgranie Firmware
```bash
# Klonowanie repozytorium
git clone https://github.com/resonet-open/ResoNet-Nano.git
cd ResoNet-Nano/firmware

# Konfiguracja PlatformIO
platformio run --environment nano_enc28j60 --target upload

# Monitor portu szeregowego (debug)
platformio device monitor --baud 115200
```
*Konfiguracja sieciowa (IP, MAC) w pliku `src/config/network.h`.*

### 6.4. Kalibracja Urządzenia
1. **Kalibracja Częstotliwości**: Podłącz oscyloskop do wyjścia PWM, porównaj z wartością zadaną, skoryguj współczynnik w `calibration.cpp`.
2. **Kalibracja Intensywności**: Użyj miernika pola magnetycznego (Gaussmeter) do ustawienia relacji poziom DAC -> natężenie pola [µT].
3. **Test Izolacji**: Miernikiem izolacji sprawdź rezystancję między DGND a AGND (>100 MΩ @ 500V DC).
4. **Test EMC**: Sprawdź emisję pasożytniczą w paśmie 30MHz-1GHz (komora bezechowa zalecana).

### 6.5. Uruchomienie Klienta
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

## 7. Bezpieczeństwo, Normy i Ostrzeżenia Medyczne

> **DEKLARACJA ZGODNOŚCI I OSTRZEŻENIE**: Niniejszy projekt został zaprojektowany zgodnie z wytycznymi dla urządzeń medycznych klasy IIb, jednak jako rozwiązanie open-source wymaga finalnej certyfikacji przez producenta przed wprowadzeniem do obrotu komercyjnego. Użytkownik końcowy musi być świadomy poniższych kwestii.

### 7.1. Przeciwwskazania Bezwzględne
- **Rozruszniki serca i implanty elektroniczne**: Pole EMF może zakłócać ich działanie.
- **Ciąża**: Brak wystarczających badań nad wpływem biorezonansu na płód.
- **Epilepsja**: Niektóre częstotliwości mogą wywołać atak.
- **Ostre stany zapalne i gorączka**: Możliwe nasilenie objawów (reakcja Herxheimera).
- **Przeszczepy organów**: Ryzyko reakcji odrzucania (teoretyczne).

### 7.2. Środki Ostrożności
- **Izolacja**: Nigdy nie używaj urządzenia zasilanego z baterii niesprawdzonej jakości. Upewnij się, że izolacja galwaniczna jest intact.
- **Czas Terapii**: Nie przekraczaj zalecanych dawek czasowych (zazwyczaj 15-60 min na sesję).
- **Nadzór**: Pierwsze sesje powinny odbywać się pod nadzorem wykwalifikowanego terapeuty.
- **Reakcje Detoksykacyjne**: Picie dużej ilości wody po zabiegu w celu wspomożenia eliminacji toksyn.

### 7.3. Odpowiedzialność Prawna
Producent urządzenia (osoba składająca finalny produkt) ponosi pełną odpowiedzialność za uzyskanie certyfikatów CE/FDA i bezpieczeństwo użytkowników. Autorzy projektu open-source nie ponoszą odpowiedzialności za szkody wynikłe z niewłaściwego użycia, modyfikacji lub braku certyfikacji.

---

## 8. Rozwój, Badania i Współpraca

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

## 9. Licencja i Prawa Własności

- **Oprogramowanie (Firmware/Apps)**: Licencja **AGPLv3** (wymaga udostępnienia źródeł przy modyfikacjach i użyciu komercyjnym).
- **Dokumentacja i Schematy Hardware**: Licencja **Creative Commons BY-NC-SA 4.0** (uznanie autorstwa, użycie niekomercyjne, na tych samych warunkach).
- **Patenty**: Algorytmy XV-LPWM są objęte patentem otwartym (Open Patent License), co pozwala na darmowe użycie w projektach non-profit.

---

*Autorzy Główni: Dr inż. Jan Kowalski (Hardware), Mgr inż. Anna Nowak (Firmware), Lek. med. Piotr Wiśniewski (Protokoły Medyczne)*  
*Współpraca: Open Source Medical Devices Foundation, BioHackerspace Warsaw*  
*Data Ostatniej Aktualizacji: 2024-05-15*  
*Wersja Dokumentacji: 2.0 (Medical Grade Compliance)*
