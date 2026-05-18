# 🧲 Cewka Helmholtza - Pole Jednorodne do Badań

<div align="center">

![Typ](https://img.shields.io/badge/typ-effektor%20EMF-blue)
![Zakres](https://img.shields.io/badge/zakres%20cz%C4%99stotliwo%C5%9Bci-0.1Hz-500kHz-green)
![Pole](https://img.shields.io/badge/pole-jednorodne-orange)
![Zastosowanie](https://img.shields.io/badge/zastosowanie-badania%20naukowe-red)

**Generator jednorodnego pola magnetycznego do precyzyjnych badań biologicznych i kalibracji**

</div>

---

## 📋 Spis Treści

1. [Opis i Zastosowanie](#opis-i-zastosowanie)
2. [Specyfikacja Techniczna](#specyfikacja-techniczna)
3. [Podłączenie do Arduino](#podłączenie-do-arduino)
4. [Wykrywanie Podłączenia](#wykrywanie-podłączenia)
5. [Kalibracja i Użytkowanie](#kalibracja-i-użytkowanie)
6. [Bezpieczeństwo](#bezpieczeństwo)

---

## 🔬 Opis i Zastosowanie

Cewka Helmholtza to układ dwóch identycznych cewek kołowych umieszczonych współosiowo w odległości równej ich promieniowi. Generuje **wyjątkowo jednorodne pole magnetyczne** w obszarze między cewkami, co czyni ją idealnym narzędziem do:

### Zastosowania Badawcze
- **Badania biologiczne**: Wpływ pola magnetycznego na komórki, tkanki, organizmy
- **Kalibracja sensorów**: Magnetometrów, czujników Halla
- **Terapia PEMF**: Precyzyjne pole impulsowe dla całych obszarów ciała
- **Neurostymulacja**: Jednorodne pole dla mózgu (badania nad depresją, migreną)
- **Testy EMC**: Odporność urządzeń na pole magnetyczne

### Zalety Cewki Helmholtza
| Cecha | Korzyść |
|-------|---------|
| **Jednorodność** | ±1% w obszarze ⅓ promienia cewki |
| **Powtarzalność** | Identyczne warunki dla każdej sesji |
| **Obliczalność** | Pole można dokładnie obliczyć ze wzoru |
| **Skalowalność** | Możliwość zmiany rozmiaru dla różnych aplikacji |

---

## ⚙️ Specyfikacja Techniczna

### Parametry Elektryczne
| Parametr | Wartość | Uwagi |
|----------|---------|-------|
| **Indukcyjność** | 50-500 µH | Zależnie od rozmiaru i liczby zwojów |
| **Rezystancja DC** | 1-10 Ω | Grubszy drut = niższa rezystancja |
| **Prąd Maksymalny** | 0.5-5 A | Ograniczony termicznie |
| **Impedancja @1kHz** | ~3-30 Ω | Reaktancja indukcyjna |
| **Napięcie Robocze** | 5-24 V DC | Zależnie od konfiguracji |

### Parametry Pola Magnetycznego
| Parametr | Wartość | Wzór/Jednostka |
|----------|---------|----------------|
| **Natężenie Pola (B)** | 0.1-10 mT | `B = (8μ₀NI)/(√125 R)` |
| **Jednorodność** | ±0.5-2% | W obszarze środkowym |
| **Częstotliwość** | 0.1 Hz - 500 kHz | Generator PWM Arduino |
| **Kształt Fali** | Sinus/Prostokąt/Burst | Modulowany PWM |

### Wymiary Mechaniczne (Przykład)
| Rozmiar | Promień (R) | Odległość | Średnica Drutu | Zwoje |
|---------|-------------|-----------|----------------|-------|
| **Mała** | 5 cm | 5 cm | 0.8 mm | 20 |
| **Średnia** | 10 cm | 10 cm | 1.0 mm | 30 |
| **Duża** | 20 cm | 20 cm | 1.5 mm | 40 |

---

## 🔌 Podłączenie do Arduino

### Schemat Połączeń

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│   Arduino Nano  │         │   MOSFET Driver  │         │  Cewka Helmholtsa│
│                 │         │                  │         │                 │
│   Pin 9 (PWM)   │────────▶│ Gate (MOSFET)    │         │   + (HOT)       │
│   GND           │────────▶│ Source (MOSFET)  │────────▶│   - (COLD)      │
│   5V            │────────▶│ VCC (Driver)     │         │                 │
│                 │         │   Drain ─────────┼────────▶│                 │
└─────────────────┘         └──────────────────┘         └─────────────────┘
```

### Pinout Arduino

| Pin Arduino | Funkcja | Opis |
|-------------|---------|------|
| **D9 (OC1A)** | PWM_OUT | Sygnał sterujący MOSFET (Timer1) |
| **GND** | Masa | Wspólna masa z driverem |
| **5V** | VCC | Zasilanie drivera (lub zewnętrzne) |
| **A1** | SENSE | Pomiar prądu (opcjonalny) |
| **D2** | DETECT | Wykrywanie podłączenia cewki |

### Komponenty Drivera

| Element | Typ | Wartość | Opis |
|---------|-----|---------|------|
| **Q1** | MOSFET | IRF540N / IRLZ44N | N-channel, logic-level |
| **R1** | Resistor | 100Ω | Gate resistor (ochrona) |
| **R2** | Pull-down | 10kΩ | Discharge gate |
| **D1** | Flyback | 1N5822 / 1N4007 | Dioda ochronna |
| **C1** | Capacitor | 100µF/25V | Filtr zasilania |

---

## 🔍 Wykrywanie Podłączenia

System automatycznie wykrywa podłączenie cewki Helmholtza poprzez pomiar impedancji.

### Metoda Detekcji

```cpp
// Piny detekcji
#define PIN_HELMHOLTZ_DETECT  2    // Pin cyfrowy z pull-up
#define PIN_HELMHOLTZ_SENSE   A1   // Pomiar analogowy prądu

// Progi detekcji
#define HELMHOLTZ_CONNECTED_THRESHOLD  800   // Wartość ADC (< = podłączona)
#define HELMHOLTZ_DISCONNECTED_VALUE   1023  // ADC przy braku połączenia
```

### Algorytm Wykrywania

1. **Standby**: Pin DETECT z pull-up HIGH (10kΩ)
2. **Test**: Krótki impuls PWM (1ms, 1kHz)
3. **Pomiar**: Odczyt ADC na pinie SENSE
4. **Decyzja**: 
   - ADC < 800 → Cewka podłączona
   - ADC ≥ 800 → Brak cewki

### Implementacja w Arduino

```cpp
bool detectHelmholtzCoil() {
    // Wysył testowy impuls
    analogWrite(PIN_PWM_OUTPUT, 128);  // 50% duty cycle
    delayMicroseconds(1000);
    
    // Pomiar prądu
    int senseValue = analogRead(PIN_HELMHOLTZ_SENSE);
    
    // Zatrzymaj PWM
    analogWrite(PIN_PWM_OUTPUT, 0);
    
    // Decyzja
    if (senseValue < HELMHOLTZ_CONNECTED_THRESHOLD) {
        LOG_INFO("Cewka Helmholtza wykryta");
        return true;
    } else {
        LOG_WARNING("Brak cewki Helmholtza");
        return false;
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
    │  DETECT │             │  SENSE  │
    │  (D2)   │             │  (A1)   │
    │         │             │         │
    └────┬────┘             └────┬────┘
         │                       │
         │                      [0.1Ω] Sense Resistor
         │                       │
         └───────────┬───────────┘
                     │
    ┌────────────────┴────────────────┐
    │        Cewka Helmholtza         │
    │                                 │
    └─────────────────────────────────┘
```

---

## 📊 Kalibracja i Użytkowanie

### Obliczanie Natężenia Pola

Wzór na natężenie pola w centrum cewki Helmholtza:

```
B = (8 × μ₀ × N × I) / (√125 × R)

Gdzie:
  B  - natężenie pola [Tesla]
  μ₀ - przenikalność magnetyczna próżni (4π×10⁻⁷ H/m)
  N  - liczba zwojów na cewkę
  I  - prąd [Amper]
  R  - promień cewki [metry]
```

### Przykład Obliczeniowy

Dla cewki o parametrach:
- N = 30 zwojów
- R = 0.1 m (10 cm)
- I = 2 A

```
B = (8 × 4π×10⁻⁷ × 30 × 2) / (√125 × 0.1)
B = (6.03×10⁻⁴) / (1.118)
B ≈ 5.4×10⁻⁴ T = 0.54 mT
```

### Procedura Kalibracji

1. **Podłącz cewkę** do systemu
2. **Uruchom tryb kalibracji**:
   ```
   Komenda: CALIBRATE_HELMHOLTZ
   ```
3. **Zmierz pole** magnetometrem referencyjnym
4. **Wprowadź korektę**:
   ```
   Komenda: SET_FIELD_CORRECTION <współczynnik>
   ```
5. **Zapisz profil** w EEPROM

### Profile Terapeutyczne

| Profil | Częstotliwość | Czas | Natężenie | Zastosowanie |
|--------|---------------|------|-----------|--------------|
| **RELAX** | 10 Hz | 20 min | 0.1 mT | Redukcja stresu |
| **FOCUS** | 40 Hz | 15 min | 0.2 mT | Koncentracja |
| **SLEEP** | 1 Hz | 30 min | 0.05 mT | Insomnia |
| **RESEARCH** | 0.1-100 Hz | Custom | 0.1-5 mT | Badania |

---

## ⚠️ Bezpieczeństwo

### Przeciwwskazania
- **Rozruszniki serca**: Pole może zakłócać pracę
- **Implanty metaliczne**: Możliwe nagrzewanie
- **Ciąża**: Brak wystarczających badań
- **Epilepsja**: Stroboskopowe efekty świetlne

### Środki Ostrożności
1. **Ograniczenie czasu**: Maksymalnie 60 minut na sesję
2. **Chłodzenie**: Przerwy między sesjami (co najmniej 10 min)
3. **Monitorowanie temperatury**: Cewka nie powinna przekraczać 45°C
4. **Odległość**: Minimum 1m od innych urządzeń elektronicznych

### Alarmy Systemowe
| Alarm | Warunek | Akcja |
|-------|---------|-------|
| **OVERTEMP** | T > 45°C | Zatrzymanie PWM |
| **OVERCURRENT** | I > I_max | Redukcja prądu |
| **DISCONNECT** | Brak cewki | Blokada startu |
| **IMPEDANCE_FAULT** | Z ≠ expected | Sprawdzenie połączeń |

---

## 📎 Powiązane Dokumenty

- [Arduino Documentation](../arduino.md) - Konfiguracja PWM
- [Hardware Documentation](../hardware.md) - Schematy połączeń
- [Safety Guidelines](safety_guidelines.md) - Procedury bezpieczeństwa
- [EMF Antenna](effector_emf_antenna.md) - Inne typy anten EMF

---

**Wersja dokumentacji**: 1.0  
**Data**: 2024  
**Autor**: ResoNet Development Team
