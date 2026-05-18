# 🔌 Elektrody Kontaktowe - Bezpośredni Kontakt ze Skórą

<div align="center">

![Typ](https://img.shields.io/badge/typ-effektor%20kontaktowy-blue)
![Zakres](https://img.shields.io/badge/zakres%20cz%C4%99stotliwo%C5%9Bci-0.1Hz-100kHz-green)
![Zastosowanie](https://img.shields.io/badge/zastosowanie-elektroterapia-orange)
![Bezpieczeństwo](https://img.shields.io/badge/bezpieczeństwo-IEC%2060601--1-red)

**Uniwersalne elektrody do bezpośredniej aplikacji sygnałów na skórę pacjenta**

</div>

---

## 📋 Spis Treści

1. [Opis i Zastosowanie](#opis-i-zastosowanie)
2. [Typy Elektrod](#typy-elektrod)
3. [Specyfikacja Techniczna](#specyfikacja-techniczna)
4. [Podłączenie do Arduino](#podłączenie-do-arduino)
5. [Wykrywanie Podłączenia](#wykrywanie-podłączenia)
6. [Protokoły Terapeutyczne](#protokoły-terapeutyczne)
7. [Bezpieczeństwo](#bezpieczeństwo)

---

## 🏥 Opis i Zastosowanie

Elektrody kontaktowe to podstawowy typ efektorów w systemie ResoNet, umożliwiający **bezpośrednią aplikację sygnałów elektrycznych i elektromagnetycznych** na skórę pacjenta. Wykorzystywane w szerokiej gamie terapii od TENS po stymulację mięśniową.

### Wskazania Terapeutyczne
- **TENS (Transcutaneous Electrical Nerve Stimulation)**: Przeciwbólowa
- **EMS (Electrical Muscle Stimulation)**: Stymulacja mięśni
- **Ionoforeza**: Transdermalne podawanie leków
- **Gojenie ran**: Stymulacja regeneracji tkanek
- **Redukcja obrzęków**: Drenaż limfatyczny

### Zalety Elektrod Kontaktowych
| Cecha | Korzyść |
|-------|---------|
| **Bezpośredni kontakt** | Maksymalna efektywność transmisji |
| **Niska impedancja** | Mniejsze straty energii |
| **Elastyczność** | Dopasowanie do różnych obszarów ciała |
| **Wielorazowość** | Ekonomiczne rozwiązanie |

---

## 🔬 Typy Elektrod

### 1. Elektrody Żelowe (Ag/AgCl)
**Zastosowanie**: Krótkoterminowe sesje, diagnostyka

| Parametr | Wartość |
|----------|---------|
| **Materiał** | Srebro/Chlorek Srebra |
| **Żel** | Przewodzący, hipoalergiczny |
| **Impedancja** | <5 kΩ @ 10 Hz |
| **Rozmiar** | 30x50 mm, 50x80 mm |
| **Żywotność** | Jednorazowe / 10-15 użyć |

### 2. Elektrody Silikonowe (Reusable)
**Zastosowanie**: Długoterminowa terapia, domowe użycie

| Parametr | Wartość |
|----------|---------|
| **Materiał** | Medical-grade silicone + carbon |
| **Kontakt** | Suchy lub z żelem |
| **Impedancja** | <10 kΩ @ 10 Hz |
| **Rozmiar** | Różne (okrągłe, prostokątne) |
| **Żywotność** | >1000 cykli |

### 3. Elektrody Tekstylne (Conductive Fabric)
**Zastosowanie**: Wearables, długotrwałe monitorowanie

| Parametr | Wartość |
|----------|---------|
| **Materiał** | Tkanina z włóknami srebrnymi |
| **Elastyczność** | Rozciągliwe 30-50% |
| **Impedancja** | <20 kΩ @ 10 Hz |
| **Pranie** | Do 50 cykli |
| **Komfort** | Bardzo wysoki |

### 4. Elektrody Węglowe (Carbon Rubber)
**Zastosowanie**: EMS, silna stymulacja mięśni

| Parametr | Wartość |
|----------|---------|
| **Materiał** | Guma węglowa przewodząca |
| **Grubość** | 2-5 mm |
| **Impedancja** | <2 kΩ |
| **Trwałość** | Bardzo wysoka |
| **Czyszczenie** | Woda z mydłem |

---

## ⚙️ Specyfikacja Techniczna

### Parametry Elektryczne
| Parametr | Wartość | Uwagi |
|----------|---------|-------|
| **Impedancja skóry+elektrody** | 500 Ω - 50 kΩ | Zależnie od przygotowania |
| **Pasmo przenoszenia** | DC - 100 kHz | Pełne spektrum |
| **Napięcie maksymalne** | 60 V DC / 120 V AC | Limit bezpieczeństwa |
| **Prąd maksymalny** | 100 mA | Ograniczenie hardware'owe |
| **Gęstość prądu** | <10 mA/cm² | Bezpieczna dla skóry |

### Parametry Mechaniczne
| Parametr | Wartość | Opis |
|----------|---------|------|
| **Kabel** | Ekranowany, 2m | Minimalizacja zakłóceń |
| **Złącze** | 2mm touchproof / DIN 42-802 | Standard medyczny |
| **Kolor** | Czerwony/Czarny | Polaryzacja |
| **Odporność** | IPX4 | Odporność na wilgoć |

---

## 🔌 Podłączenie do Arduino

### Schemat Połączeń

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│   Arduino Nano  │         │   Driver Bipolar │         │   Elektrody     │
│                 │         │                  │         │                 │
│   Pin 9 (PWM)   │────────▶│ Input (+)        │         │   RED (+)       │
│   Pin 3 (PWM)   │────────▶│ Input (-)        │         │   BLACK (-)     │
│   GND           │────────▶│ GND              │────────▶│   COMMON        │
│   D6            │────────▶│ Enable           │         │                 │
└─────────────────┘         └──────────────────┘         └─────────────────┘
```

### Pinout Arduino

| Pin Arduino | Funkcja | Opis |
|-------------|---------|------|
| **D9 (OC1A)** | PWM_PLUS | Sygnał dodatni (Timer1 A) |
| **D3 (OC2A)** | PWM_MINUS | Sygnał ujemny (Timer2 A) |
| **D6** | ENABLE | Włączanie drivera |
| **A3** | IMPEDANCE | Pomiar impedancji |
| **D7** | DETECT | Wykrywanie podłączenia |

### Komponenty Drivera Bipolarnego

| Element | Typ | Wartość | Opis |
|---------|-----|---------|------|
| **U1** | H-Bridge | L298N / TB6612FNG | Bipolar drive |
| **R_sense** | Current Sense | 0.5Ω/5W | Pomiar prądu |
| **C_out** | DC Block | 470µF/100V | Separacja DC |
| **D_clamp** | TVS | P6KE6V8CA | Ochrona przed przepięciem |

---

## 🔍 Wykrywanie Podłączenia

System wykrywa podłączenie elektrod i mierzy impedancję kontaktu ze skórą.

### Metoda Detekcji

```cpp
// Piny detekcji
#define PIN_ELECTRODE_DETECT    7    // Pin cyfrowy
#define PIN_ELECTRODE_IMPEDANCE A3   // Pomiar analogowy

// Progi detekcji
#define ELECTRODE_CONNECTED_MIN   200   // Min wartość ADC
#define ELECTRODE_CONNECTED_MAX   800   // Max wartość ADC (sucha skóra)
#define ELECTRODE_OPTIMAL_MIN     300   // Optimum min (wilgotna skóra)
#define ELECTRODE_OPTIMAL_MAX     600   // Optimum max
#define ELECTRODE_OPEN_CIRCUIT    950   // Brak połączenia
```

### Algorytm Wykrywania

1. **Test impulsu**: Generowanie bipolarnego impulsu 1ms
2. **Pomiar impedancji**: Odczyt ADC na pinie IMPEDANCE
3. **Ocena jakości kontaktu**:
   - 300-600 → Doskonały kontakt (wilgotna skóra)
   - 200-300 lub 600-800 → Akceptowalny kontakt
   - >950 → Brak elektrod
   - <200 → Zwarcie (błąd!)
4. **Decyzja**: Zezwolenie/blokada terapii

### Implementacja w Arduino

```cpp
typedef enum {
    CONTACT_EXCELLENT = 0,
    CONTACT_GOOD,
    CONTACT_ACCEPTABLE,
    CONTACT_POOR,
    CONTACT_OPEN,
    CONTACT_SHORT
} ContactQuality_t;

ContactQuality_t detectElectrodes() {
    // Włącz driver
    digitalWrite(PIN_ELECTRODE_ENABLE, HIGH);
    
    // Testowy impuls bipolarny (1ms)
    analogWrite(PIN_PWM_PLUS, 32);
    analogWrite(PIN_PWM_MINUS, 0);
    delayMicroseconds(500);
    analogWrite(PIN_PWM_PLUS, 0);
    analogWrite(PIN_PWM_MINUS, 32);
    delayMicroseconds(500);
    
    // Pomiar impedancji (średnia z 16 próbek)
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead(PIN_ELECTRODE_IMPEDANCE);
        delayMicroseconds(50);
    }
    int impedanceValue = sum / 16;
    
    // Zatrzymaj PWM
    analogWrite(PIN_PWM_PLUS, 0);
    analogWrite(PIN_PWM_MINUS, 0);
    digitalWrite(PIN_ELECTRODE_ENABLE, LOW);
    
    // Ocena jakości
    if (impedanceValue < 100) {
        LOG_ERROR("ZWARCIE elektrod! (Z=%d)", impedanceValue);
        return CONTACT_SHORT;
    } else if (impedanceValue >= ELECTRODE_CONNECTED_MIN && 
               impedanceValue <= ELECTRODE_OPTIMAL_MIN) {
        LOG_INFO("Dobry kontakt elektrod (Z=%d)", impedanceValue);
        return CONTACT_GOOD;
    } else if (impedanceValue > ELECTRODE_OPTIMAL_MIN && 
               impedanceValue <= ELECTRODE_OPTIMAL_MAX) {
        LOG_INFO("Doskonały kontakt elektrod (Z=%d)", impedanceValue);
        return CONTACT_EXCELLENT;
    } else if (impedanceValue > ELECTRODE_OPTIMAL_MAX && 
               impedanceValue <= ELECTRODE_CONNECTED_MAX) {
        LOG_WARNING("Akceptowalny kontakt (sucha skóra?) (Z=%d)", impedanceValue);
        return CONTACT_ACCEPTABLE;
    } else if (impedanceValue > ELECTRODE_OPEN_CIRCUIT) {
        LOG_WARNING("Brak elektrod!");
        return CONTACT_OPEN;
    } else {
        LOG_ERROR("Nieznany stan (Z=%d)", impedanceValue);
        return CONTACT_POOR;
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
    │  (D7)   │             │  (A3)   │
    │         │             │         │
    └────┬────┘             └────┬────┘
         │                       │
         │                      ┌┴┐
         │                      │ │ DC Block
         │                      └┬┘
         │                      [470µF]
         │                       │
         └───────────┬───────────┘
                     │
         ┌───────────┴───────────┐
         │                       │
    ┌────┴────┐             ┌────┴────┐
    │  RED (+)│             │BLACK (-)│
    │Elektroda│             │Elektroda│
    └────┬────┘             └────┬────┘
         │                       │
         └───────────┬───────────┘
                     │
                ┌────┴────┐
                │  SKÓRA  │
                └─────────┘
```

---

## 💊 Protokoły Terapeutyczne

### TENS - Pain Relief
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 80-120 Hz |
| **Szerokość impulsu** | 50-200 µs |
| **Modulacja** | Burst 2-3 Hz |
| **Czas sesji** | 20-30 minut |
| **Intensywność** | Do odczucia mrowienia |

### EMS - Muscle Stimulation
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 35-50 Hz |
| **Szerokość impulsu** | 200-400 µs |
| **Cykl pracy** | 5s ON / 10s OFF |
| **Czas sesji** | 15-20 minut |
| **Intensywność** | Widoczna kontrakcja |

### Ionoforeza
| Parametr | Wartość |
|----------|---------|
| **Tryb** | DC (stały prąd) |
| **Natężenie** | 0.1-0.5 mA/cm² |
| **Polaryzacja** | Zależna od leku |
| **Czas sesji** | 10-20 minut |
| **Elektrody** | Żelowe jednorazowe |

### Wound Healing
| Parametr | Wartość |
|----------|---------|
| **Częstotliwość** | 0.5-2 Hz (pulsy) |
| **Polaryzacja** | Anoda (faza gojenia) |
| **Czas sesji** | 30-60 minut |
| **Frekwencja** | 1-2x dziennie |
| **Elektrody** | Hydrożelowe |

---

## ⚠️ Bezpieczeństwo

### Przeciwwskazania
- **Rozrusznik serca**: Bezwzględne przeciwwskazanie
- **Ciąża**: Nie stosować na brzuch i dolną część pleców
- **Epilepsja**: Unikać okolic głowy i szyi
- **Nowotwory**: Nie stosować bezpośrednio na zmiany nowotworowe
- **Otwarte rany**: Tylko specjalistyczne protokoły

### Przygotowanie Skóry
1. **Oczyszczenie**: Umyć skórę wodą z mydłem
2. **Odtłuszczenie**: Przetrzeć alkoholem izopropylowym
3. **Nawilżenie**: Lekko zwilżyć elektrody (jeśli suche)
4. **Umiejscowienie**: Unikać włosów (ogolić jeśli konieczne)

### Środki Ostrożności
1. **Odległość elektrod**: Minimum 5 cm między elektrodami
2. **Unikanie ścieżek**: Nie umieszczać elektrod poprzecznie przez klatkę piersiową
3. **Sprawdzanie skóry**: Regularnie kontrolować podrażnienia
4. **Higiena**: Dezynfekcja elektrod wielorazowych

### Alarmy Systemowe
| Alarm | Warunek | Akcja |
|-------|---------|-------|
| **CONTACT_POOR** | Z > 50 kΩ | Zwiększ nawilżenie |
| **CONTACT_OPEN** | Brak elektrod | Zatrzymaj terapię |
| **CONTACT_SHORT** | Z < 100 Ω | Sprawdź izolację |
| **OVERCURRENT** | I > 100 mA | Natychmiastowy STOP |
| **SKIN_IRRITATION** | Pacjent sygnalizuje | Redukuj intensywność |

### Normy i Certyfikaty
- **IEC 60601-1**: Ogólne bezpieczeństwo sprzętu medycznego
- **IEC 60601-2-10**: Szczegółowe wymagania dla stymulatorów
- **ISO 10993**: Biokompatybilność materiałów
- **FDA 510(k)**: Rejestracja jako urządzenie klasy II

---

## 📎 Powiązane Dokumenty

- [Arduino Documentation](../arduino.md) - Konfiguracja PWM
- [Hardware Documentation](../hardware.md) - Schematy połączeń
- [Otic Applicator](effector_otic_applicator.md) - Aplikator uszny
- [Biofeedback Sensor](sensor_biofeedback.md) - Sensor biofeedback
- [Safety Guidelines](safety_guidelines.md) - Procedury bezpieczeństwa

---

**Wersja dokumentacji**: 1.0  
**Data**: 2024  
**Autor**: ResoNet Development Team
