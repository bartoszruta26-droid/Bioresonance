# ResoNet Nano - Android Application

## 📱 Opis

Aplikacja Android do obsługi efektorów Arduino Nano z systemu ResoNet. 
Inspiracja: bash_tui, webui oraz plik .ino z projektu ResoNet-Nano.

## 🔧 Architektura

Aplikacja została napisana w **Kotlin** z wykorzystaniem:
- **MVVM Architecture** (Model-View-ViewModel)
- **Android Jetpack Components**:
  - ViewModel z LiveData/StateFlow
  - ViewBinding
  - Lifecycle-aware components
  - Coroutines dla operacji asynchronicznych
- **Material Design** dla UI

### Struktura projektu

```
android_app/
├── app/
│   ├── src/main/
│   │   ├── java/com/resonet/nano/
│   │   │   ├── Types.kt              # Enumy: EffectorType, ModulationType, SystemState
│   │   │   ├── Models.kt             # Data classes: ChannelConfig, SystemStatus, LogEntry
│   │   │   ├── ArduinoCommunication.kt  # Komunikacja TCP z Arduino
│   │   │   ├── MainViewModel.kt      # ViewModel główny
│   │   │   └── MainActivity.kt       # Główna aktywność
│   │   ├── res/
│   │   │   ├── layout/
│   │   │   │   └── activity_main.xml # Layout główny
│   │   │   ├── values/
│   │   │   │   ├── strings.xml       # Stringi
│   │   │   │   ├── colors.xml        # Kolory
│   │   │   │   └── themes.xml        # Motyw
│   │   └── AndroidManifest.xml
│   └── build.gradle
├── build.gradle
└── settings.gradle
```

## 🎯 Funkcjonalności

### 1. Połączenie z Arduino
- TCP/IP przez Ethernet HAT (ENC28J60)
- Konfigurowalny adres IP i port (domyślnie 5001)
- Automatyczne wykrywanie stanu połączenia

### 2. Monitorowanie Statusu
- Czas pracy (uptime)
- Temperatura MCU
- Wolna pamięć RAM
- Stan PWM (ACTIVE/STOPPED)
- Aktualna częstotliwość
- Status sieci
- Wykryty typ efektora
- Stan systemu bezpieczeństwa

### 3. Konfiguracja Kanałów (8 kanałów)
Zgodne z bash_tui i webui:
1. Cewka Płaska (Flat Coil) - 727 Hz
2. Cewka Ferrytowa (Ferrite Rod) - 10 kHz
3. Płyta Kapacytacyjna (Capacitive Plate) - 5 kHz
4. Aplikator Punktowy (Pen Applicator) - 25 kHz
5. Mata EMF (EMF Mat) - 78.3 Hz
6. Podkładka Lokalna (Local Pad) - 1 kHz
7. Pierścień (Ring Applicator) - 500 Hz
8. Niestandardowa (Custom) - 10 Hz

Dla każdego kanału:
- Częstotliwość (0.1 Hz - 500 kHz)
- Cykl pracy (0-100%)
- Intensywność (0-4095)
- Modulacja (NONE, AM, FM, BURST)
- Włącz/Wyłącz

### 4. Sterowanie Terapią
- START terapii
- STOP terapii
- Skanowanie podłączonych efektorów

### 5. Logi Systemowe
- Podgląd logów z Arduino
- Poziomy: VERBOSE, DEBUG, INFO, WARNING, ERROR, FATAL

## 🔌 Protokół Komunikacyjny

Zgodny z webui i bashtui:

### Komendy wysyłane do Arduino:
```
s                  - Pobierz status
START              - Start terapii
STOP               - Stop terapii
l                  - Pobierz logi
d                  - Skanuj urządzenia
CONFIG:c,f,d,i,m   - Konfiguracja kanału
                     c = channel (1-8)
                     f = frequency_x100
                     d = duty_cycle (0-100)
                     i = intensity (0-4095)
                     m = modulation (0-3)
```

### Przykład komendy:
```
CONFIG:1,72700,50,2048,0
```
Konfiguruj kanał 1, 727 Hz, 50% duty cycle, intensywność 2048, bez modulacji

## 📋 Obsługiwane Efektory

Zgodne z `device_detector.h`:

| ID | Typ | Nazwa |
|----|-----|-------|
| 0 | NONE | Brak efektora |
| 1 | HELMHOLTZ | Cewka Helmholtza |
| 2 | OTIC | Aplikator Uszny |
| 3 | CONTACT | Elektrody Kontaktowe |
| 4 | WRAP | Aplikator Okrężny |
| 5 | IR_LED_STRIP | Pasek LED IR |
| 6 | PIEZO_SPEAKER | Głośnik Piezo/Audio |
| 7 | VIBRATOR | Wibreator |

## 🚀 Budowanie

### Wymagania:
- Android Studio Arctic Fox lub nowszy
- JDK 11+
- Android SDK 34 (API level)
- Kotlin 1.9.0+

### Kroki:
1. Otwórz projekt w Android Studio
2. Poczekaj na synchronizację Gradle
3. Podłącz urządzenie Android lub uruchom emulator
4. Kliknij **Run** (Shift+F10)

### Build z linii poleceń:
```bash
cd /workspace/android_app
./gradlew assembleDebug
```

APK będzie dostępne w: `app/build/outputs/apk/debug/app-debug.apk`

## 🔐 Uprawnienia

```xml
<uses-permission android:name="android.permission.INTERNET" />
<uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
<uses-permission android:name="android.permission.ACCESS_WIFI_STATE" />
```

**Uwaga**: `android:usesCleartextTraffic="true"` jest wymagane dla komunikacji TCP z Arduino.

## 🎨 UI/UX

Aplikacja posiada interfejs zgodny z Material Design:
- Karty (CardViews) dla poszczególnych sekcji
- Kolorowanie statusów (czerwony = błąd/rozłączony, zielony = OK/połączony)
- Progress bar podczas operacji
- Snackbar dla komunikatów
- Responsywny layout

## 📝 Różnice względem bash_tui i webui

| Cecha | bash_tui | webui | android_app |
|-------|----------|-------|-------------|
| Platforma | Terminal Linux | Przeglądarka | Android |
| Język | Bash | PHP + JS | Kotlin |
| UI | TUI (ncurses) | HTML/CSS | Material Design |
| Komunikacja | netcat | PHP sockets | Java Sockets |
| Wydajność | Średnia | Wysoka | Wysoka |
| Mobilność | Nie | Tak (responsive) | Tak (native) |

## ⚠️ Bezpieczeństwo

⚠️ **WAŻNE**: To jest aplikacja medyczna klasy IEC 60601-1:
1. Przed użyciem sprawdź izolację galwaniczną
2. Zweryfikuj parametry wyjściowe oscyloskopem
3. Konsultuj się z profesjonalistą
4. Nie używaj bez odpowiednich zabezpieczeń

## 📄 Licencja

MIT License - zgodne z głównym projektem ResoNet-Nano

## 👨‍💻 Autor

ResoNet Development Team

---

**Wersja**: 1.0 (szkielet)  
**Data**: 2024  
**Kompatybilność**: ResoNet-Nano Firmware v4.0+  
**Min. Android**: 7.0 (API 24)
