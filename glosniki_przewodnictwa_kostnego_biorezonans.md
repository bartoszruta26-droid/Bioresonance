# Głośniki Przewodnictwa Kostnego w Biorezonansie z Arduino Nano

## 1. Charakterystyka Głośników Przewodnictwa Kostnego

### 1.1. Zasada Działania

Głośniki przewodnictwa kostnego (bone conduction transducers) to przetworniki elektromechaniczne, które zamieniają sygnał elektryczny na wibracje mechaniczne przenoszone bezpośrednio przez kości czaszki do ślimaka ucha wewnętrznego, z pominięciem błony bębenkowej.

**Kluczowe cechy:**
- **Pełnozakresowe**: Typowo pracują w zakresie 20 Hz - 20 kHz (niektóre modele do 40 kHz)
- **Impedancja**: Zazwyczaj 4Ω lub 8Ω
- **Moc nominalna**: 3W - 5W typowo dla małych przetworników
- **Mechanizm**: Cewka elektromagnetyczna poruszająca membraną/styk mechaniczny

### 1.2. Różnice Względem Tradycyjnych Głośników

| Parametr | Głośnik Tradycyjny | Przewodnictwo Kostne |
|----------|-------------------|---------------------|
| Środowisko | Powietrze | Tkanka kostna |
| Efektywność | Wysoka w powietrzu | Niska, wymaga bezpośredniego kontaktu |
| Zakres częstotliwości | 20 Hz - 20 kHz | 100 Hz - 15 kHz (optymalnie) |
| Impedancja akustyczna | ~400 Rayl | ~1.5 MRayl (kość) |
| Wymagany kontakt | Bezpośredni nie wymagany | **Wymagany mocny docisk** |

### 1.3. Zastosowanie w Biorezonansie

Głośniki przewodnictwa kostnego oferują unikalne możliwości terapeutyczne:

1. **Bezpośrednia stymulacja układu nerwowego**: Wibracje przenoszone przez kość potyliczną i skroniową mogą bezpośrednio oddziaływać na struktury mózgu
2. **Ominięcie bariery słuchowej**: Dla pacjentów z uszkodzeniem narządu słuchu
3. **Fale alfa/theta/delta/beta**: Generowanie częstotliwości脑owych (brainwave entrainment)
4. **Częstotliwości Solfeggio**: 174Hz, 285Hz, 396Hz, 417Hz, 528Hz, 639Hz, 741Hz, 852Hz, 963Hz
5. **Terapia dźwiękiem Rife'a**: Częstotliwości specyficzne dla patogenów

---

## 2. Specyfikacja Techniczna Arduino Nano

### 2.1. Parametry Mikrokontrolera ATmega328P

```
Napięcie zasilania:        5V (USB) lub 7-12V (VIN pin)
Taktowanie:                16 MHz
Pamięć Flash:              32 KB (2 KB bootloader)
SRAM:                      2 KB
EEPROM:                    1 KB
Liczba pinów cyfrowych:    14 (6 PWM)
Liczba wejść analogowych:  8 (10-bit ADC)
Prąd na pin I/O:           20 mA max
Prąd całkowity:            200 mA max
```

### 2.2. Wyjścia PWM Dostępne dla Audio

Arduino Nano posiada 6 wyjść PWM:
- **Pin 3**: Timer2 - OC2B
- **Pin 5**: Timer0 - OC0B
- **Pin 6**: Timer0 - OC0A
- **Pin 9**: Timer1 - OC1A ⭐ **(rekomendowane)**
- **Pin 10**: Timer1 - OC1B ⭐ **(rekomendowane)**
- **Pin 11**: Timer2 - OC2A

**Dla zastosowań audio najlepiej użyć Timer1 (piny 9 lub 10)** ze względu na:
- 16-bitowa rozdzielczość
- Możliwość generowania wysokiej częstotliwości PWM
- Niezależność od funkcji systemowych (millis(), micros())

---

## 3. Schemat Połączeń Elektrycznych

### 3.1. Wymagane Komponenty

| Element | Specyfikacja | Ilość | Uwagi |
|---------|-------------|-------|-------|
| Arduino Nano | ATmega328P 5V/16MHz | 1 | Wersja oryginalna lub klon |
| Głośnik kostny | 4Ω/8Ω, 3-5W | 1-2 | Pełnozakresowy |
| Wzmacniacz audio | PAM8403 / MAX98357A | 1 | Klasy D, 3-5W |
| Potencjometr | 10kΩ liniowy | 1 | Regulacja głośności |
| Kondensator | 100µF/10V elektrolityczny | 2 | Filtracja zasilania |
| Rezystor | 10kΩ 1/4W | 2 | Pull-down / dzielnik |
| Przełącznik | SPST mini | 1 | Włącznik zasilania |
| Złącze Jack 3.5mm | TRS stereo | 1 | Wejście audio opcjonalne |
| Przewody | AWG 22-24 | - | Połączenia |
| Płytka stykowa | Breadboard | 1 | Prototypowanie |

### 3.2. Schemat Ideowy

```
┌─────────────────────────────────────────────────────────────────────┐
│                        ARDUINO NANO                                 │
│                                                                     │
│   ┌──────┐                                                          │
│   │ USB  │ ← 5V Zasilanie                                           │
│   └──────┘                                                          │
│                                                                     │
│   Pin 9 (PWM) ──────→ [R 10kΩ] ──────→ [IN+] wzmacniacza           │
│                              │                                      │
│   GND ───────────────────────┴──────→ [IN-] wzmacniacza            │
│                                                                     │
│   Pin A0 ──────→ [Potencjometr 10kΩ środkowy]                       │
│                  Lewy: +5V                                          │
│                  Prawy: GND                                         │
│                                                                     │
│   Pin 2 ──────→ [Przełącznik bezpieczeństwa] → GND                 │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │  WZMACNIACZ      │
                    │  PAM8403 3W      │
                    │                  │
                    │  IN+  IN-        │
                    │    \  /          │
                    │     \/           │
                    │     /\           │
                    │    /  \          │
                    │  OUT+ OUT-       │
                    └─────┬────┬───────┘
                          │    │
              ┌───────────┘    └───────────┐
              │                            │
         ┌────▼────┐                  ┌────▼────┐
         │GŁOŚNIK  │                  │GŁOŚNIK  │
         │KOSTNY L │                  │KOSTNY R │
         │  4Ω 3W  │                  │  4Ω 3W  │
         └─────────┘                  └─────────┘
              │                            │
              └────────────┬───────────────┘
                           │
                        [DOCISK]
                     Do kości skroniowej
```

### 3.3. Szczegółowy Schemat Połączeń

#### Podłączenie Wzmacniacza PAM8403

```
PAM8403 Pinout:
┌─────────────────────────────────┐
│  VIN+   VIN-   GND   IN+   IN-  │  ← Strona wejścia
│                                 │
│  OUTL+  OUTL-  NC   OUTR+ OUTR- │  ← Strona wyjścia
└─────────────────────────────────┘

Połączenia:
VIN+  → Arduino 5V (przez kondensator 100µF)
VIN-  → Arduino GND
GND   → Arduino GND
IN+   → Arduino Pin 9 (PWM) przez rezystor 1kΩ
IN-   → Arduino GND
OUTL+ → Głośnik kostny (+)
OUTL- → Głośnik kostny (-)
```

#### Opcjonalne Wejście Audio Jack 3.5mm

```
Złącze Jack TRS:
Tip (L)  → [C 10µF] → [R 10kΩ] → A0 Arduino
Ring (R) → [C 10µF] → [R 10kΩ] → A1 Arduino (opcjonalnie)
Sleeve   → GND Arduino

Warunkowanie sygnału:
- Dodaj dzielnik napięcia dla bias DC (2.5V)
- Filtr dolnoprzepustowy RC: R=1kΩ, C=10nF (fc≈16kHz)
```

---

## 4. Implementacja Programowa

### 4.1. Podstawowy Kod Generatora Częstotliwości

```cpp
/*
 * Generator Częstotliwości Biorezonansowych
 * dla Głośników Przewodnictwa Kostnego
 * Arduino Nano
 * 
 * Autor: System Biorezonansowy
 * Wersja: 1.0
 */

#include <avr/io.h>
#include <avr/interrupt.h>

// ============================================================================
// KONFIGURACJA
// ============================================================================

#define PWM_PIN         9           // Pin PWM (Timer1 OC1A)
#define POT_PIN         A0          // Potencjometr regulacji amplitudy
#define SAFETY_PIN      2           // Przycisk bezpieczeństwa (active LOW)
#define LED_PIN         13          // Dioda statusu

#define PWM_FREQ        31250UL     // Częstotliwość PWM (31.25 kHz - niesłyszalne)
#define SAMPLE_RATE     44100UL     // Częstotliwość próbkowania waveform

// Częstotliwości terapeutyczne (Hz)
const float frequencies[] = {
  174.0,   // Solfeggio 1 - redukcja bólu
  285.0,   // Solfeggio 2 - regeneracja tkanek
  396.0,   // Solfeggio 3 - uwolnienie od lęku
  417.0,   // Solfeggio 4 - ułatwienie zmian
  528.0,   // Solfeggio 5 - naprawa DNA
  639.0,   // Solfeggio 6 - harmonia relacji
  741.0,   // Solfeggio 7 - oczyszczenie
  852.0,   // Solfeggio 8 - intuicja
  963.0,   // Solfeggio 9 - połączenie z wyższym ja
  432.0,   // Strojenie naturalne
  440.0,   // Strojenie standardowe
  1000.0,  // Testowy ton
  7.83,    // Rezonans Schumanna
  10.0,    // Fala Alfa
  6.0,     // Fala Theta
  2.0,     // Fala Delta
  40.0     // Fala Gamma
};

const int freq_count = sizeof(frequencies) / sizeof(frequencies[0]);

// ============================================================================
// ZMIENNE GLOBALNE
// ============================================================================

volatile uint32_t phase_accumulator = 0;
volatile uint16_t amplitude = 512;      // 0-1023 (regulowane potencjometrem)
volatile bool safety_active = true;
volatile uint8_t current_freq_index = 0;
volatile uint32_t freq_change_timer = 0;

// Lookup table sinusoidy (256 punktów, 8-bit resolution)
const uint8_t sine_table[256] PROGMEM = {
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173,
  176, 179, 182, 185, 188, 191, 194, 197, 199, 202, 205, 208, 211, 213, 216, 219,
  222, 224, 227, 230, 232, 235, 237, 240, 242, 245, 247, 249, 252, 254, 256, 258,
  260, 262, 264, 266, 268, 270, 272, 274, 276, 278, 280, 281, 283, 285, 286, 288,
  289, 291, 292, 293, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306,
  307, 307, 308, 309, 309, 310, 310, 311, 311, 311, 312, 312, 312, 312, 312, 312,
  312, 312, 312, 312, 312, 312, 311, 311, 311, 310, 310, 309, 309, 308, 307, 307,
  306, 305, 304, 303, 302, 301, 300, 299, 298, 297, 296, 295, 293, 292, 291, 289,
  288, 286, 285, 283, 281, 280, 278, 276, 274, 272, 270, 268, 266, 264, 262, 260,
  258, 256, 254, 252, 249, 247, 245, 242, 240, 237, 235, 232, 230, 227, 224, 222,
  219, 216, 213, 211, 208, 205, 202, 199, 197, 194, 191, 188, 185, 182, 179, 176,
  173, 170, 167, 164, 161, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128,
  125, 122, 119, 116, 113, 110, 107, 104, 101,  98,  95,  92,  89,  86,  83,  80,
   77,  74,  71,  68,  65,  62,  59,  56,  54,  51,  48,  45,  42,  40,  37,  34,
   31,  29,  26,  23,  21,  18,  16,  13,  11,   8,   6,   4,   1,  -1,  -3,  -5,
   -7,  -9, -11, -13, -15, -17, -19, -21, -22, -24, -26, -27, -29, -30, -32, -33,
  -35, -36, -37, -39, -40, -41, -42, -43, -44, -45, -46, -47, -48, -49, -50, -51,
  -52, -52, -53, -54, -55, -56, -56, -57, -57, -58, -58, -58, -59, -59, -59, -59,
  -59, -59, -59, -59, -59, -59, -58, -58, -58, -57, -57, -56, -56, -55, -54, -53,
  -52, -52, -51, -50, -49, -48, -47, -46, -45, -44, -43, -42, -41, -40, -39, -37,
  -36, -35, -33, -32, -30, -29, -27, -26, -24, -22, -21, -19, -17, -15, -13, -11,
   -9,  -7,  -5,  -3,  -1,   1,   4,   6,   8,  11,  13,  16,  18,  21,  23,  26,
   29,  31,  34,  37,  40,  42,  45,  48,  51,  54,  56,  59,  62,  65,  68,  71,
   74,  77,  80,  83,  86,  89,  92,  95,  98, 101, 104, 107, 110, 113, 116, 119,
  122, 125
};

// ============================================================================
// KONFIGURACJA TIMERÓW
// ============================================================================

void setupTimer1() {
  /*
   * Timer1: Generator PWM 31.25 kHz dla sygnału audio
   * Tryb: Phase Correct PWM, 8-bit (TOP=255)
   * f_PWM = 16MHz / (2 × 1 × 256) = 31.25 kHz
   */
  
  TCCR1A = 0;
  TCCR1B = 0;
  
  // Tryb 1: Phase Correct PWM, 8-bit
  TCCR1A = _BV(COM1A1) | _BV(WGM10);  // Clear OC1A on compare match
  TCCR1B = _BV(CS10);                  // No prescaling
  
  OCR1A = 128;  // Duty cycle 50% (wartość początkowa)
  
  pinMode(PWM_PIN, OUTPUT);
}

void setupTimer2() {
  /*
   * Timer2: Interrupt próbkowania waveform (44.1 kHz)
   * Tryb: CTC
   */
  
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0;
  
  TCCR2A = _BV(WGM21);  // CTC mode
  TCCR2B = _BV(CS21);   // Prescaler = 8
  
  OCR2A = 44;  // f = 16MHz / (8 × 45) ≈ 44.44 kHz
  
  TIMSK2 = _BV(OCIE2A);  // Enable interrupt
}

// ============================================================================
// OBSŁUGA INTERRUPTÓW
// ============================================================================

ISR(TIMER2_COMPA_vect) {
  // Obliczenie indeksu w tabeli sinusoidy
  uint8_t sample_index = phase_accumulator >> 24;
  
  // Pobranie wartości z tabeli (odwrócenie faz dla pełnego mostka)
  uint8_t sample = pgm_read_byte(&sine_table[sample_index]);
  
  // Aplikacja amplitudy (0-1023 → 0-255)
  uint16_t modulated = ((uint16_t)sample * amplitude) >> 10;
  
  // Aktualizacja PWM
  OCR1A = modulated;
  
  // Inkrementacja fazy (DDS - Direct Digital Synthesis)
  float current_freq = frequencies[current_freq_index];
  uint32_t phase_increment = (current_freq * (1UL << 32)) / SAMPLE_RATE;
  phase_accumulator += phase_increment;
}

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

void read_potentiometer() {
  // Odczyt potencjometru dla regulacji amplitudy
  static uint16_t last_reading = 0;
  uint16_t reading = analogRead(POT_PIN);
  
  // Prosty filtr wygładzający
  reading = (last_reading * 3 + reading) / 4;
  last_reading = reading;
  
  // Mapowanie na zakres amplitudy (minimalna 10%, maksymalna 100%)
  amplitude = map(reading, 0, 1023, 102, 1023);
}

void check_safety() {
  // Sprawdzenie przycisku bezpieczeństwa
  if (digitalRead(SAFETY_PIN) == LOW) {
    safety_active = false;
    amplitude = 0;
    digitalWrite(LED_PIN, LOW);
  } else {
    safety_active = true;
    digitalWrite(LED_PIN, HIGH);
  }
}

void auto_frequency_sequence() {
  // Automatyczna sekwencja częstotliwości
  static uint32_t last_change = 0;
  uint32_t current_time = millis();
  
  // Zmiana co 60 sekund
  if (current_time - last_change > 60000UL) {
    last_change = current_time;
    current_freq_index = (current_freq_index + 1) % freq_count;
    
    // Reset akumulatora fazy dla płynnej zmiany
    phase_accumulator = 0;
    
    // Sygnalizacja zmiany częstotliwości
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
  }
}

// ============================================================================
// SETUP I LOOP
// ============================================================================

void setup() {
  // Konfiguracja pinów
  pinMode(LED_PIN, OUTPUT);
  pinMode(SAFETY_PIN, INPUT_PULLUP);
  
  // Inicjalizacja Serial dla debugowania
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port
  }
  
  Serial.println(F("=== Biorezonans - Głośniki Kostne ==="));
  Serial.println(F("Inicjalizacja systemu..."));
  
  // Konfiguracja timerów
  setupTimer1();
  setupTimer2();
  
  // Włączenie przerwań globalnych
  sei();
  
  // Opóźnienie startowe dla stabilizacji
  delay(1000);
  
  Serial.print(F("Wybrana częstotliwość: "));
  Serial.print(frequencies[current_freq_index]);
  Serial.println(F(" Hz"));
  Serial.println(F("System gotowy. Użyj potencjometru do regulacji."));
}

void loop() {
  // Cykliczne zadania
  read_potentiometer();
  check_safety();
  auto_frequency_sequence();
  
  // Komunikacja szeregowa (co 5 sekund)
  static uint32_t last_status = 0;
  if (millis() - last_status > 5000UL) {
    last_status = millis();
    
    Serial.print(F("Freq: "));
    Serial.print(frequencies[current_freq_index]);
    Serial.print(F(" Hz | Amp: "));
    Serial.print(amplitude);
    Serial.print(F(" | Safety: "));
    Serial.println(safety_active ? F("OK") : F("OFF"));
  }
  
  // Krótkie opóźnienie dla stabilności
  delay(10);
}
```

### 4.2. Zaawansowany Kod z Modulacją AM i Sweep

```cpp
/*
 * Zaawansowany Generator Biorezonansowy
 * - Modulacja AM dla efektów beatów różnicowych
 * - Frequency sweep (przemiatanie)
 * - Predefiniowane protokoły terapeutyczne
 * 
 * Wymaga: Arduino Nano + PAM8403 + Głośniki kostne
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// ============================================================================
// DEFINICJE I KONFIGURACJA
// ============================================================================

#define PWM_PIN       9
#define MOD_PIN       10          // Drugi PWM dla modulacji
#define POT_PIN       A0
#define SW_PIN        A1          // Przełącznik trybów
#define SAFE_PIN      2

#define CARRIER_FREQ  40000UL     // Nośna ultradźwiękowa (opcjonalnie)
#define AUDIO_FREQ    31250UL     // PWM audio

// Tryby pracy
enum Mode {
  MODE_SINGLE,      // Pojedyncza częstotliwość
  MODE_SWEEP,       // Przemiatanie zakresu
  MODE_BEAT,        // Beaty różnicowe
  MODE_PROTOCOL     // Sekwencja terapeutyczna
};

volatile Mode current_mode = MODE_SINGLE;

// Parametry sweep
struct SweepParams {
  float start_freq;
  float end_freq;
  uint16_t duration_sec;
  bool logarithmic;
};

volatile SweepParams sweep = {
  100.0,    // Start 100 Hz
  1000.0,   // Koniec 1000 Hz
  300,      // 5 minut
  true      // Logarytmiczne przemiatanie
};

// Beaty różnicowe
struct BeatParams {
  float base_freq;
  float beat_freq;
  float depth;  // 0.0 - 1.0
};

volatile BeatParams beat = {
  200.0,   // Częstotliwość bazowa
  10.0,    // Częstość beatu (10 Hz = fala Alfa)
  0.5      // Głębokość modulacji 50%
};

// ============================================================================
// ZMIENNE SYSTEMOWE
// ============================================================================

volatile uint32_t phase_main = 0;
volatile uint32_t phase_mod = 0;
volatile uint16_t output_level = 512;
volatile bool enabled = true;

// DDS constants
#define PHASE_BITS 32
#define SAMPLE_RATE 44100UL

// ============================================================================
// TABELA SINUSOIDY (256 punktów)
// ============================================================================

const uint8_t sine_lut[256] PROGMEM = {
  128,131,134,137,140,143,146,149,152,155,158,161,164,167,170,173,
  176,179,182,185,188,191,194,197,199,202,205,208,211,213,216,219,
  222,224,227,230,232,235,237,240,242,245,247,249,252,254,256,258,
  260,262,264,266,268,270,272,274,276,278,280,281,283,285,286,288,
  289,291,292,293,295,296,297,298,299,300,301,302,303,304,305,306,
  307,307,308,309,309,310,310,311,311,311,312,312,312,312,312,312,
  312,312,312,312,312,312,311,311,311,310,310,309,309,308,307,307,
  306,305,304,303,302,301,300,299,298,297,296,295,293,292,291,289,
  288,286,285,283,281,280,278,276,274,272,270,268,266,264,262,260,
  258,256,254,252,249,247,245,242,240,237,235,232,230,227,224,222,
  219,216,213,211,208,205,202,199,197,194,191,188,185,182,179,176,
  173,170,167,164,161,158,155,152,149,146,143,140,137,134,131,128,
  125,122,119,116,113,110,107,104,101,98,95,92,89,86,83,80,
  77,74,71,68,65,62,59,56,54,51,48,45,42,40,37,34,
  31,29,26,23,21,18,16,13,11,8,6,4,1,-1,-3,-5,
  -7,-9,-11,-13,-15,-17,-19,-21,-22,-24,-26,-27,-29,-30,-32,-33,
  -35,-36,-37,-39,-40,-41,-42,-43,-44,-45,-46,-47,-48,-49,-50,-51,
  -52,-52,-53,-54,-55,-56,-56,-57,-57,-58,-58,-58,-59,-59,-59,-59,
  -59,-59,-59,-59,-59,-59,-58,-58,-58,-57,-57,-56,-56,-55,-54,-53,
  -52,-52,-51,-50,-49,-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,-37,
  -36,-35,-33,-32,-30,-29,-27,-26,-24,-22,-21,-19,-17,-15,-13,-11,
  -9,-7,-5,-3,-1,1,4,6,8,11,13,16,18,21,23,26,
  29,31,34,37,40,42,45,48,51,54,56,59,62,65,68,71,
  74,77,80,83,86,89,92,95,98,101,104,107,110,113,116,119,
  122,125
};

// ============================================================================
// INICJALIZACJA SPRZĘTOWA
// ============================================================================

void init_timers() {
  // Timer1: PWM główny 31.25 kHz
  TCCR1A = _BV(COM1A1) | _BV(WGM10);
  TCCR1B = _BV(CS10);
  OCR1A = 128;
  
  // Timer2: Interrupt próbkowania
  TCCR2A = _BV(WGM21);
  TCCR2B = _BV(CS21);
  OCR2A = 44;
  TIMSK2 = _BV(OCIE2A);
  
  pinMode(PWM_PIN, OUTPUT);
  pinMode(MOD_PIN, OUTPUT);
}

// ============================================================================
// ISR - GŁÓWNY GENERATOR
// ============================================================================

ISR(TIMER2_COMPA_vect) {
  if (!enabled) {
    OCR1A = 128;
    return;
  }
  
  uint8_t sample;
  
  switch (current_mode) {
    case MODE_SINGLE:
    case MODE_SWEEP: {
      // Prosta synteza DDS
      uint8_t idx = phase_main >> 24;
      sample = pgm_read_byte(&sine_lut[idx]);
      
      // Obliczenie sweep
      if (current_mode == MODE_SWEEP) {
        static uint32_t sweep_start = 0;
        if (sweep_start == 0) sweep_start = millis();
        
        uint32_t elapsed = millis() - sweep_start;
        float progress = (float)elapsed / (sweep.duration_sec * 1000UL);
        
        if (progress >= 1.0) {
          progress = 0.0;
          sweep_start = millis();
        }
        
        float current_freq;
        if (sweep.logarithmic) {
          current_freq = sweep.start_freq * pow(sweep.end_freq / sweep.start_freq, progress);
        } else {
          current_freq = sweep.start_freq + (sweep.end_freq - sweep.start_freq) * progress;
        }
        
        uint32_t inc = (current_freq * (1UL << PHASE_BITS)) / SAMPLE_RATE;
        phase_main += inc;
      } else {
        // Stała częstotliwość
        phase_main += 2097152UL; // Przykład dla ~200 Hz
      }
      break;
    }
    
    case MODE_BEAT: {
      // Modulacja AM dla beatów
      uint8_t idx = phase_main >> 24;
      uint8_t carrier = pgm_read_byte(&sine_lut[idx]);
      
      uint8_t mod_idx = phase_mod >> 24;
      uint8_t modulator = pgm_read_byte(&sine_lut[mod_idx]);
      
      // AM: y = carrier * (1 + depth * modulator)
      int16_t mod_normalized = (int16_t)modulator - 128;
      int16_t modulation = 128 + (mod_normalized * (int16_t)(beat.depth * 128)) / 128;
      
      sample = (carrier * modulation) / 256;
      
      // Aktualizacja faz
      phase_main += (beat.base_freq * (1UL << PHASE_BITS)) / SAMPLE_RATE;
      phase_mod += (beat.beat_freq * (1UL << PHASE_BITS)) / SAMPLE_RATE;
      break;
    }
    
    case MODE_PROTOCOL:
      // Implementacja protokołu terapeutycznego
      sample = 128; // Placeholder
      break;
  }
  
  // Aplikacja poziomu wyjściowego
  uint16_t out = ((uint16_t)sample * output_level) >> 8;
  OCR1A = constrain(out, 0, 255);
}

// ============================================================================
// SETUP I LOOP
// ============================================================================

void setup() {
  Serial.begin(9600);
  init_timers();
  sei();
  
  Serial.println(F("Biorezonans Advanced - Gotowy"));
}

void loop() {
  // Odczyt potencjometru
  output_level = map(analogRead(POT_PIN), 0, 1023, 0, 1023);
  
  // Przełączanie trybów
  static int last_sw = 0;
  int sw = analogRead(SW_PIN);
  
  if (abs(sw - last_sw) > 100) {
    last_sw = sw;
    if (sw < 200) current_mode = MODE_SINGLE;
    else if (sw < 400) current_mode = MODE_SWEEP;
    else if (sw < 600) current_mode = MODE_BEAT;
    else current_mode = MODE_PROTOCOL;
  }
  
  delay(50);
}
```

---

## 5. Protokoły Terapeutyczne

### 5.1. Tabela Częstotliwości Biorezonansowych

| Zakres | Częstotliwość | Zastosowanie | Czas Sesji |
|--------|--------------|--------------|------------|
| **Schumann** | 7.83 Hz | Grounding, synchronizacja | 15-30 min |
| **Delta** | 0.5-4 Hz | Sen głęboki, regeneracja | 20-40 min |
| **Theta** | 4-8 Hz | Medytacja, kreatywność | 15-30 min |
| **Alfa** | 8-14 Hz | Relaks, nauka | 10-20 min |
| **Beta** | 14-30 Hz | Koncentracja, czujność | 5-15 min |
| **Gamma** | 30-100 Hz | Wysokie przetwarzanie | 5-10 min |
| **Solfeggio 1** | 174 Hz | Redukcja bólu | 10-15 min |
| **Solfeggio 2** | 285 Hz | Regeneracja tkanek | 15-20 min |
| **Solfeggio 3** | 396 Hz | Uwolnienie od lęku | 10-15 min |
| **Solfeggio 4** | 417 Hz | Ułatwienie zmian | 10-15 min |
| **Solfeggio 5** | 528 Hz | Naprawa DNA, transformacja | 15-20 min |
| **Solfeggio 6** | 639 Hz | Harmonia relacji | 10-15 min |
| **Solfeggio 7** | 741 Hz | Oczyszczenie, ekspresja | 10-15 min |
| **Solfeggio 8** | 852 Hz | Intuicja, duchowość | 10-15 min |
| **Solfeggio 9** | 963 Hz | Połączenie z wyższym ja | 15-20 min |

### 5.2. Przykładowy Protokół Sesji

**Protokół "Pełna Regeneracja" (60 minut):**

```
Czas    | Częstotliwość | Cel
--------|---------------|----------------------------------
0-5 min | 7.83 Hz       | Grounding, wprowadzenie
5-15 min| 528 Hz        | Naprawa komórkowa
15-25 min| 639 Hz       | Harmonia emocjonalna
25-35 min| 285 Hz       | Regeneracja tkanek
35-45 min| 174 Hz       | Redukcja bólu/napięcia
45-55 min| 963 Hz       | Integracja duchowa
55-60 min| 7.83 Hz       | Zakończenie, powrót
```

---

## 6. Bezpieczeństwo i Przeciwwskazania

### 6.1. Środki Ostrożności

⚠️ **WAŻNE:** Przed użyciem urządzenia skonsultuj się z lekarzem!

1. **Maksymalny poziom natężenia**
   - Nie przekraczać 85 dB dla długich sesji
   - Dla głośników kostnych: maksymalnie 70% mocy wzmacniacza

2. **Czas ekspozycji**
   - Sesje nie powinny przekraczać 60 minut
   - Przerwy minimum 2 godziny między sesjami

3. **Docisk głośników**
   - Umiarkowany, komfortowy docisk
   - Nie stosować nadmiernego nacisku

### 6.2. Przeciwwskazania

❌ **Nie stosować w przypadkach:**
- Ciąża (wszystkie trymestry)
- Rozrusznik serca lub inne implanty elektroniczne
- Aktywne nowotwory
- Ostre stany zapalne głowy/szyi
- Epilepsja fotosensytywna (przy synchronizacji ze światłem)
- Świeże rany lub urazy czaszki
- Dzieci poniżej 12 roku życia (bez nadzoru specjalisty)

### 6.3. Monitorowanie Temperatury

```cpp
// Dodaj termistor NTC 10k na pin A2
#define TEMP_PIN A2

float read_temperature() {
  int adc = analogRead(TEMP_PIN);
  float resistance = 10000.0 * (adc / (1023.0 - adc));
  float steinhart = log(resistance / 10000.0);
  steinhart /= 3950.0;
  steinhart += 1.0 / 298.15;
  steinhart = 1.0 / steinhart;
  return steinhart - 273.15; // °C
}

// W pętli głównej:
if (read_temperature() > 45.0) {
  enabled = false;  // Wyłącz przy przegrzaniu
}
```

---

## 7. Kalibracja i Testowanie

### 7.1. Procedura Kalibracji

1. **Test połączeń**
   ```
   - Sprawdź multimetrem ciągłość połączeń
   - Upewnij się o braku zwarć 5V-GND
   - Zweryfikuj polaryzację głośników
   ```

2. **Test niskiego poziomu**
   ```
   - Ustaw potencjometr na minimum
   - Włącz zasilanie
   - Stopniowo zwiększaj poziom
   - Sprawdź czy słyszysz delikatne wibracje
   ```

3. **Pomiar impedancji**
   ```
   - Zmierz impedancję głośnika multimetrem
   - Powinno być ~4Ω lub ~8Ω
   - Wartość DC będzie niższa (~3Ω lub ~6Ω)
   ```

### 7.2. Narzędzia Diagnostyczne

**Oscyloskop (opcjonalnie):**
- Podłącz sondę do pinu 9 Arduino
- Sprawdź kształt fali PWM
- Zweryfikuj częstotliwość nośną

**Multimetr z pomiarem częstotliwości:**
- Sprawdź dokładność generowanych częstotliwości
- Porównaj z wartościami referencyjnymi

---

## 8. Rozszerzenia i Modyfikacje

### 8.1. Dodanie Wyświetlacza OLED

```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void display_info(float freq, int level) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Freq: ");
  display.print(freq);
  display.println(" Hz");
  display.print("Level: ");
  display.println(level);
  display.display();
}
```

### 8.2. Sterowanie Bluetooth

```cpp
#include <SoftwareSerial.h>

SoftwareSerial BT(10, 11); // RX, TX

void setupBluetooth() {
  BT.begin(9600);
  BT.println("Gotowy do sterowania");
}

void handleBluetooth() {
  if (BT.available()) {
    char cmd = BT.read();
    switch(cmd) {
      case 'U': frequency += 10; break;
      case 'D': frequency -= 10; break;
      case '+': amplitude += 50; break;
      case '-': amplitude -= 50; break;
    }
  }
}
```

### 8.3. Karta SD do Zapisu Protokołów

```cpp
#include <SPI.h>
#include <SD.h>

File protocol_file;

void load_protocol(String filename) {
  protocol_file = SD.open(filename);
  if (protocol_file) {
    // Wczytaj parametry sesji
    protocol_file.close();
  }
}
```

---

## 9. Lista Zakupów i Koszty

### 9.1. Podstawowy Zestaw

| Element | Szacowany koszt (PLN) | Link/Gdzie kupić |
|---------|----------------------|------------------|
| Arduino Nano (klon) | 15-25 zł | AliExpress, lokalne sklepy |
| PAM8403 wzmacniacz | 5-10 zł | AliExpress, eBay |
| Głośnik kostny (para) | 40-80 zł | AliExpress (link z pytania) |
| Potencjometr 10kΩ | 2-5 zł | Sklep elektroniczny |
| Breadboard + przewody | 15-25 zł | Zestaw startowy |
| Obudowa | 20-40 zł | Druk 3D lub kupna |
| **RAZEM** | **~100-185 zł** | |

### 9.2. Zestaw Rozszerzony

Dodatkowo:
- Wyświetlacz OLED 0.96": 15-25 zł
- Moduł Bluetooth HC-05: 15-25 zł
- Czytnik kart SD: 10-15 zł
- Termistor NTC + obwód: 5 zł
- Przełączniki, diody: 10 zł
- **Dodatkowo: ~60-80 zł**

---

## 10. FAQ - Najczęstsze Pytania

### Q: Czy to urządzenie medyczne?
**A:** NIE. To jest eksperymentalne urządzenie DIY. Nie zastępuje profesjonalnej terapii medycznej.

### Q: Jak mocno dociskać głośniki?
**A:** Umiarkowanie. Powinny być w kontakcie z kością, ale bez powodowania dyskomfortu.

### Q: Dlaczego nie słyszę dźwięku?
**A:** Głośniki kostne przenoszą wibracje, nie dźwięk powietrzny. Powinieneś czuć wibracje na kościach czaszki.

### Q: Czy mogę użyć obu głośników jednocześnie?
**A:** Tak, podłącz je równolegle (uwaga na impedancję) lub użyj wzmacniacza stereo.

### Q: Jakie są najlepsze częstotliwości na początek?
**A:** Zacznij od 528 Hz (naprawa) lub 432 Hz (relaks) przez 10-15 minut.

### Q: Czy mogę zasilić z baterii?
**A:** Tak, bateria Li-Ion 3.7V + moduł boost do 5V lub bateria 9V z regulatorem.

---

## 11. Źródła i Literatura

### Publikacje Naukowe:
1. Stenfelt, S., & Goode, R. L. (2005). "Bone-conducted sound: Physiological and clinical aspects." *Otology & Neurotology*
2. Eeg-Olofsson, M. (2010). "Bone Conduction: Current and Future Perspectives."
3. Dobrev, I., et al. (2018). "Laser Doppler Vibrometry of Bone Conduction."

### Dokumentacja Techniczna:
- ATmega328P Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega328P_Datasheet.pdf
- PAM8403 Datasheet: https://www.diodes.com/assets/Datasheets/PAM8403.pdf

### Społeczności:
- Arduino Forum: https://forum.arduino.cc/
- Reddit r/biohackers
- Grupy Facebook: "DIY Biohacking", "Arduino Projects"

---

## 12. Disclaimer

⚠️ **OSTRZEŻENIE PRAWNE I MEDYCZNE:**

Ten dokument ma charakter wyłącznie informacyjny i edukacyjny. Opisane urządzenia i metody:

- **NIE SĄ** zatwierdzonymi urządzeniami medycznymi
- **NIE MOGĄ** zastępować profesjonalnej diagnozy i leczenia
- Są przeznaczone **TYLKO** do celów badawczych i hobbystycznych
- Użycie odbywa się **NA WŁASNE RYZYKO**

Autorzy i dystrybutorzy tego dokumentu **NIE PONOSZĄ ODPOWIEDZIALNOŚCI** za jakiekolwiek szkody zdrowotne, materialne lub prawne wynikające z zastosowania opisanych rozwiązań.

Przed rozpoczęciem jakiejkolwiek terapii skonsultuj się z kwalifikowanym specjalistą medycznym.

---

*Dokument utworzony: 2024*
*Wersja: 1.0*
*Licencja: CC BY-NC-SA 4.0*
