# 📡 Aplikator Okrężny - Elastyczny do Owijania

<div align="center">

![Typ](https://img.shields.io/badge/typ-effektor%20elastyczny-blue)
![Zakres](https://img.shields.io/badge/zakres%20cz%C4%99stotliwo%C5%9Bci-1Hz-200kHz-green)
![Zastosowanie](https://img.shields.io/badge/zastosowanie-kończyny%2C%20tułów-orange)
![Elastyczność](https://img.shields.io/badge/elastyczność-360°-red)

**Elastyczny aplikator do owijania wokół kończyn i tułowia dla równomiernej terapii**

</div>

---

## 📋 Spis Treści

1. [Opis i Zastosowanie](#opis-i-zastosowanie)
2. [Konstrukcja i Materiały](#konstrukcja-i-materiały)
3. [Specyfikacja Techniczna](#specyfikacja-techniczna)
4. [Podłączenie do Arduino](#podłączenie-do-arduino)
5. [Wykrywanie Podłączenia](#wykrywanie-podłączenia)
6. [Protokoły Terapeutyczne](#protokoły-terapeutyczne)
7. [Bezpieczeństwo](#bezpieczeństwo)

---

## 🏥 Opis i Zastosowanie

Aplikator okrężny (coil applicator / wrap-around applicator) to **elastyczny efektor** zaprojektowany do owijania wokół różnych części ciała - kończyn, tułowia, szyi. Zapewnia równomierną dystrybucję pola elektromagnetycznego na dużym obszarze.

### Wskazania Terapeutyczne
- **Stawy**: Kolana, łokcie, nadgarstki (artretyzm, zwyrodnienia)
- **Kręgosłup**: Odcinek lędźwiowy, szyjny (ból, dyskopatia)
- **Mięśnie**: Urazy sportowe, napięcia, regeneracja
- **Naczynia krwionośne**: Żylaki, obrzęki limfatyczne
- **Gojenie**: Złamania, rany pooperacyjne

### Zalety Aplikatora Okrężnego
| Cecha | Korzyść |
|-------|---------|
| **Elastyczność** | Dopasowanie do każdej anatomii |
| **Równomierność** | Jednolite pole na całym obszarze |
| **Uniwersalność** | Jeden aplikator dla wielu zastosowań |
| **Komfort** | Miękki, nieograniczający ruchów |

---

## 🔬 Konstrukcja i Materiały

### Warstwy Aplikatora

```
┌─────────────────────────────────────────┐
│  Warstwa Zewnętrzna                     │
│  (Neopren / Lycra - oddychająca)        │
├─────────────────────────────────────────┤
│  Warstwa Ekranująca                     │
│  (Siatka miedziana - minimalizacja EMI) │
├─────────────────────────────────────────┤
│  Cewka Główna                           │
│  (Drut wielożyłowy w silikonie)         │
├─────────────────────────────────────────┤
│  Warstwa Kontaktowa                     │
│  (Miękki żel silikonowy)                │
└─────────────────────────────────────────┘
```

### Materiały
| Komponent | Materiał | Właściwości |
|-----------|----------|-------------|
| **Obudowa** | Neopren + Lycra | Elastyczny, wodoodporny |
| **Przewodnik** | Drut Litz 42×0.1mm | Niska rezystancja AC |
| **Izolacja** | Silicone medical-grade | Biokompatybilny |
| **Ekran** | Copper mesh 0.2mm | Tłumienie >30 dB |
| **Zapięcie** | Velcro® industrial | Wytrzymałe, regulowane |

### Warianty Rozmiarowe
| Rozmiar | Długość | Szerokość | Obwód maksymalny | Zastosowanie |
|---------|---------|-----------|------------------|--------------|
| **S** | 30 cm | 8 cm | 40 cm | Nadgarstek, kostka |
| **M** | 50 cm | 10 cm | 60 cm | Kolano, łokieć, szyja |
| **L** | 80 cm | 12 cm | 100 cm | Udo, ramię |
| **XL** | 120 cm | 15 cm | 140 cm | Tułów, biodra |

---

## ⚙️ Specyfikacja Techniczna

### Parametry Elektryczne
| Parametr | Wartość | Uwagi |
|----------|---------|-------|
| **Indukcyjność** | 20-100 µH | Zależnie od rozmiaru |
| **Rezystancja DC** | 0.5-3 Ω | Niska strata mocy |
| **Impedancja @1kHz** | 1-10 Ω | Reaktancja indukcyjna |
| **Prąd maksymalny** | 1-3 A | Ograniczony termicznie |
| **Pojemność pasożytnicza** | <50 pF | Minimalizacja strat HF |

### Parametry Pola EMF
| Parametr | Wartość | Opis |
|----------|---------|------|
| **Natężenie pola** | 0.05-5 mT | Regulowane |
| **Jednorodność** | ±10% | Na wewnętrznej powierzchni |
| **Głębokość penetracji** | 5-15 cm | Zależnie od częstotliwości |
| **Kierunek pola** | Promieniowy | Prostopadły do skóry |

### Parametry Mechaniczne
| Parametr | Wartość | Opis |
|----------|---------|------|
| **Elastyczność** | 0-100% długości | Pełny zakres regulacji |
| **Grubość** | 8-12 mm | Profil niski |
| **Waga** | 100-400 g | Zależnie od rozmiaru |
| **IP Rating** | IPX5 | Odporność na wilgoć |
| **Temperatura pracy** | 0-45°C | Zakres komfortu |

---

## 🔌 Podłączenie do Arduino

### Schemat Połączeń

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│   Arduino Nano  │         │   Driver Prądu   │         │ Aplikator       │
│                 │         │   Stałego         │         │ Okrężny         │
│   Pin 9 (PWM)   │────────▶│ Control Input    │         │   Coil IN       │
│   GND           │────────▶│ GND              │────────▶│   Coil OUT      │
│   D8            │────────▶│ Enable           │         │                 │
│   A4            │────────▶│ Temp Sense       │         │   NTC Sensor    │
└─────────────────┘         └──────────────────┘         └─────────────────┘
```

### Pinout Arduino

| Pin Arduino | Funkcja | Opis |
|-------------|---------|------|
| **D9 (OC1A)** | PWM_CTRL | Sygnał sterujący (Timer1) |
| **D8** | ENABLE | Włączanie drivera prądu |
| **GND** | Masa | Wspólna masa |
| **A4** | TEMP | Pomiar temperatury aplikatora |
| **D10** | DETECT | Wykrywanie podłączenia |
| **A5** | CURRENT | Pomiar prądu (opcjonalny) |

### Komponenty Drivera Prądu Stałego

| Element | Typ | Wartość | Opis |
|---------|-----|---------|------|
| **U1** | Current Regulator | LM317T / LT3080 | Stały prąd |
| **Q1** | MOSFET | IRF540N | Switching element |
| **R_sense** | Current Sense | 0.33Ω/10W | Pomiar i limit |
| **L1** | Inductor | 100µH/3A | Wygładzanie prądu |
| **NTC** | Thermistor | 10kΩ @ 25°C | Monitorowanie temp. |

---

## 🔍 Wykrywanie Podłączenia

System wykrywa podłączenie aplikatora okrężnego poprzez pomiar ciągłości cewki i impedancji.

### Metoda Detekcji

```cpp
// Piny detekcji
#define PIN_WRAP_DETECT     10   // Pin cyfrowy
#define PIN_WRAP_CURRENT    A5   // Pomiar prądu
#define PIN_WRAP_TEMP       A4   // Pomiar temperatury

// Progi detekcji
#define WRAP_CONNECTED_MIN    50   // Min wartość ADC (niska R)
#define WRAP_CONNECTED_MAX    400  // Max wartość ADC
#define WRAP_OPEN_CIRCUIT     900  // Brak połączenia
#define WRAP_TEMP_MAX         45   // Max temperatura °C
```

### Algorytm Wykrywania

1. **Test ciągłości**: Pomiar rezystancji DC cewki
2. **Test impedancji**: Krótki impuls AC 1kHz
3. **Pomiar temperatury**: Sprawdzenie NTC
4. **Decyzja**:
   - ADC 50-400 + Temp < 45°C → Aplikator gotowy
   - ADC > 900 → Przerwana cewka
   - Temp > 45°C → Przegrzanie

### Implementacja w Arduino

```cpp
typedef enum {
    WRAP_READY = 0,
    WRAP_CONNECTED,
    WRAP_OPEN,
    WRAP_SHORT,
    WRAP_OVERTEMP
} WrapStatus_t;

WrapStatus_t detectWrapApplicator() {
    // Pomiar rezystancji DC
    digitalWrite(PIN_WRAP_ENABLE, LOW);
    delayMicroseconds(100);
    
    int resistanceValue = analogRead(PIN_WRAP_CURRENT);
    
    // Krótki testowy impuls
    digitalWrite(PIN_WRAP_ENABLE, HIGH);
    analogWrite(PIN_PWM_CTRL, 64);
    delayMicroseconds(500);
    
    int impedanceValue = analogRead(PIN_WRAP_CURRENT);
    
    // Zatrzymaj
    analogWrite(PIN_PWM_CTRL, 0);
    digitalWrite(PIN_WRAP_ENABLE, LOW);
    
    // Pomiar temperatury
    int tempRaw = analogRead(PIN_WRAP_TEMP);
    float temperature = 50.0 - (tempRaw * 50.0 / 1024.0);  // NTC 10k
    
    // Decyzja
    if (temperature > WRAP_TEMP_MAX) {
        LOG_ERROR("Przegrzanie aplikatora! T=%.1f°C", temperature);
        return WRAP_OVERTEMP;
    }
    
    if (resistanceValue < 20) {
        LOG_ERROR("Zwarcie aplikatora okrężnego!");
        return WRAP_SHORT;
    } else if (resistanceValue >= WRAP_CONNECTED_MIN && 
               resistanceValue <= WRAP_CONNECTED_MAX) {
        LOG_INFO("Aplikator okrężny wykryty (R=%d, T=%.1f°C)", 
                 resistanceValue, temperature);
        return WRAP_READY;
    } else if (resistanceValue > WRAP_OPEN_CIRCUIT) {
        LOG_WARNING("Przerwana cewka aplikatora!");
        return WRAP_OPEN;
    } else {
        LOG_WARNING("Nieznany stan aplikatora (R=%d)", resistanceValue);
        return WRAP_CONNECTED;  // Podłączony ale wymaga kalibracji
    }
}
```

### Schemat Detekcji

```
                    +5V
                     │
                    [10kΩ] Pull-up
                     │
         ┌───────────┴───────────┐
         │                       │
    ┌────┴────┐             ┌────┴────┐
    │ DETECT  │             │ CURRENT │
    │  (D10)  │             │  (A5)   │
    │         │             │         │
    └────┬────┘             └────┬────┘
         │                       │
         │                      ┌┴┐
         │                      │ │ Current Sense
         │                      └┬┘
         │                      [0.33Ω]
         │                       │
         └───────────┬───────────┘
                     │
         ┌───────────┴───────────┐
         │   Aplikator Okrężny   │
         │      (Coil + NTC)     │
         │                       │
         │  ┌───────────────┐    │
         │  │     NTC       │    │
         │  │  (Temp sense) │    │
         │  └───────┬───────┘    │
         │          │            │
         └──────────┼────────────┘
                    │
                   GND
```

---

## 💊 Protokoły Terapeutyczne

### Joint Pain Relief (Stawy)
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 10-30 Hz |
| **Modulacja** | Sinusoidalna AM |
| **Natężenie pola** | 0.5-2 mT |
| **Czas sesji** | 20-30 minut |
| **Frekwencja** | 2-3x dziennie |

### Spine Therapy (Kręgosłup)
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 5-15 Hz |
| **Modulacja** | Sweep 5-15 Hz co 60s |
| **Natężenie pola** | 1-3 mT |
| **Czas sesji** | 30-45 minut |
| **Pozycja** | Leżąca na plecach |

### Muscle Recovery (Regeneracja mięśni)
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 727 Hz (rezonansowa) |
| **Modulacja** | Burst 2 Hz |
| **Natężenie pola** | 0.3-1 mT |
| **Czas sesji** | 15-20 minut |
| **Timing** | Po treningu |

### Bone Healing (Gojenie kości)
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 15-20 Hz |
| **Modulacja** | Pulsy prostokątne |
| **Natężenie pola** | 0.1-0.5 mT |
| **Czas sesji** | 60-120 minut |
| **Frekwencja** | 2-4x dziennie |

### Lymphatic Drainage (Drenaż limfatyczny)
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 0.5-2 Hz |
| **Modulacja** | Fala przemieszczająca się* |
| **Natężenie pola** | 0.2-0.8 mT |
| **Czas sesji** | 30-45 minut |
| **Kierunek** | Od dystalnego do proksymalnego |

*Wymaga sekwencyjnego sterowania wieloma cewkami

---

## ⚠️ Bezpieczeństwo

### Przeciwwskazania
- **Rozruszniki serca**: Ryzyko zakłóceń
- **Ciąża**: Nie stosować na brzuch i miednicę
- **Aktywne krwawienie**: Może nasilić krwawienie
- **Ostre stany zapalne**: Konsultacja z lekarzem
- **Implanty metaliczne**: Możliwe nagrzewanie

### Środki Ostrożności
1. **Nie zaciskaj za mocno**: Utrzymuj komfortowy ucisk
2. **Monitoruj skórę**: Sprawdzaj podrażnienia po sesji
3. **Unikaj włosów**: Może powodować dyskomfort przy usuwaniu
4. **Chłodzenie**: Przerwy między długimi sesjami

### Monitorowanie Temperatury

System stale monitoruje temperaturę aplikatora:

```cpp
void monitorWrapTemperature() {
    static uint32_t lastCheck = 0;
    
    if (millis() - lastCheck >= 1000) {  // Co 1 sekundę
        int tempRaw = analogRead(PIN_WRAP_TEMP);
        float temperature = 50.0 - (tempRaw * 50.0 / 1024.0);
        
        if (temperature > 42.0) {
            LOG_WARNING("Wzrost temperatury: %.1f°C", temperature);
            // Redukuj moc o 20%
            currentIntensity *= 0.8;
        }
        
        if (temperature > 45.0) {
            LOG_ERROR("KRYTYCZNA temperatura! %.1f°C", temperature);
            // Natychmiastowe zatrzymanie
            emergencyStop();
        }
        
        lastCheck = millis();
    }
}
```

### Alarmy Systemowe
| Alarm | Warunek | Akcja |
|-------|---------|-------|
| **WRAP_OPEN** | Brak ciągłości | Blokada startu |
| **WRAP_SHORT** | Zwarcie cewki | STOP + alarm |
| **OVERTEMP** | T > 45°C | Emergency shutdown |
| **POOR_CONTACT** | Zbyt luźno | Ostrzeżenie |
| **OVERCURRENT** | I > I_max | Redukcja mocy |

---

## 📎 Powiązane Dokumenty

- [Arduino Documentation](../arduino.md) - Konfiguracja PWM
- [Hardware Documentation](../hardware.md) - Schematy połączeń
- [Helmholtz Coil](effector_helmholtz_coil.md) - Cewka Helmholtza
- [Contact Electrodes](effector_contact_electrodes.md) - Elektrody kontaktowe
- [Safety Guidelines](safety_guidelines.md) - Procedury bezpieczeństwa

---

**Wersja dokumentacji**: 1.0  
**Data**: 2024  
**Autor**: ResoNet Development Team
