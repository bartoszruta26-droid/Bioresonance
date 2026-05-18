# 📚 Dokumentacja Efektorów i Sensorów - ResoNet Nano

<div align="center">

![Version](https://img.shields.io/badge/version-1.0-blue)
![Status](https://img.shields.io/badge/status-complete-green)
![Devices](https://img.shields.io/badge/devices-6%20effectors%2B%201%20sensor-orange)

**Kompletny przewodnik po efektorach i sensorach systemu ResoNet Nano**

</div>

---

## 📋 Spis Treści

1. [Wprowadzenie](#wprowadzenie)
2. [Efektory](#efektory)
3. [Sensory](#sensory)
4. [Wykrywanie Podłączenia](#wykrywanie-podłączenia)
5. [Tabela Porównawcza](#tabela-porównawcza)
6. [Integracja z Arduino](#integracja-z-arduino)

---

## 🎯 Wprowadzenie

System **ResoNet Nano** obsługuje szeroką gamę efektorów (urządzeń wyjściowych) oraz sensorów (urządzeń wejściowych), umożliwiając elastyczne dostosowanie terapii do indywidualnych potrzeb pacjenta. Każdy efektor i sensor jest automatycznie wykrywany przez system, co zapewnia bezpieczeństwo i wygodę użytkowania.

### Architektura Systemu

```
┌─────────────────────────────────────────────────────────────────┐
│                      RESONET NANO SYSTEM                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │   EFECTORY   │    │   ARDUINO    │    │    SENSORY   │      │
│  │   (OUTPUT)   │◀───│     NANO     │───▶│   (INPUT)    │      │
│  │              │    │              │    │              │      │
│  │ • Helmholtz  │    │  Detection   │    │ • Biofeedback│      │
│  │ • Otic       │◀───│  Algorithm   │───▶│ • GSR/HRV    │      │
│  │ • Contact    │    │              │    │ • Temp       │      │
│  │ • Wrap       │    │  Safety      │    │              │      │
│  │ • LED+EMF    │    │  Monitoring  │    │              │      │
│  │ • Thermo     │    │              │    │              │      │
│  │ • Vibro      │    │              │    │              │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔌 Efektory

Efektory to urządzenia wyjściowe aplikujące sygnały terapeutyczne na ciało pacjenta.

### Dostępne Efektory

| Nazwa | Typ | Częstotliwość | Zastosowanie | Dokumentacja |
|-------|-----|---------------|--------------|--------------|
| **🧲 Cewka Helmholtza** | Pole jednorodne | 0.1Hz - 500kHz | Badania naukowe, kalibracja | [Dokumentacja](docs/effectors/effector_helmholtz_coil.md) |
| **🦻 Aplikator Uszny** | Wysokie częstotliwości | 1kHz - 500kHz | Szumy uszne, terapia głowy | [Dokumentacja](docs/effectors/effector_otic_applicator.md) |
| **🔌 Elektrody Kontaktowe** | Bezpośredni kontakt | DC - 100kHz | TENS, EMS, ionoforeza | [Dokumentacja](docs/effectors/effector_contact_electrodes.md) |
| **📡 Aplikator Okrężny** | Elastyczny wrap | 1Hz - 200kHz | Kończyny, tułów, stawy | [Dokumentacja](docs/effectors/effector_wrap_applicator.md) |
| **💡 Matrix LED + EMF** | Fotoniczno-EMF | 400-700nm + EMF | Terapia światłem + pole | [W przygotowaniu] |
| **🌡️ Termo-EMF Probe** | Grzanie + EMF | EMF + thermal | Terapia cieplna głęboka | [W przygotowaniu] |
| **🔊 Wibracyjna Końcówka** | Mechanika + EMF | Audio + piezo + EMF | Masaż wibracyjny + terapia | [W przygotowaniu] |

### Szczegółowa Dokumentacja Efektorów

#### 1. Cewka Helmholtza
- **Zastosowanie**: Generowanie jednorodnego pola magnetycznego
- **Charakterystyka**: Precyzyjne badania naukowe, kalibracja sensorów
- **Detekcja**: Pomiar impedancji cewki
- **[Pełna dokumentacja →](docs/effectors/effector_helmholtz_coil.md)**

#### 2. Aplikator Uszny (Otic)
- **Zastosowanie**: Terapia schorzeń uszu i głowy
- **Charakterystyka**: Wysokie częstotliwości, ergonomiczny kształt
- **Detekcja**: Pomiar impedancji charakterystycznej ucha
- **[Pełna dokumentacja →](docs/effectors/effector_otic_applicator.md)**

#### 3. Elektrody Kontaktowe
- **Zastosowanie**: Uniwersalna elektroterapia
- **Charakterystyka**: Bezpośredni kontakt ze skórą, różne typy
- **Detekcja**: Pomiar impedancji skóry + jakości kontaktu
- **[Pełna dokumentacja →](docs/effectors/effector_contact_electrodes.md)**

#### 4. Aplikator Okrężny
- **Zastosowanie**: Owijanie wokół kończyn i tułowia
- **Charakterystyka**: Elastyczny, równomierna dystrybucja pola
- **Detekcja**: Ciągłość cewki + temperatura
- **[Pełna dokumentacja →](docs/effectors/effector_wrap_applicator.md)**

#### 5. Pasek LED IR
- **Zastosowanie**: Terapia fotoniczna tkanek miękkich, przyspieszanie gojenia
- **Charakterystyka**: Dioda IR 850nm/940nm z carrier 38kHz, modulacja terapeutyczna 1-100Hz
- **Detekcja**: Pomiar prądu + pin detekcji cyfrowej
- **Piny Arduino**: D5 (IR_PWM), D6 (IR_MOD), D3 (IR_DETECT), A6 (IR_CURRENT)
- **Specyfikacja**:
  - Częstotliwość nośna: 38kHz (standard IR)
  - Zakres modulacji: 1-100 Hz (AM/FM/Burst)
  - Moc: do 500mA @ 5V
  - Detekcja zwarć i przerwy w obwodzie
- **Bezpieczeństwo**: Izolacja galwaniczna 2500V RMS, monitoring prądu

---

## 📊 Sensory

Sensory to urządzenia wejściowe monitorujące parametry fizjologiczne pacjenta.

### Dostępne Sensory

| Nazwa | Mierzone Parametry | Interfejs | Zastosowanie | Dokumentacja |
|-------|-------------------|-----------|--------------|--------------|
| **📈 Biofeedback Sensor** | GSR, HRV, Temperatura | I2C + SPI | Adaptacja terapii | [Dokumentacja](docs/sensors/sensor_biofeedback.md) |

### Szczegółowa Dokumentacja Sensorów

#### 1. Biofeedback Sensor
- **Parametry**: 
  - GSR (Galvanic Skin Response) - przewodnictwo skóry
  - HRV (Heart Rate Variability) - zmienność tętna
  - Temperatura skóry
- **Interfejsy**: I2C (GSR, Temp) + SPI (PPG/HRV)
- **Adaptacja**: Automatyczne dostosowanie parametrów terapii
- **[Pełna dokumentacja →](docs/sensors/sensor_biofeedback.md)**

---

## 🔍 Wykrywanie Podłączenia

System ResoNet Nano implementuje zaawansowane algorytmy automatycznego wykrywania podłączonych urządzeń.

### Metody Detekcji

| Typ | Metoda | Opis |
|-----|--------|------|
| **Impedancja** | Pomiar AC/DC | Dla cewek, elektrod |
| **I2C Scan** | Adresy I2C | Dla sensorów cyfrowych |
| **SPI Test** | Read ID register | Dla sensorów SPI |
| **Temperatura** | NTC thermistor | Monitorowanie przegrzania |
| **Continuity** | Pomiar rezystancji | Sprawdzenie ciągłości |

### Algorytm Ogólny

```
1. Standby → Pull-up HIGH na pinach detekcji
2. Test → Krótki impuls testowy (AC lub DC)
3. Pomiar → Odczyt ADC / komunikacja cyfrowa
4. Walidacja → Sprawdzenie czy wartości w zakresie
5. Decyzja → Device connected / disconnected / error
6. Monitorowanie → Ciągła walidacja podczas pracy
```

### Przykładowe Progi Detekcji

| Urządzenie | Pin | Min ADC | Max ADC | Open Circuit |
|------------|-----|---------|---------|--------------|
| Helmholtz Coil | A1 | - | 800 | >900 |
| Otic Applicator | A2 | 100 | 600 | >900 |
| Contact Electrodes | A3 | 200 | 800 | >950 |
| Wrap Applicator | A5 | 50 | 400 | >900 |
| Biofeedback (I2C) | A4/A5 | - | - | No ACK |

---

## 📊 Tabela Porównawcza

### Porównanie Efektorów

| Cecha | Helmholtz | Otic | Contact | Wrap |
|-------|-----------|------|---------|------|
| **Zakres freq.** | 0.1Hz-500kHz | 1kHz-500kHz | DC-100kHz | 1Hz-200kHz |
| **Impedancja** | 1-10 Ω | 8-32 Ω | 500Ω-50kΩ | 0.5-3 Ω |
| **Detekcja** | ✅ | ✅ | ✅ | ✅ |
| **Temp. monitor** | ❌ | ❌ | ❌ | ✅ |
| **Bezpieczeństwo** | IP20 | IPX4 | IPX4 | IPX5 |
| **Cena** | $$$ | $$ | $ | $$ |

### Porównanie Sensorów

| Cecha | Biofeedback |
|-------|-------------|
| **Parametry** | GSR, HRV, Temp |
| **Dokładność GSR** | ±5% |
| **Dokładność HRV** | ±5 ms |
| **Dokładność Temp** | ±0.3°C |
| **Częstotliwość** | 100-1000 Hz |
| **Izolacja** | 2500V RMS |

---

## 💻 Integracja z Arduino

### Konfiguracja Pinów

```cpp
// types.h - Definicje pinów dla efektorów i sensorów

// --- Efektory ---
#define PIN_PWM_OUTPUT          9    // Główny PWM (wszystkie efektory)
#define PIN_HELMHOLTZ_DETECT    2    // Detekcja cewki Helmholtza
#define PIN_HELMHOLTZ_SENSE     A1   // Pomiar prądu Helmholtz
#define PIN_OTIC_ENABLE         4    // Enable aplikatora usznego
#define PIN_OTIC_IMPEDANCE      A2   // Pomiar impedancji otic
#define PIN_ELECTRODE_ENABLE    6    // Enable elektrod
#define PIN_ELECTRODE_IMPEDANCE A3   // Pomiar impedancji elektrod
#define PIN_WRAP_ENABLE         8    // Enable aplikatora okrężnego
#define PIN_WRAP_TEMP           A4   // Temperatura wrap
#define PIN_WRAP_CURRENT        A5   // Prąd wrap

// --- Sensory ---
#define PIN_BIO_DETECT          7    // Detekcja biofeedback
#define PIN_PPG_INT             5    // Interrupt PPG
#define PIN_GSR_DRDY            6    // Data ready GSR
// I2C: A4 (SDA), A5 (SCL)
// SPI: D11 (MOSI), D12 (MISO), D13 (SCK), D10 (CS)
```

### Funkcje Detekcji

```cpp
// Przykładowe wywołanie w głównej pętli

void setup() {
    // Inicjalizacja
    initPins();
    initPWM();
    initI2C();
    initSPI();
}

void loop() {
    // Cykliczne sprawdzanie podłączenia
    checkAllDevices();
    
    // Obsługa terapii
    if (deviceConnected && safetyOk) {
        runTherapy();
    }
    
    // Biofeedback adaptation
    if (biofeedbackEnabled) {
        adaptTherapyBasedOnSensors();
    }
}

void checkAllDevices() {
    // Sprawdź każdy efektor
    helmholtzConnected = detectHelmholtzCoil();
    oticConnected = detectOticApplicator();
    electrodesConnected = detectElectrodes();
    wrapConnected = detectWrapApplicator();
    
    // Sprawdź sensory
    bioStatus = detectBiofeedbackSensors(&bioSensorStatus);
}
```

---

## 🔗 Powiązane Dokumenty

- [Arduino Documentation](arduino.md) - Główna dokumentacja Arduino
- [Hardware Documentation](hardware.md) - Schematy sprzętowe
- [README](README.md) - Omówienie projektu

---

## 📝 Changelog

### v1.0 (2024)
- ✅ Dodano dokumentację Cewki Helmholtza
- ✅ Dodano dokumentację Aplikatora Usznego
- ✅ Dodano dokumentację Elektrod Kontaktowych
- ✅ Dodano dokumentację Aplikatora Okrężnego
- ✅ Dodano dokumentację Sensora Biofeedback
- ✅ Utworzono strukturę katalogów `docs/effectors` i `docs/sensors`

---

**Wersja dokumentacji**: 1.0  
**Data**: 2024  
**Autor**: ResoNet Development Team
