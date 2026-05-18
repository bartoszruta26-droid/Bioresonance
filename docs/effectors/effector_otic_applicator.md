# 🦻 Aplikator Uszny (Otic) - Wysokie Częstotliwości

<div align="center">

![Typ](https://img.shields.io/badge/typ-effektor%20kontaktowy-blue)
![Zakres](https://img.shields.io/badge/zakres%20cz%C4%99stotliwo%C5%9Bci-1kHz-500kHz-green)
![Zastosowanie](https://img.shields.io/badge/zastosowanie-terapia%20uszna-orange)
![Bezpieczeństwo](https://img.shields.io/badge/bezpieczeństwo-IEC%2060601--1-red)

**Wysokoczęstotliwościowy aplikator do terapii schorzeń uszu i głowy**

</div>

---

## 📋 Spis Treści

1. [Opis i Zastosowanie](#opis-i-zastosowanie)
2. [Specyfikacja Techniczna](#specyfikacja-techniczna)
3. [Podłączenie do Arduino](#podłączenie-do-arduino)
4. [Wykrywanie Podłączenia](#wykrywanie-podłączenia)
5. [Protokoły Terapeutyczne](#protokoły-terapeutyczne)
6. [Bezpieczeństwo](#bezpieczeństwo)

---

## 🏥 Opis i Zastosowanie

Aplikator uszny (otic applicator) to wyspecjalizowany efektor przeznaczony do aplikacji sygnałów elektromagnetycznych bezpośrednio do przewodu słuchowego. Wykorzystuje **wysokie częstotliwości** (1 kHz - 500 kHz) dla optymalnej penetracji tkanek.

### Wskazania Terapeutyczne
- **Szumy uszne (Tinnitus)**: Terapia modulacją AM/FM
- **Niedosłuch odbiorczy**: Stymulacja komórek rzęsatych
- **Infekcje ucha**: Wspomaganie leczenia przeciwzapalnego
- **Zawroty głowy**: Wpływ na układ przedsionkowy
- **Migreny**: Punkty akupunkturowe wokół ucha

### Zalety Aplikatora Usznego
| Cecha | Korzyść |
|-------|---------|
| **Wysoka częstotliwość** | Lepsza penetracja, mniejsze nagrzewanie |
| **Ergonomiczny kształt** | Idealne dopasowanie do anatomii ucha |
| **Izolacja galwaniczna** | Bezpieczeństwo pacjenta |
| **Wymienna końcówka** | Higiena, możliwość sterylizacji |

---

## ⚙️ Specyfikacja Techniczna

### Parametry Elektryczne
| Parametr | Wartość | Uwagi |
|----------|---------|-------|
| **Impedancja** | 8-32 Ω | Dopasowanie do skóry |
| **Pasmo przenoszenia** | 1 kHz - 500 kHz | -3 dB |
| **Napięcie wyjściowe** | 0-12 V pp | Regulowane |
| **Prąd maksymalny** | 50 mA | Ograniczenie bezpieczeństwa |
| **Pojemność** | <100 pF | Minimalizacja strat HF |

### Parametry Mechaniczne
| Parametr | Wartość | Opis |
|----------|---------|------|
| **Średnica końcówki** | 6-8 mm | Standardowa anatomia |
| **Długość aktywnej części** | 15-20 mm | Głębokość wprowadzenia |
| **Materiał** | Medical-grade silicone + Ag/AgCl | Biokompatybilny |
| **Kabel** | Ekranowany, 1.5m | Minimalizacja zakłóceń |
| **Złącze** | 3.5mm jack / BNC | Szybka wymiana |

### Charakterystyka Częstotliwościowa
| Pasmo | Zakres | Zastosowanie |
|-------|--------|--------------|
| **LF** | 1-100 Hz | Relaksacja, redukcja bólu |
| **MF** | 100 Hz - 10 kHz | Stymulacja nerwów |
| **HF** | 10 kHz - 500 kHz | Głęboka penetracja, szumy uszne |

---

## 🔌 Podłączenie do Arduino

### Schemat Połączeń

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│   Arduino Nano  │         │   Driver HF      │         │ Aplikator Uszny │
│                 │         │                  │         │                 │
│   Pin 9 (PWM)   │────────▶│ Input (Buffer)   │         │   Tip (HOT)     │
│   GND           │────────▶│ GND              │────────▶│   Ring (GND)    │
│   D4            │────────▶│ Enable           │         │                 │
│                 │         │   Output ────────┼────────▶│                 │
└─────────────────┘         └──────────────────┘         └─────────────────┘
```

### Pinout Arduino

| Pin Arduino | Funkcja | Opis |
|-------------|---------|------|
| **D9 (OC1A)** | PWM_OUT | Sygnał główny (Timer1) |
| **D4** | ENABLE | Włączanie drivera (HIGH = active) |
| **GND** | Masa | Wspólna masa |
| **A2** | IMPEDANCE | Pomiar impedancji (AC coupling) |
| **D5** | DETECT | Wykrywanie podłączenia |

### Komponenty Drivera HF

| Element | Typ | Wartość | Opis |
|---------|-----|---------|------|
| **U1** | Op-Amp | OPA2134 / LM4562 | Audio grade, niskie szumy |
| **Q1/Q2** | Transistors | BC547/BC557 | Push-pull output |
| **R1** | Current Limit | 220Ω | Ochrona przed zwarciem |
| **C1** | DC Block | 10µF bipolar | Separacja DC |
| **D1/D2** | Clamping | 1N4148 | Ograniczenie napięcia |

---

## 🔍 Wykrywanie Podłączenia

System wykrywa podłączenie aplikatora usznego poprzez pomiar impedancji charakterystycznej.

### Metoda Detekcji

```cpp
// Piny detekcji
#define PIN_OTIC_DETECT     5    // Pin cyfrowy
#define PIN_OTIC_IMPEDANCE  A2   // Pomiar analogowy

// Progi detekcji
#define OTIC_CONNECTED_MIN   100   // Min wartość ADC
#define OTIC_CONNECTED_MAX   600   // Max wartość ADC
#define OTIC_OPEN_CIRCUIT    900   // Brak połączenia
```

### Algorytm Wykrywania

1. **Test AC**: Generowanie sygnału 10 kHz przez 5ms
2. **Pomiar**: Odczyt wartości RMS na pinie IMPEDANCE
3. **Walidacja**: Sprawdzenie czy impedancja w zakresie 8-32 Ω
4. **Decyzja**:
   - ADC 100-600 → Aplikator podłączony
   - ADC > 900 → Brak aplikatora
   - ADC < 100 → Zwarcie (błąd!)

### Implementacja w Arduino

```cpp
bool detectOticApplicator() {
    // Włącz driver
    digitalWrite(PIN_OTIC_ENABLE, HIGH);
    
    // Generuj testowy sygnał 10 kHz
    uint32_t freq_x100 = 10000 * 100;  // 10 kHz
    pwm_set_frequency(freq_x100);
    analogWrite(PIN_PWM_OUTPUT, 64);  // 25% duty cycle
    delayMicroseconds(5000);
    
    // Pomiar impedancji (średnia z 10 próbek)
    uint32_t sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(PIN_OTIC_IMPEDANCE);
        delayMicroseconds(100);
    }
    int impedanceValue = sum / 10;
    
    // Zatrzymaj PWM
    analogWrite(PIN_PWM_OUTPUT, 0);
    digitalWrite(PIN_OTIC_ENABLE, LOW);
    
    // Decyzja
    if (impedanceValue >= OTIC_CONNECTED_MIN && 
        impedanceValue <= OTIC_CONNECTED_MAX) {
        LOG_INFO("Aplikator uszny wykryty (Z=%d)", impedanceValue);
        return true;
    } else if (impedanceValue > OTIC_OPEN_CIRCUIT) {
        LOG_WARNING("Brak aplikatora usznego");
        return false;
    } else {
        LOG_ERROR("Zwarcie aplikatora! (Z=%d)", impedanceValue);
        return false;  // Błąd - zwarcie
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
    │ DETECT  │             │IMPEDANCE│
    │  (D5)   │             │  (A2)   │
    │         │             │         │
    └────┬────┘             └────┬────┘
         │                       │
         │                      ┌┴┐
         │                      │ │ AC Coupling
         │                      └┬┘
         │                      [10µF]
         │                       │
         └───────────┬───────────┘
                     │
    ┌────────────────┴────────────────┐
    │       Aplikator Uszny           │
    │    (Tip + Ring electrodes)      │
    └─────────────────────────────────┘
```

---

## 💊 Protokoły Terapeutyczne

### Tinnitus Relief (Szumy Uszne)
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość nośna** | 100 kHz |
| **Modulacja AM** | 10 Hz, 50% depth |
| **Czas sesji** | 20 minut |
| **Intensywność** | Umiarkowana (6/10) |
| **Frekwencja** | 2x dziennie |

### Hearing Enhancement
| Parametr | Wartość |
|----------|---------|
| **Sweep frequency** | 1-10 kHz (co 30s) |
| **Modulacja FM** | ±5%, 0.5 Hz |
| **Czas sesji** | 15 minut |
| **Intensywność** | Niska (4/10) |
| **Frekwencja** | 1x dziennie |

### Pain Relief (Przeciwbólowy)
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 727 Hz (rezonansowa) |
| **Modulacja Burst** | 1 Hz, 50% duty |
| **Czas sesji** | 10 minut |
| **Intensywność** | Wysoka (8/10) |
| **Frekwencja** | W razie potrzeby |

### Vestibular Therapy (Przedionkowy)
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 40 Hz (gamma) |
| **Modulacja** | Brak (ciągły) |
| **Czas sesji** | 30 minut |
| **Intensywność** | Bardzo niska (3/10) |
| **Frekwencja** | Codziennie rano |

---

## ⚠️ Bezpieczeństwo

### Przeciwwskazania
- **Perforacja błony bębenkowej**: Ryzyko uszkodzenia
- **Aktywne infekcje ucha**: Możliwe pogorszenie
- **Implanty ślimakowe**: Zakłócenia elektroniczne
- **Nadwrażliwość słuchowa**: Dyskomfort

### Środki Ostrożności
1. **Higiena**: Jednorazowe nakładki silikonowe
2. **Sterylizacja**: Końcówki autoklawowalne (121°C, 15 min)
3. **Ograniczenie intensywności**: Maksymalnie 12 Vpp
4. **Monitorowanie komfortu**: Natychmiastowe przerwanie przy bólu

### Alarmy Systemowe
| Alarm | Warunek | Akcja |
|-------|---------|-------|
| **IMPEDANCE_HIGH** | Z > 100 Ω | Sprawdź połączenie |
| **IMPEDANCE_LOW** | Z < 5 Ω | Zwarcie - STOP |
| **OVERCURRENT** | I > 50 mA | Redukcja mocy |
| **DISCOMFORT** | Pacjent sygnalizuje | Pause/Stop |

### Normy i Certyfikaty
- **IEC 60601-1**: Bezpieczeństwo sprzętu medycznego
- **IEC 60601-2-10**: Szczegółowe wymagania dla stymulatorów nerwów
- **ISO 10993**: Biokompatybilność materiałów
- **CE Class IIa**: Urządzenie medyczne średniego ryzyka

---

## 📎 Powiązane Dokumenty

- [Arduino Documentation](../arduino.md) - Konfiguracja PWM
- [Hardware Documentation](../hardware.md) - Schematy połączeń
- [Contact Electrodes](effector_contact_electrodes.md) - Elektrody kontaktowe
- [Safety Guidelines](safety_guidelines.md) - Procedury bezpieczeństwa

---

**Wersja dokumentacji**: 1.0  
**Data**: 2024  
**Autor**: ResoNet Development Team
