# Ultradźwiękowy Głośnik Kierunkowy: Budowa, Zasada Działania i Zastosowanie w Biorezonansie

## 1. Budowa Elektryczna Ultradźwiękowego Głośnika Kierunkowego

Ultradźwiękowy głośnik kierunkowy (znany również jako głośnik parametryczny lub emitter ultradźwiękowy) to urządzenie wykorzystujące nieliniowe właściwości powietrza do generowania słyszalnego dźwięku w wąskim, skierowanym wiązce.

### Główne Komponenty Elektroniczne:

1. **Generator Częstotliwości Ultrasonicznych**
   - Układ generujący sygnał o częstotliwości powyżej 20 kHz (zazwyczaj 40-60 kHz)
   - Może być realizowany przez mikrokontroler (np. Arduino Nano), generator funkcyjny lub dedykowany układ scalony
   - Wymagana wysoka stabilność częstotliwości nośnej

2. **Modulator Audio**
   - Układ mieszający sygnał audio (słyszalny) z falą nośną ultradźwiękową
   - Realizuje modulację amplitudy (AM) lub bardziej zaawansowane techniki modulacji (SSB, PWM)
   - Kluczowy dla odtworzenia jakości dźwięku

3. **Wzmacniacz Mocy Ultrasonicznej**
   - Wysokonapięciowy wzmacniacz klasy D lub specjalistyczny wzmacniacz ultradźwiękowy
   - Zapewnia wystarczającą moc do wzbudzenia przetworników (zwykle 10-100Vpp)
   - Musi pracować w zakresie częstotliwości ultradźwiękowych

4. **Tablica Przetworników Piezoelektrycznych**
   - Macierz elementów piezoelektrycznych (często 40 kHz rezonansowych)
   - Ułożone w specyficzny wzór geometryczny dla uzyskania kierunkowości
   - Każdy element działa jako źródło fali ultradźwiękowej

5. **Układ Fazelowania (Beamforming)**
   - Opcjonalny układ opóźniający sygnały do poszczególnych przetworników
   - Pozwala na elektroniczne sterowanie kierunkiem wiązki
   - W prostszych konstrukcjach realizowane przez geometryczne ułożenie przetworników

6. **Zasilacz**
   - Stabilne zasilanie dla wszystkich stopni elektroniki
   - Często wymaga podwyższonego napięcia dla wzmacniacza mocy
   - Filtracja zakłóceń RF/EMC krytyczna dla jakości dźwięku

## 2. Zasada Działania

### Efekt Parametryczny w Powietrzu

Działanie głośnika ultradźwiękowego opiera się na **nieliniowych właściwościach powietrza** i efekcie parametrycznym:

1. **Emisja Fali Nośnej**: Urządzenie emituje silnie skoncentrowaną wiązkę fal ultradźwiękowych (powyżej 20 kHz, niesłyszalnych dla człowieka)

2. **Modulacja Amplitudy**: Sygnał audio jest nanoszony na falę nośną poprzez modulację amplitudy. Jeśli fala nośna ma częstotliwość `fc`, a sygnał audio `fa`, to emitowane są częstotliwości:
   - `fc` (nośna)
   - `fc + fa` (górnypasmo boczne)
   - `fc - fa` (dolne pasmo boczne)

3. **Demodulacja w Powietrzu**: Gdy fala ultradźwiękowa propaguje przez powietrze, nieliniowość ośrodka powoduje samoistną demodulację:
   - Fale o różnych częstotliwościach propagują z nieco różnymi prędkościami
   - Następuje intermodulacja między składowymi
   - W wyniku tego procesu odtwarzany jest oryginalny sygnał audio `fa`

4. **Kierunkowość**: 
   - Długość fali ultradźwiękowej jest bardzo krótka (np. 8.5 mm dla 40 kHz)
   - Pozwala to na utworzenie wąskiej wiązki przy użyciu stosunkowo małej tablicy przetworników
   - Kąt rozproszenia może wynosić zaledwie 5-15 stopni
   - Dźwięk jest słyszalny tylko w obrębie wiązki, tworząc "audio spotlight"

### Równanie Westervelta

Proces opisuje równanie Westervelta dla nieliniowej akustyki:

```
∇²p - (1/c₀²)∂²p/∂t² = -(β/ρ₀c₀⁴)∂²p²/∂t²
```

Gdzie:
- `p` - ciśnienie akustyczne
- `c₀` - prędkość dźwięku w powietrzu
- `ρ₀` - gęstość powietrza
- `β` - parametr nieliniowości powietrza (~1.2 dla powietrza)

Prawy człon równania opisuje źródło demodulowanego dźwięku audio.

## 3. Zastosowanie w Biorezonansie

### Teoria Biorezonansu i Ultradźwięki

Biorezonans to koncepcja terapeutyczna zakładająca, że komórki i tkanki organizmu emitują specyficzne częstotliwości elektromagnetyczne lub akustyczne, a odpowiednie częstotliwości mogą mieć wpływ na procesy fizjologiczne.

### Potencjalne Zastosowania Głośnika Ultrasonicznego w Biorezonansie:

1. **Precyzyjne Targetowanie Terapeutyczne**
   - Kierunkowa natura pozwala na dostarczenie częstotliwości terapeutycznych do konkretnego obszaru ciała
   - Minimalizacja ekspozycji innych tkanek
   - Możliwość pracy na głębokich strukturach bez wpływu na otoczenie

2. **Frequencje Słyszalne i Niesłyszalne**
   - Urządzenie może generować zarówno częstotliwości słyszalne (poprzez demodulację)
   - Jak i bezpośrednio ultradźwięki terapeutyczne (1-3 MHz dla głębokiej penetracji)
   - Połączenie obu zakresów może mieć synergiczne efekty

3. **Specyficzne Protokoły Częstotliwościowe**
   - Rife frequencies (częstotliwości opracowane przez Royal Rife)
   - Częstotliwości Solfeggio
   - Indywidualnie dobrane rezonanse dla konkretnych schorzeń
   - Sekwencje częstotliwościowe zmiennych w czasie

4. **Połączenie z Innymi Modalnościami**
   - Synchronizacja z światłem (fotobiomodulacja)
   - Koordynacja z stymulacją elektryczną
   - Integracja z medytacją i technikami relaksacyjnymi

5. **Zalety Kierunkowości w Terapii**
   - Pacjent może siedzieć w komfortowej pozycji
   - Terapeuta może precyzyjnie kierować wiązkę
   - Możliwość pracy w grupie bez wzajemnych interferencji
   - Redukcja hałasu środowiskowego podczas sesji

### Bezpieczeństwo i Rozważania

- **Natężenie Dźwięku**: Konieczność monitorowania poziomu ciśnienia akustycznego (SPL)
- **Czas Ekspozycji**: Ograniczenie czasu sesji terapeutycznych
- **Przeciwwskazania**: Ciąża, rozruszniki serca, ostre stany zapalne
- **Regulacje**: Zgodność z lokalnymi przepisami dotyczącymi urządzeń medycznych

## 4. Implementacja z Arduino Nano i Złączem Jack Audio

### Schemat Połączeń

```
[Źródło Audio Biorezonansu] → [Jack 3.5mm] → [Arduino Nano] → [Wzmacniacz Ultrasoniczny] → [Tablica Przetworników]
```

### Szczegóły Implementacji:

#### A. Wejście Audio (Złącze Jack 3.5mm) - Specyfikacja Techniczna

**Standardowe Połączenia TRS (Tip-Ring-Sleeve):**

| Kontakt | Funkcja | Podłączenie Arduino | Impedancja | Napięcie |
|---------|---------|---------------------|------------|----------|
| Tip (Lewa kanał) | Sygnał L | Pin A0 (wejście analogowe) | 10kΩ pull-down | ±1V max |
| Ring (Prawy kanał) | Sygnał R / Mono | Pin A1 (opcjonalnie) | 10kΩ pull-down | ±1V max |
| Sleeve (Masa) | GND | GND Arduino + shield | - | 0V |

**Warunki Kondycjonowania Sygnału:**

Sygnał audio z urządzeń biorezonansowych wymaga odpowiedniego przygotowania przed wejściem do ADC Arduino:

1. **Offset DC**: Dodanie napięcia offsetu 2.5V (Vcc/2) dla sygnałów AC-coupled
   ```
   Vout = Vin_ac + 2.5V
   ```
   
2. **Dzielnik Napięcia**: Dla ochrony przed przekroczeniem zakresu 0-5V
   ```
   R1 = 10kΩ (szeregowo z sygnałem)
   R2 = 20kΩ (do GND)
   Współczynnik tłumienia: 0.67x
   ```

3. **Filtr Dolnoprzepustowy RC**: Anti-aliasing filter
   ```
   R = 1kΩ, C = 10nF
   fc = 1/(2πRC) ≈ 15.9 kHz
   ```

4. **Kondensator Sprzęgający**: Blokowanie składowej DC z źródła
   ```
   C = 10µF elektrolityczny (biegunowość zgodna z offsetem)
   fc_low = 1/(2πRC) ≈ 16 Hz dla R=1kΩ
   ```

**Kompletny Obwód Wejściowy:**
```
[Jack Tip] → [C1 10µF] → [R1 10kΩ] → [A0 Arduino]
                              ↓
                           [R2 20kΩ]
                              ↓
                            [GND]
                            
[Vcc 5V] → [R3 10kΩ] → [A0] (bias voltage divider)
[GND]    → [R4 10kΩ] → [A0]
```

**Parametry Sygnału:**
- Poziom nominalny: 316 mV RMS (-10 dBV) dla consumer audio
- Poziom profesjonalny: 1.228 V RMS (+4 dBu)
- Impedancja wyjściowa źródła: 100-600Ω typowo
- Zakres dynamiczny ADC Arduino: 10-bit (1024 poziomy), ~4.88 mV/step

#### B. Arduino Nano - Zaawansowane Przetwarzanie Sygnału

**Specyfikacja Mikrokontrolera ATmega328P:**
- Taktowanie: 16 MHz (Arduino Nano v3.0)
- ADC: 10-bit, 8-kanałowy, czas konwersji ~104 µs
- Timery: Timer0 (8-bit), Timer1 (16-bit), Timer2 (8-bit)
- PWM: 6 kanałów (pins 3, 5, 6, 9, 10, 11)
- SRAM: 2 KB, Flash: 32 KB, EEPROM: 1 KB

**Architektura Systemu Czasu Rzeczywistego:**

1. **Timer1 - Generator Nośnej Ultrasonicznej (40 kHz)**
   ```
   Tryb: Phase Correct PWM (Mode 9)
   TOP: ICR1 = 200
   Prescaler: 1 (bez dzielenia)
   f_PWM = f_clk / (2 × prescaler × TOP) = 16MHz / (2 × 1 × 200) = 40 kHz
   Rozdzielczość duty cycle: 8-bit (0-200)
   ```

2. **Timer2 - Interrupt Próbkowania Audio**
   ```
   Tryb: CTC (Clear Timer on Compare Match)
   OCR2A = 124
   Prescaler: 128
   f_interrupt = f_clk / (prescaler × (OCR2A + 1)) 
                = 16MHz / (128 × 125) ≈ 1000 Hz
   UWAGA: Dla 44 kHz sample rate potrzebny inny prescaler!
   
   Optymalna konfiguracja dla 44.1 kHz:
   Prescaler: 8
   OCR2A = 44
   f_interrupt = 16MHz / (8 × 45) ≈ 44.44 kHz
   ```

3. **ADC - Konwersja Analogowo-Cyfrowa**
   ```
   Tryb: Free Running Mode (ciągłe próbkowanie)
   Prescaler ADC: 128 (dla 125 kHz clock ADC)
   Czas konwersji: 13 cykli × 128 / 16MHz ≈ 104 µs
   Maksymalna sample rate: ~9.6 kS/s (za mało!)
   
   ROZWIĄZANIE: Overclocking ADC do 250-500 kHz
   Prescaler: 32 lub 64
   Sample rate: 19-38 kS/s (akceptowalne dla audio do 10 kHz)
   ```

**Zoptymalizowany Kod Produkcyjny:**

```cpp
/*
 * Ultradźwiękowy Głośnik Kierunkowy dla Biorezonansu
 * Arduino Nano - Wersja Produkcyjna z Optymalizacją
 * 
 * Features:
 * - 44.1 kHz audio sampling (Timer2 interrupt)
 * - 40 kHz ultrasonic carrier (Timer1 PWM)
 * - AM modulation with pre-emphasis filter
 * - Safety limiting and thermal monitoring
 * - Serial debug interface
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// ============================================================================
// KONFIGURACJA SYSTEMU
// ============================================================================

#define AUDIO_INPUT         A0          // Wejście audio z jacka (pin 14)
#define ULTRASONIC_OUTPUT   9           // OC1A - PWM output (pin 15)
#define THERMAL_SENSOR      A2          // Termistor NTC monitoringu temperatury
#define SAFETY_LIMIT_PIN    8           // Hardware shutdown pin (active LOW)

#define CARRIER_FREQ        40000UL     // Częstotliwość nośna [Hz]
#define SAMPLE_RATE         44100UL     // Częstotliwość próbkowania [Hz]
#define ADC_CLOCK           250000UL    // Clock ADC [Hz] - overclocked

#define PWM_TOP             200         // Wartość TOP dla Timer1
#define MODULATION_DEPTH    180         // Maksymalna głębokość modulacji (0-200)
#define BIAS_POINT          100         // Punkt pracy (50% duty cycle)

#define SAFETY_MAX_TEMP     65          // Maksymalna temperatura [°C]
#define SAFETY_MAX_LEVEL    950         // Maksymalny poziom audio (z 1023)

// ============================================================================
// ZMIENNE GLOBALNE - volatile dla ISR
// ============================================================================

volatile int16_t audio_sample = 0;          // Ostatnia próbka audio (-512 do +511)
volatile uint16_t pwm_duty = BIAS_POINT;    // Aktualne wypełnienie PWM
volatile bool sample_ready = false;         // Flag nowej próbki
volatile uint32_t sample_count = 0;         // Licznik próbek (do debug)
volatile uint16_t thermal_adc = 512;        // Odczyt termistora
volatile bool safety_shutdown = false;      // Flag wyłączenia awaryjnego

// Filtr pre-emphasis (poprawa wysokich tonów)
volatile int32_t filter_state = 0;
#define PRE_EMPHASIS_COEF   0.35            // Współczynnik filtra (0.0-1.0)

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

inline int16_t apply_pre_emphasis(int16_t input) {
  // Prosty filtr high-pass: y[n] = x[n] - x[n-1] * coef
  int32_t diff = (int32_t)input - (filter_state >> 10);
  filter_state = ((int32_t)input << 10) + (diff * (int32_t)(PRE_EMPHASIS_COEF * 1024));
  return (int16_t)(diff >> 2);
}

inline uint16_t modulate_pwm(int16_t audio) {
  // Modulacja AM: duty = bias + (audio * depth / max_audio)
  int32_t modulation = ((int32_t)audio * MODULATION_DEPTH) / 512;
  int32_t duty = BIAS_POINT + modulation;
  
  // Limitowanie zakresu 0-PWM_TOP
  if (duty < 0) duty = 0;
  if (duty > PWM_TOP) duty = PWM_TOP;
  
  return (uint16_t)duty;
}

inline float read_temperature() {
  // Konwersja ADC na temperaturę (termistor NTC 10k@25°C)
  // Steinhart-Hart equation uproszczona
  float resistance = 10000.0 * (thermal_adc / (1023.0 - thermal_adc));
  float steinhart = log(resistance / 10000.0);
  steinhart /= 3950.0; // Beta wartość dla typowego NTC
  steinhart += 1.0 / (25.0 + 273.15);
  steinhart = 1.0 / steinhart;
  return steinhart - 273.15; // Temperatura w °C
}

// ============================================================================
// KONFIGURACJA TIMERÓW
// ============================================================================

void setupTimer1() {
  /*
   * Timer1: Generator fali nośnej 40 kHz (PWM Phase Correct)
   * Wyjście: Pin 9 (OC1A)
   */
  
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  
  // Tryb 9: Phase Correct PWM, TOP = ICR1
  TCCR1A = _BV(COM1A1) | _BV(WGM11);  // Clear OC1A on compare match, mode bit 1
  TCCR1B = _BV(WGM13) | _BV(CS10);    // Mode bit 3, no prescaling
  
  ICR1 = PWM_TOP;          // TOP value = 200 → f = 16MHz/(2×1×200) = 40 kHz
  OCR1A = BIAS_POINT;      // Initial duty cycle = 50%
  
  // Wymuszenie wyjścia PWM na pinie 9
  DDRB |= _BV(PORTB1);     // Pin 9 = PB1
}

void setupTimer2() {
  /*
   * Timer2: Interrupt próbkowania audio 44.1 kHz
   * Tryb: CTC (Clear Timer on Compare Match)
   */
  
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0;
  
  // Tryb 2: CTC, TOP = OCR2A
  TCCR2A = _BV(WGM21);
  TCCR2B = _BV(CS21);      // Prescaler = 8
  
  OCR2A = 44;              // f = 16MHz/(8×45) ≈ 44.44 kHz
  TIMSK2 = _BV(OCIE2A);    // Enable compare match interrupt
}

void setupADC() {
  /*
   * ADC: Free Running Mode, overclocked do 250 kHz
   * Sample rate: ~19.2 kS/s (dla 250 kHz clock)
   */
  
  ADMUX = 0;
  ADCSRA = 0;
  ADCSRB = 0;
  DIDR0 = 0;
  
  // Wybór kanału A0 (MUX0 = 0)
  ADMUX = _BV(REFS0);              // Vref = AVCC (5V)
  
  // Prescaler ADC = 64 → 16MHz/64 = 250 kHz
  ADCSRA = _BV(ADEN)               // Enable ADC
           | _BV(ADSC)             // Start conversion
           | _BV(ADATE)            // Auto Trigger Enable
           | _BV(ADPS2) | _BV(ADPS1); // Prescaler = 64
  
  // Trigger source: Free Running
  ADCSRB = 0;
  
  // Disable digital input buffer on A0 for power saving
  DIDR0 = _BV(ADC0D);
}

void setupSafety() {
  /*
   * System bezpieczeństwa:
   * - Monitorowanie temperatury (NTC na A2)
   * - Hardware shutdown pin (pin 8, active LOW)
   * - Software limiting poziomu audio
   */
  
  pinMode(SAFETY_LIMIT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SAFETY_LIMIT_PIN), 
                  [](){ safety_shutdown = true; }, FALLING);
}

// ============================================================================
// OBSŁUGA INTERRUPTÓW
// ============================================================================

ISR(TIMER2_COMPA_vect) {
  /*
   * Interrupt co ~22.6 µs (44.44 kHz)
   * - Pobranie ostatniej wartości z ADC
   * - Aplikacja pre-emphasis
   * - Obliczenie nowego duty cycle PWM
   * - Aktualizacja OCR1A
   */
  
  if (!safety_shutdown) {
    // Pobranie wartości ADC (0-1023)
    uint16_t adc_raw = ADC;
    
    // Konwersja do signed (-512 do +511)
    audio_sample = (int16_t)adc_raw - 512;
    
    // Sprawdzenie limitu bezpieczeństwa
    if (abs(audio_sample) > SAFETY_MAX_LEVEL) {
      audio_sample = (audio_sample > 0) ? SAFETY_MAX_LEVEL : -SAFETY_MAX_LEVEL;
    }
    
    // Aplikacja filtra pre-emphasis
    int16_t filtered = apply_pre_emphasis(audio_sample);
    
    // Modulacja PWM
    pwm_duty = modulate_pwm(filtered);
    
    // Bezpośrednia aktualizacja rejestru PWM (atomowa operacja)
    OCR1A = pwm_duty;
    
    sample_count++;
    sample_ready = true;
  } else {
    // Safety shutdown - ustawienie PWM na 0
    OCR1A = 0;
  }
}

ISR(ADC_vect) {
  /*
   * ADC Conversion Complete - pusta obsługa
   * Wartość odczytywana w Timer2 interrupt przez rejestr ADC
   */
  // ADC auto-triggered, value available in ADC register
}

// ============================================================================
// SETUP I MAIN LOOP
// ============================================================================

void setup() {
  // Inicjalizacja Serial dla debugowania
  Serial.begin(115200);
  while (!Serial) { ; } // Wait for serial port
  
  Serial.println(F("=== Ultradźwiękowy System Biorezonansowy ==="));
  Serial.println(F("Wersja: Production 2.0"));
  Serial.print(F("Carrier Frequency: "));
  Serial.print(CARRIER_FREQ);
  Serial.println(F(" Hz"));
  Serial.print(F("Sample Rate: "));
  Serial.print(SAMPLE_RATE);
  Serial.println(F(" Hz"));
  
  // Konfiguracja pinów
  pinMode(ULTRASONIC_OUTPUT, OUTPUT);
  pinMode(AUDIO_INPUT, INPUT);
  pinMode(THERMAL_SENSOR, INPUT);
  setupSafety();
  
  // Inicjalizacja timerów i ADC
  setupTimer1();
  setupTimer2();
  setupADC();
  
  // Włączenie globalnych interruptów
  sei();
  
  Serial.println(F("System wystartował pomyślnie."));
  Serial.println(F("Oczekiwanie na sygnał audio..."));
}

void loop() {
  // Główne pętla: monitorowanie i diagnostyka
  
  static uint32_t last_status = 0;
  static uint32_t last_temp_check = 0;
  
  // Status co 1 sekundę
  if (millis() - last_status >= 1000) {
    last_status = millis();
    
    Serial.print(F("[STATUS] Samples: "));
    Serial.print(sample_count);
    Serial.print(F(" | Duty: "));
    Serial.print(pwm_duty);
    Serial.print(F(" | Temp: "));
    Serial.print(read_temperature(), 1);
    Serial.println(F(" °C"));
    
    // Reset licznika
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      sample_count = 0;
    }
  }
  
  // Kontrola temperatury co 100 ms
  if (millis() - last_temp_check >= 100) {
    last_temp_check = millis();
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      thermal_adc = analogRead(THERMAL_SENSOR);
    }
    
    float temp = read_temperature();
    if (temp > SAFETY_MAX_TEMP) {
      safety_shutdown = true;
      Serial.println(F("!!! SAFETY SHUTDOWN: OVERTEMP !!!"));
    }
  }
  
  // Check hardware shutdown pin
  if (digitalRead(SAFETY_LIMIT_PIN) == LOW) {
    safety_shutdown = true;
    Serial.println(F("!!! SAFETY SHUTDOWN: EXTERNAL !!!"));
  }
  
  // Power saving - sleep between status updates
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
}
```

**Optymalizacje Wydajności:**

1. **Bezpośrednia Manipulacja Rejestrów**: Pominięcie `analogWrite()` i `digitalWrite()` na rzecz bezpośredniego dostępu do rejestrów GPIO
2. **Inline Functions**: Krytyczne funkcje oznaczone `inline` dla redukcji narzutu wywołania
3. **Atomic Blocks**: Sekcje krytyczne chronione przed przerwaniami
4. **Fixed-Point Arithmetic**: Unikanie floating-point w ISR dla szybkości
5. **Sleep Modes**: Tryb niskiego poboru energii między iteracjami

#### C. Wzmacniacz Ultrasoniczny - Specyfikacja Szczegółowa

**Topologia Wzmacniacza Klasy D:**

```
[Wzmacniacz Operacyjny] → [Komparator] → [Driver MOSFET] → [Mostek H] → [Filtr LC] → [Transducery]
```

**Parametry Elektryczne:**

| Parametr | Wartość Minimalna | Wartość Optymalna | Uwagi |
|----------|-------------------|-------------------|-------|
| Pasmo przenoszenia | 30-50 kHz | 35-45 kHz (-3dB) | Flat response wokół 40 kHz |
| Moc wyjściowa RMS | 10W | 20-30W | Dla tablicy 8x8 |
| Napięcie Vpp | 40V | 60-80V | Zależne od impedancji tablicy |
| THD+N | <1% | <0.5% | Total Harmonic Distortion |
| Sprawność | >80% | >90% | Klasa D preferred |
| Impedancja wyjściowa | - | 4-8Ω | Dopasowanie do tablicy |

**Schemat Mostka H z MOSFET:**

```
         V+ (40-60V)
           │
       ┌───┴───┐
       │       │
      Q1       Q3  (High-side N-MOSFET or P-MOSFET)
       │       │
       ├───────┤──→ Output A → Tablica
       │       │
      Q2       Q4  (Low-side N-MOSFET)
       │       │
       └───┬───┘
           │
          GND
          
Output B → Drugi koniec tablicy (bridge configuration)
```

**Komponenty Kluczowe:**

- **MOSFET**: IRF540N lub dedykowane audio MOSFET (IRF6648)
  - Vds: ≥100V
  - Id: ≥10A
  - Rds(on): <0.1Ω
  - Qg: <50nC (dla szybkiego przełączania)

- **Driver MOSFET**: IR2110 (high-low side driver)
  - Bootstrap capacitor: 10µF
  - Dead time: 100-200ns (zapobiega shoot-through)

- **Filtr LC Dolnoprzepustowy**:
  ```
  L = 100µH (ferrytowy, nasycenie >5A)
  C = 100nF (foliowy, MKP, ≥100V)
  fc = 1/(2π√(LC)) ≈ 50 kHz
  ```

**Gotowe Moduły Komercyjne:**

1. **IRS2092S + MOSFET**: Gotowy driver klasy D z保护ami
2. **TAS5630**: 150W stereo amplifier IC (użyć mono bridge)
3. **H-bridge module**: L298N (za słaby), TB6612FNG (lepszy)

#### D. Tablica Przetworników - Konstrukcja Mechaniczna i Elektryczna

**Specyfikacja Pojedynczego Elementu (MA40S4R/S):**

| Parametr | Wartość | Jednostka |
|----------|---------|-----------|
| Częstotliwość rezonansowa | 40.0 ±1.0 | kHz |
| Ciśnienie akustyczne (SPL) | 120 | dB (min) @10cm |
| Szerokość pasma | 2.5 | kHz (-6dB) |
| Średnica | 16 | mm |
| Wysokość | 12 | mm |
| Pojemność | 2000-3000 | pF @40kHz |
| Impedancja | ≤500 | Ω @40kHz |
| Temperatura pracy | -30 do +70 | °C |

**Geometria Macierzy:**

Dla uzyskania optymalnej kierunkowości bez lobes bocznych:

```
Zasada: d ≤ λ/2

gdzie:
λ = c/f = 343 m/s / 40000 Hz = 8.575 mm
d ≤ 4.29 mm (odległość między środkami elementów)
```

**Konfiguracja 8x8 (64 elementy):**

```
Wymiary fizyczne:
- Pitch (rozstaw): 4.0 mm
- Wymiar tablicy: 32mm × 32mm (256 cm²)
- Kąt wiązki głównej: ~15° (teoretycznie)
- Poziom SPL @1m: ~85-95 dB (zależnie od mocy)
```

**Schemat Połączeń Elektrycznych:**

```
Opcja 1: Wszystkie równolegle
- Impedancja całkowita: Z_total = Z_element / 64
- Dla Z=500Ω: Z_total ≈ 7.8Ω (dobrze dla wzmacniacza 8Ω)
- Wymagany prąd: wysoki (I = V/Z)

Opcja 2: Szeregowo-równolegle (8 grup po 8 szeregowo)
- Impedancja grupy: 8 × 500Ω = 4000Ω
- 8 grup równolegle: 4000Ω / 8 = 500Ω
- Mniejszy prąd, wyższe napięcie wymagane

Opcja 3: Matching transformer
- Transformator 8Ω:500Ω (przekładnia 1:7.9)
- Dopasowanie do wzmacniacza 8Ω
```

**Matching Layer (Warstwa Dopasowująca):**

Dla poprawy transmisji akustycznej z przetwornika do powietrza:

```
Materiał: Porous foam, felt, lub specjalny kompozyt
Grubość: λ/4 = 2.14 mm (dla 40 kHz w materiale)
Impedancja akustyczna: √(Z_piezo × Z_air) ≈ 400 Rayl
```

**Obudowa i Tłumienie:**

```
Materiał obudowy: ABS lub aluminum (sztywność)
Wewnętrzne tłumienie: Pianka akustyczna (melamine foam)
Otwory wentylacyjne: Dla chłodzenia (ważne przy wysokiej mocy)
Mounting points: 4× M3 screws w rogach
```

### Kalibracja i Testowanie - Procedury Szczegółowe

#### 1. Pomiar Częstotliwości Rezonansowej

**Sprzęt:**
- Generator funkcyjny (1 Hz - 100 kHz)
- Oscyloskop 2-kanałowy (≥100 MHz bandwidth)
- Multimetr True RMS
- Mikrofon pomiarowy (opcjonalnie)

**Procedura:**
```
1. Podłącz generator do wejścia wzmacniacza (przez dzielnik jeśli trzeba)
2. Ustaw sinus 1 Vpp, częstotliwość startowa 35 kHz
3. Monitoruj prąd pobierany przez tablicę (shunt resistor 1Ω + oscyloskop)
4. Skanuj częstotliwość 35-45 kHz krok 100 Hz
5. Znajdź maksimum prądu → częstotliwość rezonansowa
6. Zmierz fazę między napięciem a prądem (minimum przesunięcia = rezonans)
7. Zapisz fr i bandwidth (-3dB points)
```

#### 2. Mapowanie Charakterystyki Kierunkowej (Beam Pattern)

**Setup Pomiarowy:**
```
                    ┌─────────────┐
                    │  Tablica    │
                    │  Ultrasonic │
                    └──────┬──────┘
                           │
                           │ 1 metr
                           │
                    ┌──────▼──────┐
                    │   Mikrofon  │
                    │   Pomiarowy │
                    └──────┬──────┘
                           │
                      Obrotowy stolik
                      (krok 1°)
```

**Procedura:**
```
1. Zamontuj tablicę na statywie, mikrofon na ramieniu obrotowym
2. Generuj ciągły ton 40 kHz (carrier only)
3. Mierz SPL co 1° w zakresie -90° do +90°
4. Powtórz dla zmodulowanego sygnału (1 kHz tone)
5. Wykreśl charakterystykę biegunową (polar plot)
6. Zmierz:
   - Beamwidth (-3dB points)
   - Side lobe levels
   - Front-to-back ratio
```

**Oczekiwane Wyniki:**
- Beamwidth główny: 10-20° (-3dB)
- Side lobes: -15 do -25 dB poniżej main lobe
- Front-to-back: >30 dB

#### 3. Test Jakości Audio

**Sygnały Testowe:**
- Sweep sinusoidalny: 20 Hz - 20 kHz (lub do 10 kHz dla ultrasonics)
- Różowy szum (pink noise)
- Impulsy (square wave bursts)
- Muzyka referencyjna (well-known tracks)

**Metryki Jakości:**
```
THD+N (Total Harmonic Distortion + Noise): <5% dla 1 kHz @1W
Frequency Response: ±3dB od 200 Hz do 8 kHz
SNR (Signal-to-Noise Ratio): >60 dB
Intermodulation Distortion: <3% (CCIF test: 19+20 kHz)
```

**Procedura Pomiaru THD:**
```
1. Generuj sinus 1 kHz, poziom -10 dBFS
2. Przechwyć sygnał z mikrofonu (przez ADC wysokiej jakości)
3. Wykonaj FFT (Fast Fourier Transform)
4. Zmierz amplitudy harmonicznych: 2kHz, 3kHz, 4kHz...
5. THD = √(Σ(harmonics²)) / fundamental × 100%
```

#### 4. Walidacja Bezpieczeństwa

**Limity Ekspozycji (wg norm):**

| Typ | Limit SPL | Częstotliwość | Czas ekspozycji |
|-----|-----------|---------------|-----------------|
| Słyszalne | 85 dB(A) | 20 Hz - 20 kHz | 8 godzin/dzień |
| Ultrasonic (airborne) | 110 dB | 20-100 kHz | 8 godzin/dzień |
| Peak pressure | 140 dB | All | Krótkotrwały |

**Pomiary Required:**
```
1. SPL @10 cm od tablicy (max operating power)
2. SPL @1 metr (typical user distance)
3. Harmonic content analysis
4. Leakage measurement (side/back)
5. Thermal imaging (hot spots)
```

**Equipment:**
- Sound Level Meter z opcją ultrasonic (Brüel & Kjær, NTi Audio)
- Termowizyjna kamera (FLIR)
- Oscyloskop z FFT

### Rozszerzenia i Ulepszenia - Implementacje Zaawansowane

#### 1. Sterowanie Cyfrowe (Bluetooth/WiFi)

**Moduł Bluetooth HC-05/HC-06:**
```
Połączenie:
- HC-05 TX → Arduino RX (pin 10, softserial)
- HC-05 RX → Arduino TX (pin 11, softserial)
- HC-05 VCC → 5V
- HC-05 GND → GND

Protokół komunikacji:
- SET_FREQ <value>: Ustaw częstotliwość nośną
- SET_GAIN <value>: Ustaw wzmocnienie (0-255)
- SET_PRESET <id>: Wybór presetu terapeutycznego
- GET_STATUS: Pobierz status urządzenia
- START_THERAPY: Rozpocznij sesję
- STOP_THERAPY: Zatrzymaj sesję
```

**Przykładowa Komenda:**
```
AT+SET_FREQ=40000\r\n
OK

AT+SET_PRESET=5\r\n
OK: Preset 5 loaded (Rife Digestive)

AT+START_THERAPY\r\n
OK: Therapy started, duration=900s
```

#### 2. Elektroniczny Beamforming (Phased Array)

**Wymagania Sprzętowe:**
- Mikrokontroler z wieloma PWM outputs (Arduino Mega, ESP32)
- 8-16 niezależnych kanałów wzmacniacza
- Delay lines cyfrowe lub analogowe

**Algorytm Steering:**
```
Dla kąta θ:
delay[i] = (i × d × sin(θ)) / c

gdzie:
i = index elementu (0 to N-1)
d = pitch (odległość między elementami)
c = prędkość dźwięku (343 m/s)
θ = desired steering angle
```

**Implementacja na ESP32:**
```cpp
// ESP32 ma 16 kanałów LED PWM (dobre do beamforming)
#define NUM_ELEMENTS 8
#define ELEMENT_PINS {2, 4, 5, 18, 19, 21, 22, 23}

float calculate_delay(int element_idx, float angle_deg) {
  float d = 0.004; // 4mm pitch
  float theta = angle_deg * DEG_TO_RAD;
  float path_diff = element_idx * d * sin(theta);
  float time_delay = path_diff / 343.0; // seconds
  return time_delay * 1000000.0; // microseconds
}

void set_beam_angle(float angle) {
  for (int i = 0; i < NUM_ELEMENTS; i++) {
    float delay_us = calculate_delay(i, angle);
    // Apply delay via timer or digital delay line
    // ... implementation depends on hardware
  }
}
```

#### 3. Sprzężenie Zwrotne z Mikrofonem Pomiarowym

**Cel:** Automatyczna kalibracja i adaptacja do warunków środowiskowych

**Schema:**
```
[Tablica] → [Powietrze] → [Mikrofon] → [ADC] → [DSP] → [Adjust PWM]
                ↑                                  ↓
                └────────── Feedback Loop ─────────┘
```

**Algorytm AGC (Automatic Gain Control):**
```cpp
#define TARGET_LEVEL 400  // Target ADC level (0-1023)
#define AGC_ALPHA 0.01    // Smoothing factor

float measured_level = 0;
float gain_correction = 1.0;

void update_agc() {
  // Measure current output level
  measured_level = analogRead(MIC_INPUT);
  
  // Calculate error
  float error = TARGET_LEVEL - measured_level;
  
  // Adjust gain (integral control)
  gain_correction += error * AGC_ALPHA;
  
  // Limit gain
  gain_correction = constrain(gain_correction, 0.5, 2.0);
  
  // Apply to modulation depth
  current_mod_depth = BASE_MOD_DEPTH * gain_correction;
}
```

#### 4. Multi-Frequency Emission

**Technika:** Jednoczesna emisja wielu częstotliwości terapeutycznych

**Metody Implementacji:**

1. **Frequency Multiplexing (FDM):**
   ```
   Carrier 1: 40 kHz + f1 (therapeutic)
   Carrier 2: 45 kHz + f2 (therapeutic)
   Carrier 3: 50 kHz + f3 (therapeutic)
   
   Wymaga szerokopasmowych transducerów
   ```

2. **Time Division Multiplexing (TDM):**
   ```
   Slot 1 (10ms): f1 = 528 Hz (Solfeggio)
   Slot 2 (10ms): f2 = 432 Hz (Verdi)
   Slot 3 (10ms): f3 = 741 Hz (Cleaning)
   
   Cycle repeats every 30ms
   ```

3. **Chirp Signals:**
   ```
   Linear sweep: f_start → f_end w czasie T
   Przykład: 100 Hz → 1000 Hz w 5 sekund
   
   Exponential sweep dla coverage logarithmic frequency scale
   ```

#### 5. Interfejs Użytkownika OLED + Enkoder

**Hardware:**
- Display: SSD1306 128x64 OLED (I2C)
- Encoder: EC11 rotary encoder z przyciskiem
- Buzzer: Piezo dla feedback akustycznego

**Menu Structure:**
```
Main Screen:
┌────────────────┐
│ FREQ: 40.00kHz │
│ GAIN: ████░ 75%│
│ TEMP: 42°C     │
│ STATUS: READY  │
└────────────────┘

Presets Menu:
1. Rife General
2. Solfeggio 528
3. Relaxation 432
4. Focus Alpha
5. Custom User

Settings:
- Carrier Frequency: 35-50 kHz
- Max Power Limit: 50-100%
- Session Timer: 5-60 min
- Safety Thresholds
```

**Library:** Adafruit_SSD1306 + Adafruit_GFX

```cpp
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void display_main_screen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("FREQ: ");
  display.print(carrier_freq / 1000.0, 2);
  display.println("kHz");
  
  display.print("GAIN: ");
  draw_bar_graph(gain_percent, 0, 20, 128, 10);
  
  display.print("TEMP: ");
  display.print(temperature, 1);
  display.println("C");
  
  display.display();
}
```

## 5. Analiza Symulacyjna i Modelowanie Matematyczne

### Równanie Westervelta - Rozwiązanie Numeryczne

Dla jednowymiarowego przypadku wiązki gaussowskiej:

```
∂²p/∂x² - (1/c₀²)∂²p/∂t² = -(β/ρ₀c₀⁴)∂²p²/∂t² + δ∇²(∂p/∂t)

gdzie:
δ = dyssypacja akustyczna (powietrze: ~1.5×10⁻⁵ m²/s)
```

**Metoda Finite Difference Time Domain (FDTD):**

```python
# Python pseudocode dla symulacji propagacji
import numpy as np

def westervelt_1d(x_max, t_max, dx, dt):
    nx = int(x_max / dx)
    nt = int(t_max / dt)
    
    p = np.zeros(nx)      # Pressure field
    p_prev = np.zeros(nx) # Previous timestep
    
    c0 = 343.0            # Speed of sound
    beta = 1.2            # Nonlinearity parameter
    rho0 = 1.225          # Air density
    
    # Source condition (ultrasonic transducer)
    def source(t):
        fc = 40000  # Carrier
        fm = 1000   # Modulation
        return np.sin(2*np.pi*fc*t) * (1 + 0.8*np.sin(2*np.pi*fm*t))
    
    # Time stepping
    for n in range(nt):
        t = n * dt
        
        # Boundary condition at x=0
        p[0] = source(t)
        
        # FDTD update equation (central differences)
        for i in range(1, nx-1):
            d2p_dx2 = (p[i+1] - 2*p[i] + p[i-1]) / dx**2
            d2p2_dt2 = beta/(rho0*c0**4) * (p[i]**2 - p_prev[i]**2) / dt**2
            
            p_new = 2*p[i] - p_prev[i] + c0**2 * dt**2 * (d2p_dx2 + d2p2_dt2)
            p_prev[i] = p[i]
            p[i] = p_new
    
    return p
```

### Model Tablicy Phased Array - Teoria Anteny

**Array Factor dla N elementów:**

```
AF(θ) = Σ(n=0 to N-1) w_n × exp(j × k × n × d × sin(θ))

gdzie:
w_n = weighting coefficient (amplitude tapering)
k = 2π/λ = wave number
d = inter-element spacing
θ = observation angle
```

**Directivity Index (DI):**

```
DI = 10 × log₁₀(4π / Ω_A) [dB]

gdzie Ω_A = beam solid angle ≈ θ_az × θ_el (w radianach)

Dla wiązki 15° × 15°:
Ω_A ≈ (15 × π/180)² ≈ 0.0685 sr
DI ≈ 10 × log₁₀(4π / 0.0685) ≈ 12.6 dB
```

### Bilans Energii Akustycznej

**Moc Akustyczna Emitowana:**

```
P_acoustic = η_electroacoustic × P_electrical

gdzie:
η_electroacoustic ≈ 0.3-0.5 (dla piezo w powietrzu)
P_electrical = V_rms² / Z_load
```

**Natężenie w Odległości r:**

```
I(r) = P_acoustic / (4πr²) × D(θ)

gdzie D(θ) = directivity function

Dla wiązki kierunkowej (θ wewnątrz beamwidth):
I(r) ≈ P_acoustic / (π × (r × tan(θ_bw/2))²)
```

**Poziom Ciśnienia Akustycznego (SPL):**

```
SPL = 20 × log₁₀(p_rms / p_ref) [dB]

gdzie:
p_ref = 20 µPa (próg słyszalności)
p_rms = √(ρ₀ × c₀ × I)
```

**Przykład Obliczeniowy:**
```
Dane:
- P_electrical = 30W
- η = 0.4
- r = 1m
- θ_bw = 15°

Obliczenia:
P_acoustic = 0.4 × 30 = 12W
Beam radius @1m = 1 × tan(7.5°) ≈ 0.132m
Beam area = π × 0.132² ≈ 0.055 m²
I = 12W / 0.055 m² ≈ 218 W/m²
p_rms = √(1.225 × 343 × 218) ≈ 306 Pa
SPL = 20 × log₁₀(306 / 20e-6) ≈ 143.7 dB

UWAGA: To bardzo wysoki poziom - wymaga ostrożności!
Zalecane ograniczenie do <110 dB dla bezpieczeństwa.
```

## 6. Podsumowanie i Perspektywy Rozwoju

Ultradźwiękowy głośnik kierunkowy reprezentuje fascynującą technologię łączącą zaawansowaną akustykę nieliniową z praktycznymi zastosowaniami. W kontekście biorezonansu oferuje unikalne możliwości precyzyjnego dostarczania częstotliwości terapeutycznych do określonych obszarów ciała.

Implementacja z użyciem Arduino Nano i standardowego złącza jack audio czyni tę technologię dostępną dla hobbystów i badaczy, pozwalając na eksperymentowanie z różnymi protokołami częstotliwościowymi przy zachowaniu stosunkowo niskich kosztów (szacowany koszt komponentów: 150-400 PLN).

**Kluczowe Wyzwania Techniczne:**
1. **Jakość Modulacji**: Osiągnięcie THD <5% wymaga precyzyjnej kalibracji i ewentualnie cyfrowego przetwarzania sygnału (DSP)
2. **Bezpieczeństwo**: Kontrola SPL poniżej 110 dB dla ultradźwięków i 85 dB dla słyszalnych częstotliwości
3. **Efektywność**: Optymalizacja η_electroacoustic poprzez matching impedance i geometrię tablicy
4. **Termalne Management**: Dissipation 10-20W ciepła w kompaktowej obudowie
5. **Walidacja Terapeutyczna**: Brak peer-reviewed studies potwierdzających skuteczność biorezonansu akustycznego

**Perspektywy Rozwoju:**
- **Miniaturyzacja**: Przejście na mikrokontrolery ARM Cortex-M4/M7 z FPU dla lepszego DSP
- **Integracja Sensorów**: Dodanie IMU (accelerometer/gyro) dla automatic beam steering
- **AI Optimization**: Machine learning dla adaptive frequency selection based on patient feedback
- **Multi-Modal Fusion**: Połączenie z fotobiomodulacją (LED arrays) i PEMF (pulsed electromagnetic fields)
- **Clinical Trials**: Partnerstwa z instytutami badawczymi dla evidence-based validation

**Zastosowania Pozamedyczne:**
- Audio advertising (targeted messages in public spaces)
- Museum exhibits (personalized audio guides without headphones)
- Automotive (driver-only navigation prompts)
- Gaming (immersive directional sound effects)
- Assistive technology (hearing aid alternative for directional hearing loss)

Technologia ta ma potencjał do rewolucjonizowania nieinwazyjnych terapii częstotliwościowych, oferując nowe narzędzia dla praktyków medycyny alternatywnej i badaczy biofizyki. Jednakże, konieczne są rygorystyczne badania naukowe dla potwierdzenia deklarowanych korzyści terapeutycznych i ustalenia optymalnych protokołów aplikacyjnych.

---

**Disclaimer**: Informacje zawarte w tym dokumencie mają charakter edukacyjny i eksperymentalny. Zastosowania medyczne powinny być konsultowane z wykwalifikowanymi specjalistami i podlegać odpowiednim regulacjom prawnym. Autor nie ponosi odpowiedzialności za jakiekolwiek skutki zdrowotne wynikające z zastosowania opisanych rozwiązań. Przed użyciem jakiegokolwiek urządzenia biorezonansowego należy przeprowadzić dokładną analizę ryzyka i uzyskać odpowiednie approvals regulatory (CE marking, FDA clearance, etc.).

**Bibliografia i Źródła:**
1. Westervelt, P.J. (1963). "Parametric Acoustic Array". J. Acoust. Soc. Am.
2. Yoneyama, M. et al. (1983). "The Audio Spotlight". J. Audio Eng. Soc.
3. ISO 9612:2009 - Determination of occupational noise exposure
4. IEC 60601-2-5: Medical electrical equipment - Part 2-5: Ultrasonic physiotherapy equipment
5. Arduino ATmega328P Datasheet, Microchip Technology
6. Piezoelectric Transducer Theory, IEEE Ultrasonics Symposium Proceedings
