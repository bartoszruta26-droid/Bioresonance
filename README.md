# ResoNet-Nano: Zaawansowany System Biorezonansu

## 1. Wstęp i Opis Projektu

**ResoNet-Nano** to profesjonalny, otwartoźródłowy projekt urządzenia do biorezonansu, oparty na architekturze rozproszonej. System wykorzystuje mikrokontroler **Arduino Nano** jako jednostkę wykonawczą (slave), która odbiera polecenia częstotliwościowe z zewnętrznych aplikacji klienckich (Android, Linux TUI, Linux WebGUI) poprzez sieć Ethernet. Głównym zadaniem systemu jest generowanie precyzyjnych sygnałów modulowanych szerokością impulsu (PWM) o specyficznych częstotliwościach terapeutycznych, które są kondycjonowane i emitowane za pośrednictwem dedykowanej anteny EMF.

Projekt kładzie nacisk na:
- **Precyzję częstotliwościową**: Wykorzystanie rozszerzonego generatora PWM (XV-LPWM) do uzyskania rozdzielczości wykraczającej poza standardowe możliwości Arduino.
- **Komunikację sieciową**: Stabilne połączenie Ethernet oparte na chipsecie **ENC28J60**.
- **Elastyczność sterowania**: Wieloplatformowe wsparcie dla klientów mobilnych i desktopowych.
- **Bezpieczeństwo i izolację**: Specjalistyczny uchwyt sondy (ProbeHolder) zapewniający odpowiednie warunki pracy dla anteny.

---

## 2. Architektura Systemu

System składa się z dwóch głównych warstw:
1.  **Warstwa Sprzętowa (Hardware/Firmware)**: Mikrokontroler Arduino Nano z modułami peryferyjnymi.
2.  **Warstwa Aplikacyjna (Client/Host)**: Aplikacje dostarczające interfejs użytkownika i logikę doboru częstotliwości.

### 2.1. Schemat Połączeń i Komponenty Sprzętowe

#### A. Jednostka Centralna: Arduino Nano
Serce systemu, odpowiedzialne za parsowanie pakietów sieciowych, obliczanie rejestrów timera i sterowanie wyjściami GPIO.
- **Mikrokontroler**: ATmega328P (lub kompatybilny).
- **Zasilanie**: Stabilizowane 5V (zalecane zewnętrzne źródło ze względu na pobór prądu przez moduł Ethernet).

#### B. Moduł Komunikacyjny: ENC28J60 Ethernet Hat
Moduł ten realizuje warstwę fizyczną i łącza danych stosu TCP/IP.
- **Chipset**: Microchip ENC28J60 (Single Chip Ethernet Controller).
- **Interfejs**: SPI (Serial Peripheral Interface).
- **Połączenia z Arduino Nano**:
  - `CS` (Chip Select) -> Pin D10 (konfigurowalny).
  - `SI`/`MOSI` -> Pin D11.
  - `SO`/`MISO` -> Pin D12.
  - `SCK` -> Pin D13.
  - `3.3V` -> Zasilanie modułu (kritczne: ENC28J60 pracuje na 3.3V, choć piny logiczne są często tolerancyjne, zalecany level-shifter lub bezpośrednie podłączenie jeśli moduł posiada stabilizator).
  - `GND` -> Wspólna masa.
- **Rola w projekcie**: Odbieranie pakietów UDP/TCP zawierających parametry częstotliwości (Hz) i czas trwania impulsu.

#### C. Generator Sygnału: XV-LPWM (Extended Variable Low-Frequency PWM)
Standardowe PWM Arduino (8-bitowe) jest niewystarczające do precyzyjnej generacji niskich częstotliwości biorezonansowych (np. 0.5 Hz - 100 kHz) z wysoką rozdzielczością. Moduł XV-LPWM obejmuje:
- **Konfiguracja Timerów**: Bezpośrednia manipulacja rejestrami timerów ATmega328P (Timer1 i Timer2) w trybie Fast PWM lub Phase Correct PWM z programowalnym preskalerem.
- **Rozdzielczość**: Rozszerzenie efektywnej rozdzielczości do 16-bitów lub więcej poprzez techniki software'owego dzielenia częstotliwości bazowej.
- **Wyjście**: Sygnał prostokątny o wypełnieniu (duty cycle) dostosowanym do charakterystyki danej dolegliwości (często 50% dla fal nośnych, lub zmienne dla modulacji).

#### D. ProbeHolder (Uchwyt Sondy)
Mechaniczno-elektroniczny interfejs między generatorem a anteną.
- **Funkcja**: Izolacja galwaniczna i dopasowanie impedancji.
- **Budowa**: Płytko-złączowa obejmująca układ warunkowania sygnału (np. wzmacniacz operacyjny w konfiguracji bufora, filtr dolnoprzepustowy RC usuwający harmoniczne wyższego rzędu).
- **Zabezpieczenia**: Diody TVS chroniące Arduino przed przepięciami indukowanymi w antenie.

#### E. Antena EMF
Element emitujący pole elektromagnetyczne.
- **Typ**: Cewka płaska (pętla) lub cewka ferrytowa, dostrojona do zakresu częstotliwości roboczych.
- **Podłączenie**: Do wyjścia ProbeHolder.
- **Charakterystyka**: Generowanie zmiennego pola magnetycznego oddziałującego z tkankami biologicznymi.

---

## 3. Oprogramowanie Układowe (Firmware)

Firmware Arduino został napisany w C++ z wykorzystaniem frameworku Arduino, z bezpośrednim dostępem do rejestrów sprzętowych dla optymalizacji czasu rzeczywistego.

### 3.1. Stos Sieciowy
Wykorzystano bibliotekę `UIPEthernet` lub `EthernetENC` (zoptymalizowaną dla ENC28J60), która obsługuje stos TCP/IP.
- **Protokół komunikacji**: UDP (dla niskich opóźnień) lub TCP (dla pewności dostarczenia).
- **Format ramki danych**:
  ```json
  {
    "freq": 727.0,      // Częstotliwość w Hz
    "duration": 60,     // Czas emisji w sekundach
    "modulation": "SQ", // Typ modulacji (SQ=Square, SI=Sine approx, etc.)
    "intensity": 50     // Wypełnienie PWM (0-100%)
  }
  ```

### 3.2. Algorytm Generacji PWM
1. **Odebranie pakietu**: Przerwanie lub pętla główna odczytuje dostępne dane z gniazda sieciowego.
2. **Parsowanie**: Ekstrakcja wartości częstotliwości.
3. **Obliczenia Timerów**:
   - Wzór na rejestr TOP dla trybu Fast PWM:
     $$ TOP = \frac{F_{CPU}}{Prescaler \cdot F_{PWM}} - 1 $$
   - Automatyczny dobór preskalera (1, 8, 64, 256, 1024) w celu utrzymania wysokiej rozdzielczości przy zachowaniu żądanej częstotliwości.
4. **Aktualizacja Rejestrów**: Zapis do `ICR1` (dla Timer1) lub `OCRxA/B`.

---

## 4. Aplikacje Klienta (Host)

System przewiduje trzy typy klientów sterujących:

### 4.1. Aplikacja Android
- **Technologia**: Kotlin/Java lub Flutter.
- **Funkcje**:
  - Baza danych częstotliwości (np. baza Rife, Hulda Clark).
  - Interfejs wyboru dolegliwości (przyporządkowanie choroba -> zestaw częstotliwości).
  - Suwak intensywności i czasu.
  - Komunikacja przez WiFi z modułem ENC28J60 (po skonfigurowaniu adresu IP w sieci lokalnej).

### 4.2. Aplikacja Linux TUI (Text User Interface)
- **Technologia**: Python (biblioteka `urwid` lub `ncurses`) lub C++.
- **Zastosowanie**: Sterowanie z terminala, skrypty automatyzujące terapie, integracja z systemami zdalnego monitoringu.
- **Cechy**: Lekkość, możliwość uruchomienia na Raspberry Pi bez środowiska graficznego.

### 4.3. Aplikacja Linux WebGUI
- **Technologia**: Serwer Flask/Django (Python) lub Node.js + Frontend React/Vue.
- **Dostęp**: Przeglądarka internetowa (lokalnie lub zdalnie).
- **Funkcje**: Wizualizacja przebiegu terapii, wykresy częstotliwości, zarządzanie profilami pacjentów.

---

## 5. Instrukcja Uruchomienia

### 5.1. Wymagania Sprzętowe
- Arduino Nano v3.0 (ATmega328P).
- Moduł Ethernet ENC28J60.
- Przewody połączeniowe (goldpin).
- Antena EMF (gotowa lub samodzielnie nawinięta).
- Komputer z systemem Linux lub urządzenie z Androidem.

### 5.2. Konfiguracja Sprzętowa
1. Podłącz moduł ENC28J60 do Arduino Nano zgodnie z pinoutem SPI (D10-D13).
2. Podłącz wyjście PWM (np. Pin D9 lub D10, zależnie od konfiguracji timera) do wejścia ProbeHolder.
3. Podłącz antenę do ProbeHolder.
4. Podłącz Arduino do sieci LAN kablem Ethernet.

### 5.3. Kompilacja i Wgranie Firmware
```bash
# Klonowanie repozytorium
git clone <repo_url>
cd ResoNet-Nano/firmware

# Instalacja zależności (PlatformIO lub Arduino CLI)
pio run --target upload
```
*Uwaga: Przed kompilacją należy skonfigurować adres IP, maskę podsieci i bramę w pliku `config.h`.*

### 5.4. Uruchomienie Klienta
- **Android**: Zainstaluj aplikację `.apk`, wprowadź adres IP urządzenia ResoNet-Nano, wybierz częstotliwość i naciśnij "Start".
- **Linux TUI**:
  ```bash
  python3 client_tui.py --host 192.168.1.100 --freq 727 --time 60
  ```
- **Linux WebGUI**:
  ```bash
  python3 web_server.py
  # Otwórz przeglądarkę pod adresem http://localhost:5000
  ```

---

## 6. Aspekty Bezpieczeństwa i Ostrzeżenia

> **WAŻNE**: Niniejsze urządzenie jest projektem badawczo-rozwojowym (DIY) i **nie stanowi wyrobu medycznego**. Nie zostało zatwierdzone przez organy regulacyjne (takie jak FDA, CE, URPL) do celów diagnostycznych ani terapeutycznych.

- **Ryzyko elektryczne**: Upewnij się, że układ jest prawidłowo izolowany. Nie dotykaj anteny podczas pracy przy wysokich mocach.
- **Przeciwwskazania**: Osoby z rozrusznikami serca, implantami elektronicznymi, kobiety w ciąży oraz osoby cierpiące na epilepsję nie powinny korzystać z urządzeń generujących pola EMF bez konsultacji z lekarzem.
- **Odpowiedzialność**: Użytkownik korzysta z projektu na własną odpowiedzialność. Autorzy kodu i dokumentacji nie ponoszą odpowiedzialności za ewentualne szkody na zdrowiu lub mieniu.

---

## 7. Rozwój i Współpraca

Projekt jest otwarty na wkład społeczności. Obszary wymagające rozwoju:
- Implementacja bardziej złożonych kształtów fali (sinusoidalnych przy użyciu filtrów DAC).
- Rozbudowa bazy częstotliwości w aplikacjach klienckich.
- Dodanie obsługi WiFi przez mostek UART-WiFi (ESP-01) jako alternatywy dla ENC28J60.
- Kalibracja polowa anteny przy użyciu mierników Gaussa.

## 8. Licencja

Projekt udostępniony na licencji **MIT** (kod źródłowy) oraz **Creative Commons BY-SA 4.0** (dokumentacja i schematy hardware). Szczegóły w plikach `LICENSE` i `HARDWARE_LICENSE`.

---

*Autorzy: Zespół ResoNet Open Source*
*Data ostatniej aktualizacji: 2023-10-27*
