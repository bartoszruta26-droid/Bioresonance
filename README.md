# 🌟 ResoNet-Nano: System Biorezonansu Open Source

<div align="center">

![Platform](https://img.shields.io/badge/platform-Arduino%20Nano%20%7C%20Linux%20%7C%20Android-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B%20%7C%20PHP%20%7C%20Kotlin-green)
![License](https://img.shields.io/badge/license-AGPLv3-orange)
![Version](https://img.shields.io/badge/version-2.0-purple)

**Zaawansowany system biorezonansu z architekturą rozproszoną opartą o Arduino Nano i Ethernet.**

</div>

---

## 📋 Spis Treści

- [Szybka Instalacja](#-szybka-instalacja)
- [Opis Projektu](#-opis-projektu)
- [Architektura](#-architektura)
- [Sprzęt](#-sprzęt)
- [Firmware Arduino](#-firmware-arduino)
- [Aplikacje](#-aplikacje)
  - [GUI Desktop](#-gui-desktop)
  - [WebUI](#-webui)
  - [Android](#-android)
  - [TUI](#-tui)
- [Bezpieczeństwo](#-bezpieczeństwo)
- [Licencja](#-licencja)

---

## 🚀 Szybka Instalacja

Najszybszy sposób instalacji wszystkich komponentów:

```bash
# Pobierz installer
wget https://raw.githubusercontent.com/bartoszruta26-droid/Bioresonance/main/installer.sh
chmod +x installer.sh

# Uruchom installer
sudo ./installer.sh
```

### Opcje installera:
1. **Instalacja firmware Arduino** - kompilacja i upload do płytki
2. **Pobierz projekt z GitHub** - klonowanie repozytorium
3. **Instalacja zależności systemowych** - pakiety potrzebne do kompilacji
4. **Kompilacja GUI Desktop** - aplikacja C++/ImGui
5. **Instalacja WebUI** - Apache + PHP + pliki webui/
6. **Budowanie Android App** - aplikacja Kotlin na urządzenia mobilne
7. **Instalacja TUI** - terminalowy interfejs Bash
8. **Weryfikacja instalacji** - sprawdzenie wszystkich komponentów

---

## 🌟 Opis Projektu

**ResoNet-Nano** to open-source'owy system biorezonansu wykorzystujący Arduino Nano jako jednostkę wykonawczą. System generuje precyzyjne sygnały PWM (0.01 Hz - 500 kHz) emitowane przez antenę EMF.

### Kluczowe cechy:
- 🎯 **Precyzja**: Generator XV-LPWM z rozdzielczością 0.01 Hz
- 🔒 **Bezpieczeństwo**: Izolacja galwaniczna 2500V RMS
- 🌐 **Ethernet**: Komunikacja TCP/UDP przez ENC28J60
- 💻 **Multi-platformowość**: Android, WebUI (PHP), GUI (C++), TUI (Bash)

---

## 🏗️ Architektura

```
┌─────────────────┐     Ethernet      ┌─────────────────────┐
│  Arduino Nano   │◄─────────────────►│  Aplikacja Klienta  │
│  + ENC28J60     │  TCP/UDP          │  - Android          │
│                 │                   │  - WebUI (PHP)      │
└────────┬────────┘                   │  - GUI (C++)        │
         │ PWM                        │  - TUI (Bash)       │
         ▼                            └─────────────────────┘
┌─────────────────┐
│   ProbeHolder   │
│  (izolacja +    │
│  dopasowanie)   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   Antena EMF    │
└─────────────────┘
```

---

## 🔧 Sprzęt

### Wymagane komponenty:
- **Arduino Nano** (ATmega328P)
- **Moduł Ethernet** ENC28J60
- **ProbeHolder** - moduł izolacji i dopasowania impedancji
- **Antena EMF** - cewka ferrytowa lub płaska

### Połączenia:
| Arduino | Pin | Cel |
|---------|-----|-----|
| D10 | CS | ENC28J60 |
| D11 | MOSI | ENC28J60 |
| D12 | MISO | ENC28J60 |
| D13 | SCK | ENC28J60 |
| D9 | PWM | Wyjście na ProbeHolder |

📋 **Pełna dokumentacja:** [hardware.md](hardware.md)

---

## 🔧 Firmware Arduino

### Struktura projektu:
```
ResoNet_Nano/
├── ResoNet_Nano.ino
├── types.h
├── safety_system.*
├── pwm_engine.*
└── network_system.*
```

### Kompilacja:
```bash
# Arduino IDE
# 1. Otwórz ResoNet_Nano/ResoNet_Nano.ino
# 2. Zainstaluj bibliotekę EthernetENC
# 3. Wybierz Arduino Nano
# 4. Wgraj

# PlatformIO
cd ResoNet_Nano
platformio run --target upload
```

📋 **Szczegóły:** [arduino.md](arduino.md)

---

## 🖥️ Aplikacje

### 🎮 GUI Desktop (C++/ImGui)

Aplikacja desktopowa napisana w C++ z biblioteką Dear ImGui.

**Wymagania:**
```bash
sudo apt install cmake libsdl2-dev libgl1-mesa-dev
```

**Kompilacja:**
```bash
cd gui
mkdir build && cd build
cmake ..
make
./resonet_gui
```

📋 **Więcej:** [gui/README.md](gui/README.md)

---

### 🌍 WebUI (PHP/Apache)

Panel sterowania przez przeglądarkę internetową.

**Instalacja:**
```bash
sudo apt install apache2 php php-sockets
sudo cp -r webui/* /var/www/html/bioresonance/
sudo systemctl restart apache2
```

**Dostęp:** `http://localhost/bioresonance/`

📋 **Więcej:** [webui/README.md](webui/README.md)

---

### 📱 Android (Kotlin)

Aplikacja mobilna na urządzenia z Androidem.

**Wymagania:**
- Android Studio Arctic Fox+
- JDK 17
- Android SDK 30+

**Budowanie:**
```bash
cd android_app
./gradlew assembleDebug
# APK: android_app/app/build/outputs/apk/debug/app-debug.apk
```

📋 **Więcej:** [android_app/README.md](android_app/README.md)

---

### 💻 TUI (Bash)

Terminalowy interfejs w Bashu.

**Uruchomienie:**
```bash
cd bash_tui
chmod +x bioresonance_tui.sh
./bioresonance_tui.sh
```

📋 **Więcej:** [bash_tui/README.md](bash_tui/README.md)

---

## ⚕️ Bezpieczeństwo

⚠️ **WAŻNE**: To urządzenie eksperymentalne. Nie używaj do celów medycznych bez odpowiednich certyfikatów.

### Zasady bezpieczeństwa:
- Używaj tylko zasilaczy izolowanych
- Sprawdź izolację przed każdym użyciem
- Nie przekraczaj zalecanych czasów sesji
- Konsultuj się ze specjalistami

---

## 📜 Licencja

- **Oprogramowanie**: AGPLv3
- **Dokumentacja**: CC BY-NC-SA 4.0

---

<div align="center">

**Autor:** bartoszruta26-droid  
**Repozytorium:** https://github.com/bartoszruta26-droid/Bioresonance

</div>
