# 📊 Biofeedback Sensor - GSR, HRV z Adaptacją

<div align="center">

![Typ](https://img.shields.io/badge/typ-sensor%20biofeedback-blue)
![Pomiary](https://img.shields.io/badge/pomiary-GSR%2C%20HRV%2C%20Temp-green)
![Adaptacja](https://img.shields.io/badge/adaptacja-automaticzna-orange)
![Bezpieczeństwo](https://img.shields.io/badge/bezpieczeństwo-IEC%2060601--1-red)

**Wieloparametrowy sensor biofeedbacku z automatyczną adaptacją terapii**

</div>

---

## 📋 Spis Treści

1. [Opis i Zastosowanie](#opis-i-zastosowanie)
2. [Mierzone Parametry](#mierzone-parametry)
3. [Specyfikacja Techniczna](#specyfikacja-techniczna)
4. [Podłączenie do Arduino](#podłączenie-do-arduino)
5. [Wykrywanie Podłączenia](#wykrywanie-podłączenia)
6. [Algorytmy Adaptacji](#algorytmy-adaptacji)
7. [Protokoły Biofeedback](#protokoły-biofeedback)
8. [Bezpieczeństwo](#bezpieczeństwo)

---

## 🧠 Opis i Zastosowanie

Sensor biofeedbacku to zaawansowany moduł pomiarowy umożliwiający **monitorowanie parametrów fizjologicznych pacjenta w czasie rzeczywistym** i automatyczne dostosowywanie terapii na podstawie zebranych danych. System wykorzystuje pętlę sprzężenia zwrotnego dla optymalizacji efektów terapeutycznych.

### Zastosowania Kliniczne
- **Redukcja stresu**: Trening relaksacyjny z GSR feedbackiem
- **Kardiologia**: Analiza HRV dla zdrowia serca
- **Neurologia**: Wsparcie leczenia migren, lęków
- **Rehabilitacja**: Monitorowanie postępu terapii
- **Psychoterapia**: Biologiczny feedback emocji

### Zalety Systemu Biofeedback
| Cecha | Korzyść |
|-------|---------|
| **Real-time monitoring** | Natychmiastowa reakcja na zmiany |
| **Adaptacyjność** | Terapia dopasowana do aktualnego stanu |
| **Obiektywizacja** | Mierzalne postępy w leczeniu |
| **Motywacja** | Wizualizacja postępów dla pacjenta |

---

## 📈 Mierzone Parametry

### 1. GSR (Galvanic Skin Response)
**Opis**: Pomiar przewodnictwa skóry jako wskaźnik pobudzenia układu współczulnego.

| Parametr | Zakres | Rozdzielczość | Dokładność |
|----------|--------|---------------|------------|
| **Przewodnictwo** | 0.1 - 100 µS | 0.01 µS | ±5% |
| **Częstotliwość próbkowania** | 10-100 Hz | - | - |
| **Napięcie pomiarowe** | 0.5V DC | - | Stabilne |

**Interpretacja**:
- **Niskie GSR** (< 5 µS): Relaks, niski stres
- **Średnie GSR** (5-20 µS): Stan normalny
- **Wysokie GSR** (> 20 µS): Stres, pobudzenie emocjonalne

### 2. HRV (Heart Rate Variability)
**Opis**: Analiza zmienności akcji serca poprzez pomiar PPG (fotopletyzmografia).

| Parametr | Zakres | Rozdzielczość | Dokładność |
|----------|--------|---------------|------------|
| **Tętno** | 30-200 BPM | 1 BPM | ±2 BPM |
| **RR Interval** | 300-2000 ms | 1 ms | ±5 ms |
| **SDNN** | 0-100 ms | 1 ms | - |
| **RMSSD** | 0-100 ms | 1 ms | - |
| **LF/HF Ratio** | 0.1-10 | 0.01 | - |

**Wskaźniki HRV**:
- **SDNN**: Standard deviation of NN intervals - ogólna zmienność
- **RMSSD**: Root mean square of successive differences - aktywność przywspółczulna
- **LF**: Low frequency (0.04-0.15 Hz) - mieszana aktywność
- **HF**: High frequency (0.15-0.4 Hz) - aktywność przywspółczulna

### 3. Temperatura Skóry
**Opis**: Pomiar temperatury obwodowej jako wskaźnik perfuzji i stresu.

| Parametr | Zakres | Rozdzielczość | Dokładność |
|----------|--------|---------------|------------|
| **Temperatura** | 20-40°C | 0.1°C | ±0.3°C |
| **Czas reakcji** | < 2 s | - | - |

**Interpretacja**:
- **Niska temp.** (< 30°C): Stres, słaba cyrkulacja
- **Normalna temp.** (32-34°C): Stan prawidłowy
- **Wysoka temp.** (> 35°C): Gorączka, wysiłek

---

## ⚙️ Specyfikacja Techniczna

### Parametry Elektryczne
| Parametr | Wartość | Uwagi |
|----------|---------|-------|
| **Zasilanie** | 3.3V / 5V | Niski pobór mocy |
| **Pobór prądu** | < 10 mA | Tryb ciągły |
| **Izolacja** | 2500V RMS | Bezpieczeństwo pacjenta |
| **Częstotliwość próbkowania** | 100-1000 Hz | Multiplexing kanałów |

### Sensory
| Sensor | Typ | Producent (przykład) |
|--------|-----|----------------------|
| **GSR** | Elektrody Ag/AgCl + wzmacniacz transimpedancyjny | Custom |
| **PPG/HRV** | MAX30102 / AFE4400 | Maxim Integrated / TI |
| **Temperatura** | NTC 10kΩ / MLX90614 IR | Vishay / Melexis |

### Interfejsy Komunikacyjne
| Interfejs | Typ | Szybkość |
|-----------|-----|----------|
| **I2C** | GSR + Temp | 400 kHz |
| **SPI** | PPG/HRV | 2 MHz |
| **ADC** | Wszystkie analogowe | 10-bit / 12-bit |

---

## 🔌 Podłączenie do Arduino

### Schemat Połączeń

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│   Arduino Nano  │         │   Interface      │         │   Elektrody     │
│                 │         │   Board          │         │   / Sensory     │
│   A6 (I2C SDA)  │◀───────▶│ SDA              │────────▶│   GSR (2 el.)   │
│   A7 (I2C SCL)  │◀───────▶│ SCL              │         │                 │
│   D11 (SPI MOSI)│◀───────▶│ MOSI             │────────▶│   PPG (palec)   │
│   D12 (SPI MISO)│◀───────▶│ MISO             │         │                 │
│   D13 (SPI SCK) │◀───────▶│ SCK              │         │   NTC (skóra)   │
│   3.3V          │────────▶│ VCC              │         │                 │
│   GND           │────────▶│ GND              │────────▶│   COMMON        │
│   D5            │◀───────▶│ INT (PPG)        │         │                 │
│   D6            │◀───────▶│ DRDY (GSR)       │         │                 │
└─────────────────┘         └──────────────────┘         └─────────────────┘
```

### Pinout Arduino

| Pin Arduino | Funkcja | Sensor | Opis |
|-------------|---------|--------|------|
| **A4 (SDA)** | I2C Data | GSR, Temp | Dwukierunkowa linia danych |
| **A5 (SCL)** | I2C Clock | GSR, Temp | Sygnał zegarowy |
| **D11 (MOSI)** | SPI Out | PPG | Dane do PPG |
| **D12 (MISO)** | SPI In | PPG | Dane z PPG |
| **D13 (SCK)** | SPI Clock | PPG | Zegar SPI |
| **D5** | INT | PPG | Interrupt nowy sample |
| **D6** | DRDY | GSR | Data ready |
| **A0** | ADC | NTC | Pomiar temperatury |
| **D7** | DETECT | All | Wykrywanie podłączenia |

### Komponenty Interface Board

| Element | Typ | Opis |
|---------|-----|------|
| **U1** | MAX30102 | PPG + SpO2 sensor |
| **U2** | Custom ASIC | Wzmacniacz GSR |
| **R_NTC** | 10kΩ @ 25°C | Termistor NTC |
| **ISO** | ISO1540 | Izolator I2C/SPI |
| **Electrodes** | Ag/AgCl | Elektrody jednorazowe |

---

## 🔍 Wykrywanie Podłączenia

System wykrywa podłączenie każdego sensora niezależnie i weryfikuje jakość sygnału.

### Metoda Detekcji

```cpp
// Piny detekcji
#define PIN_BIO_DETECT      7    // Pin cyfrowy główny
#define PIN_PPG_INT         5    // Interrupt PPG
#define PIN_GSR_DRDY        6    // Data ready GSR

// Adresy I2C
#define GSR_I2C_ADDRESS     0x48
#define TEMP_I2C_ADDRESS    0x4A
#define PPG_SPI_CS          10

// Progi jakości sygnału
#define GSR_SIGNAL_MIN      100   // Min wartość ADC
#define GSR_SIGNAL_MAX      900   // Max wartość ADC
#define PPG_SIGNAL_MIN      50000 // Min amplitude PPG
#define HRV_CONFIDENCE_MIN  0.7   // Min confidence score
```

### Algorytm Wykrywania

1. **Scan I2C**: Sprawdzenie obecności sensorów GSR i Temp
2. **Test SPI**: Komunikacja z sensorem PPG
3. **Quality Check**: Ocena jakości sygnałów (3 sekundy)
4. **Decyzja**:
   - Wszystkie sensory OK → Ready
   - Częściowa awaria → Degraded mode
   - Brak sensorów → Error

### Implementacja w Arduino

```cpp
typedef enum {
    BIO_READY = 0,
    BIO_PARTIAL,
    BIO_GSR_MISSING,
    BIO_PPG_MISSING,
    BIO_TEMP_MISSING,
    BIO_ERROR
} BioStatus_t;

typedef struct {
    bool gsrConnected;
    bool ppgConnected;
    bool tempConnected;
    float gsrQuality;      // 0.0 - 1.0
    float ppgQuality;      // 0.0 - 1.0
    float confidence;      // Overall confidence
} BioSensorStatus_t;

BioStatus_t detectBiofeedbackSensors(BioSensorStatus_t* status) {
    Wire.begin();
    
    // Test GSR (I2C)
    Wire.beginTransmission(GSR_I2C_ADDRESS);
    status->gsrConnected = (Wire.endTransmission() == 0);
    
    // Test PPG (SPI)
    digitalWrite(PPG_SPI_CS, LOW);
    delayMicroseconds(10);
    uint8_t ppgId = SPI.transfer(0x00);  // Read ID register
    digitalWrite(PPG_SPI_CS, HIGH);
    status->ppgConnected = (ppgId == 0x15);  // MAX30102 ID
    
    // Test Temp (I2C)
    Wire.beginTransmission(TEMP_I2C_ADDRESS);
    status->tempConnected = (Wire.endTransmission() == 0);
    
    // Quality check (3 seconds)
    if (status->gsrConnected) {
        status->gsrQuality = evaluateGSRQuality();
    }
    if (status->ppgConnected) {
        status->ppgQuality = evaluatePPGQuality();
    }
    
    // Oblicz całkowite zaufanie
    int sensorsOk = (status->gsrConnected ? 1 : 0) +
                    (status->ppgConnected ? 1 : 0) +
                    (status->tempConnected ? 1 : 0);
    
    status->confidence = (sensorsOk / 3.0) * 
                         ((status->gsrQuality + status->ppgQuality) / 2.0);
    
    // Decyzja
    if (sensorsOk == 3 && status->confidence > 0.7) {
        LOG_INFO("Biofeedback: wszystkie sensory gotowe");
        return BIO_READY;
    } else if (sensorsOk >= 2) {
        LOG_WARNING("Biofeedback: tryb częściowy (%d/3)", sensorsOk);
        return BIO_PARTIAL;
    } else if (!status->gsrConnected) {
        LOG_ERROR("Biofeedback: brak sensora GSR");
        return BIO_GSR_MISSING;
    } else if (!status->ppgConnected) {
        LOG_ERROR("Biofeedback: brak sensora PPG");
        return BIO_PPG_MISSING;
    } else {
        LOG_ERROR("Biofeedback: błąd krytyczny");
        return BIO_ERROR;
    }
}

float evaluateGSRQuality() {
    // Pobierz 100 próbek GSR
    uint32_t sum = 0;
    uint32_t variance = 0;
    
    for (int i = 0; i < 100; i++) {
        int value = readGSR();
        sum += value;
        delay(10);
    }
    
    float mean = sum / 100.0;
    
    // Sprawdź czy sygnał nie jest płaski (odłączony)
    // ani nie szumi nadmiernie (zły kontakt)
    if (mean < 100 || mean > 950) return 0.0;
    
    // Uproszczona ocena wariancji
    return constrain((mean - 100) / 700.0, 0.0, 1.0);
}

float evaluatePPGQuality() {
    // Sprawdź czy wykryto puls
    uint32_t startTime = millis();
    int pulseCount = 0;
    
    while (millis() - startTime < 3000) {
        if (digitalRead(PPG_INT) == LOW) {
            pulseCount++;
        }
        delay(1);
    }
    
    // Oczekiwany puls: ~3-200 na 3 sekundy (60-4000 BPM - szeroki zakres)
    if (pulseCount < 3 || pulseCount > 200) return 0.0;
    
    return constrain(pulseCount / 100.0, 0.0, 1.0);
}
```

### Schemat Detekcji

```
                    +3.3V
                     │
         ┌───────────┴───────────┐
         │                       │
    ┌────┴────┐             ┌────┴────┐
    │  I2C    │             │   SPI   │
    │  Scan   │             │   Test  │
    │         │             │         │
    └────┬────┘             └────┬────┘
         │                       │
    ┌────┴───────────────────────┴────┐
    │                                 │
    │      Interface Board            │
    │  ┌─────────┐  ┌─────────┐       │
    │  │   GSR   │  │   PPG   │       │
    │  │  0x48   │  │  (SPI)  │       │
    │  └────┬────┘  └────┬────┘       │
    │       │            │            │
    │  ┌────┴────┐  ┌────┴────┐       │
    │  │  TEMP   │  │  EKG    │       │
    │  │  0x4A   │  │ (opt.)  │       │
    │  └─────────┘  └─────────┘       │
    │                                 │
    └───────────────┬─────────────────┘
                    │
         ┌──────────┴──────────┐
         │                     │
    ┌────┴────┐           ┌────┴────┐
    │Elektrody│           │  PPG    │
    │   GSR   │           │ Sensor  │
    └─────────┘           └─────────┘
```

---

## 🔄 Algorytmy Adaptacji

### Pętla Biofeedback

```
┌─────────────────────────────────────────────────────────────┐
│                    BIOFEEDBACK LOOP                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐              │
│  │ POMIAR   │───▶│ ANALIZA  │───▶│ DECYZJA  │              │
│  │ GSR,HRV  │    │ STANU    │    │ THERAPY  │              │
│  └──────────┘    └──────────┘    └──────────┘              │
│       ▲                              │                      │
│       │                              │                      │
│       │         ┌──────────┐         │                      │
│       └─────────│ ADAPTACJA│◀────────┘                      │
│                 │ PARAMETRÓW│                               │
│                 └──────────┘                                │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Reguły Adaptacji

#### Na podstawie GSR:
```cpp
void adaptBasedOnGSR(float gsrValue) {
    // GSR w µS
    if (gsrValue > 30.0) {
        // Wysoki stres - zwiększ relaksację
        therapyParams.frequency = 10.0;  // Hz (alfa)
        therapyParams.intensity = 0.6;
        therapyParams.duration += 300000;  // +5 minut
        
        LOG_INFO("Wysoki stres - adaptacja: relaksacja");
    } else if (gsrValue < 5.0) {
        // Bardzo zrelaksowany - utrzymuj stan
        therapyParams.frequency = 7.83;  // Schumann
        therapyParams.intensity = 0.4;
        
        LOG_INFO("Głęboki relaks - utrzymanie");
    }
}
```

#### Na podstawie HRV:
```cpp
void adaptBasedOnHRV(float hrvScore, float lfHfRatio) {
    // HRV Score: 0-100 (wyższy = lepszy)
    // LF/HF Ratio: <1.0 (przewaga przywspółczulna), >2.0 (współczulna)
    
    if (hrvScore < 30 && lfHfRatio > 2.0) {
        // Niska HRV, dominacja sympatykowa - stres
        therapyParams.frequency = 6.0;  // Theta
        therapyParams.modulation = MODULATION_AM;
        therapyParams.modFrequency = 0.5;  // Wolna modulacja
        
        LOG_INFO("Stres (niska HRV) - theta stimulation");
    } else if (hrvScore > 70 && lfHfRatio < 1.0) {
        // Dobra HRV, dominacja parasympatykowa - relaks
        therapyParams.frequency = 10.0;  // Alfa
        therapyParams.modulation = MODULATION_NONE;
        
        LOG_INFO("Dobra HRV - maintenance alfa");
    }
}
```

#### Fuzzy Logic Decision:
```cpp
typedef struct {
    float stressLevel;      // 0.0 - 1.0
    float relaxationLevel;  // 0.0 - 1.0
    float coherence;        // 0.0 - 1.0
} BioState_t;

BioState_t calculateBioState(float gsr, float hrv, float temp) {
    BioState_t state;
    
    // Normalizacja wejść
    float gsrNorm = constrain((gsr - 2.0) / 30.0, 0.0, 1.0);
    float hrvNorm = constrain(hrv / 100.0, 0.0, 1.0);
    float tempNorm = constrain((temp - 28.0) / 10.0, 0.0, 1.0);
    
    // Obliczenie stanów (uproszczony fuzzy logic)
    state.stressLevel = (gsrNorm * 0.5) + ((1.0 - hrvNorm) * 0.3) + 
                        ((1.0 - tempNorm) * 0.2);
    
    state.relaxationLevel = 1.0 - state.stressLevel;
    
    state.coherence = hrvNorm * tempNorm;  // Spójność autonomiczna
    
    return state;
}
```

---

## 💊 Protokoły Biofeedback

### Stress Reduction Protocol
| Parametr | Wartość | Adaptacja |
|----------|---------|-----------|
| **Cel** | Redukcja GSR o 30% | Ciągła |
| **Częstotliwość bazowa** | 10 Hz (Alfa) | ±2 Hz |
| **Modulacja** | AM 0.5 Hz | Stała |
| **Czas sesji** | 20-40 min | Auto-extend |
| **Kryterium sukcesu** | GSR < 10 µS przez 5 min | - |

### HRV Coherence Training
| Parametr | Wartość | Adaptacja |
|----------|---------|-----------|
| **Cel** | HRV Score > 70 | Stopniowa |
| **Częstotliwość** | 6 Hz (Resonance) | ±0.5 Hz |
| **Oddychanie guide** | 5.5-6.5 breaths/min | Audio/visual |
| **Czas sesji** | 15-20 min | - |
| **Kryterium sukcesu** | Coherence > 0.7 przez 3 min | - |

### Sleep Preparation
| Parametr | Wartość | Adaptacja |
|----------|---------|-----------|
| **Cel** | Temp ↑, GSR ↓ | Gradualna |
| **Częstotliwość** | 4-7 Hz (Theta) | Sweep down |
| **Intensywność** | Niska (0.3) | Fade out |
| **Czas sesji** | 30-60 min | Auto-off |
| **Kryterium sukcesu** | Sen wykryty (brak ruchu) | Wyłącz |

### Pain Management
| Parametr | Wartość | Adaptacja |
|----------|---------|-----------|
| **Cel** | Subiektywna redukcja bólu | Feedback-driven |
| **Częstotliwość** | 727 Hz + 10 Hz | Dual |
| **Modulacja** | Burst | Pacjent-controlled |
| **Czas sesji** | Do ulgi | Max 60 min |
| **Kryterium sukcesu** | Pacjent potwierdza | - |

---

## ⚠️ Bezpieczeństwo

### Przeciwwskazania
- **Brak świadomej zgody**: Pacjent musi rozumieć proces
- **Ciężkie zaburzenia psychiczne**: Konsultacja psychiatryczna
- **Nieuregulowana epilepsja**: Ryzyko wywołania ataku
- **Ostre stany kardialne**: Stabilizacja przed biofeedbackiem

### Środki Ostrożności
1. **Kalibracja początkowa**: 5 minut baseline przed terapią
2. **Monitorowanie jakości**: Ciągła walidacja sygnałów
3. **Fallback mode**: Powrót do parametrów bezpiecznych przy utracie sygnału
4. **Dokumentacja**: Logowanie wszystkich zmian parametrów

### Alarmy Systemowe
| Alarm | Warunek | Akcja |
|-------|---------|-------|
| **SENSOR_DISCONNECT** | Utrata sensora | Pause therapy |
| **LOW_SIGNAL_QUALITY** | Quality < 0.3 | Warn + recalibrate |
| **ADAPTATION_LIMIT** | Parametry poza safe range | Clamp to limits |
| **PATIENT_DISTRESS** | GSR spike + HRV drop | Emergency stop |

### Limety Bezpieczeństwa
```cpp
#define SAFE_FREQ_MIN       1.0     // Hz
#define SAFE_FREQ_MAX       100.0   // Hz
#define SAFE_INTENSITY_MAX  0.8     // 80% max power
#define SAFE_SESSION_MAX    3600000 // 60 minut
```

---

## 📎 Powiązane Dokumenty

- [Arduino Documentation](../arduino.md) - Konfiguracja PWM
- [Hardware Documentation](../hardware.md) - Schematy połączeń
- [Contact Electrodes](effector_contact_electrodes.md) - Elektrody
- [Safety Guidelines](safety_guidelines.md) - Procedury bezpieczeństwa

---

**Wersja dokumentacji**: 1.0  
**Data**: 2024  
**Autor**: ResoNet Development Team
