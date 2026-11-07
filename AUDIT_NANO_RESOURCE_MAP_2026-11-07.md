# Audyt zasobów Arduino Nano — main

Data audytu: 2026-11-07  
Audytowana gałąź: `main`  
Commit bazowy: `be1d66247a1e6ae8d4b7086f1f0c23b521754463`  
Płytka: Arduino Nano, ATmega328P, założony zegar 16 MHz.

## Wynik

**Wynik: nie można bezpiecznie wdrożyć wyjść wielokanałowych na obecnej mapie pinów.** Mapa zawiera kolizje pinów, timerów i magistral. Nie jest to wyłącznie problem programowy — wymaga zatwierdzenia nowej mapy połączeń i weryfikacji schematu.

## 1. Timery

| Zasób | Obecne użycie | Konflikt / uwaga |
|---|---|---|
| Timer0 | Arduino `millis()`, `micros()`, `delay()`; PWM D5/D6 | Nie wolno zmieniać preskalera ani trybu w celu uzyskania nośnej bez utraty poprawności zegara systemowego. |
| Timer1 | `pwm_engine.cpp`, Fast PWM, OC1A/D9 | Zajęty przez główny generator PWM. Obliczenia TOP nie uwzględniają konsekwentnie składnika +1, a ograniczanie TOP powoduje, że deklarowany zakres 0,1 Hz–500 kHz nie jest osiągany. |
| Timer2 | `ir_led_engine.cpp`, OC2B/D3 | Wartość TOP obliczona dla 38 kHz wynosi ok. 420, po czym jest obcinana do 255. Rzeczywista częstotliwość przy preskalerze 1 wynosi wówczas ok. 62,5 kHz, nie 38 kHz. |
| Timer2 | `device_detector.cpp::piezo_set_tone()` | Funkcja bezpośrednio nadpisuje TCCR2A/TCCR2B i OCR2A, kolidując z IR PWM. Ustawia OC2A, którego wyjście sprzętowe znajduje się na D11 (SPI MOSI). |

Dodatkowo `ir_led_init()` nie jest wywoływane w `setup()` głównego szkicu.

## 2. Kolizje pinów

| Pin | Przypisania znalezione w kodzie | Problem |
|---|---|---|
| D3 | `PIN_EMERGENCY_STOP`, `IR_LED_PIN` (OC2B), `PIN_IR_DETECT` | Trzy funkcje na jednym pinie; krytyczna kolizja. |
| D4 | `PIN_OTIC_ENABLE`, `PIN_VIBRATOR_ENABLE` | Dwa niezależne sterowania. |
| D5 | `PIN_PPG_INT`, `PIN_PIEZO_PWM` | Wejście czujnika i wyjście PWM. Dodatkowo sprzętowy PWM należy do Timer0. |
| D6 | `PIN_ELECTRODE_ENABLE`, `PIN_GSR_DRDY`, `PIN_PIEZO_FREQ`, `IR_MODULATION_PIN` | Cztery funkcje, w tym wejście czujnika i sterowanie. |
| D7 | `PIN_BIO_DETECT`, `PIN_PIEZO_ENABLE` | Wejście detekcji i wyjście sterujące. |
| D8 | `PIN_WRAP_ENABLE`, `PIN_VIBRATOR_PWM` | Sterowanie i PWM jednocześnie. |
| D9 | `PIN_PWM_OUTPUT` (OC1A), testy detekcji efektorów przez `analogWrite()` | Detektor może zmieniać konfigurację aktywnego generatora Timer1. |
| D10 | `PIN_ENC28J60_CS`, `PPG_SPI_CS` | Dwa urządzenia SPI współdzielą ten sam CS; selekcja PPG może aktywować ENC28J60 i odwrotnie. |
| D11–D13 | SPI MOSI/MISO/SCK; D11 jest też OC2A | D13 jest dodatkowo `STATUS_LED`; miganie LED zakłóca SCK. |

## 3. Kolizje magistral

- SPI: ENC28J60 i PPG mają przypisany D10 jako CS. Współdzielenie SPI wymaga osobnych linii CS i poprawnej inicjalizacji obu urządzeń.
- I2C: Nano używa A4/A5 jako SDA/SCL. Kod jednocześnie deklaruje A4 jako `PIN_WRAP_TEMP` i A5 jako `PIN_WRAP_CURRENT`. To wyklucza niezależne użycie tych pinów jako wejść analogowych w tej konfiguracji.
- UART: D0/D1 są potrzebne do USB-Serial; nie powinny być używane jako zwykłe wyjścia bez jawnej decyzji o rezygnacji z diagnostyki szeregowej.

## 4. Błędy generatora IR

1. Timer2 Fast PWM mode 7 ma 8-bitowy rejestr TOP (`OCR2A`), więc obliczonej wartości ok. 420 nie da się zapisać.
2. Przy obecnym obcięciu TOP do 255 i preskalerze 1 częstotliwość wynosi ok. 62,5 kHz.
3. Pin D3 jest równocześnie wejściem E-Stop i detekcją IR.
4. `ir_led_init()` nie jest wywoływane z głównego `setup()`.
5. Sterownik piezo bezpośrednio nadpisuje Timer2.
6. Schemat `tscircuit_schematic.tsx` opisuje IR na D5, podczas gdy nagłówek i kod wskazują D3; D5 ma też inne przypisania.

## 5. Wnioski architektoniczne

- Obecna mapa Nano nie zapewnia wolnych, jednoznacznych zasobów dla dwóch kanałów elektrod, pojedynczej anteny i IR, przy zachowaniu ENC28J60, czujników i obecnych efektorów.
- Dwa kanały elektrodowe nie mogą być implementowane przez proste dodanie kolejnych `digitalWrite()` lub `analogWrite()`. Wymagają osobnych interfejsów do zewnętrznych układów generatora i pomiaru, odrębnych od pinów SPI/UART/I2C.
- Wysokoczęstotliwościowy generator MHz powinien być zewnętrznym układem; ATmega328P pełni funkcję sterownika i nadzorcy.
- Każdy tor przeznaczony do kontaktu z pacjentem wymaga określonych limitów elektrycznych, niezależnego sprzętowego ograniczania i walidacji. Sam oscyloskop nie potwierdza bezpieczeństwa prądu, energii ani ekspozycji RF.
- Nie należy deklarować zgodności z IEC 60601-1 na podstawie obecnego kodu. Komentarz w `safety_system.h` jest deklaracją, nie dowodem zgodności.

## 6. Zalecana kolejność napraw

1. Zatwierdzić nową mapę pinów i magistral, w tym rozdzielić CS ENC28J60 i PPG, zwolnić D3 z konfliktu E-Stop/IR/detekcja oraz rozdzielić funkcje D4–D8.
2. Wycofać bezpośrednią konfigurację Timer2 z detektora piezo; zasoby timerów mają mieć jednego właściciela.
3. Naprawić generator IR i dodać test obliczeń częstotliwości; dopiero po zmianie mapy pinów przypisać fizyczne wyjście.
4. Wprowadzić model dwóch kanałów elektrodowych i niezależne interfejsy zewnętrznych generatorów oraz pomiarów.
5. Dodać sterownik jednej anteny szerokopasmowej jako osobnego modułu z pomiarem parametrów RF.
6. Rozszerzyć protokół dopiero po ustaleniu modelu poleceń, statusów, błędów i wersjonowania.
7. Uruchomić CI, testy hosta i testy stanowiskowe na obciążeniach zastępczych. Testów fizycznych nie można uznać za wykonane bez podłączonego sprzętu i raportu pomiarowego.

## 7. Granice tego audytu

Audyt jest analizą kodu i deklarowanej mapy połączeń w repozytorium. Nie potwierdza rzeczywistego okablowania płytki, jakości izolacji, parametrów analogowych ani zachowania urządzenia na stanowisku. Przed wdrożeniem należy uzgodnić rzeczywisty schemat i listę modułów sprzętowych.
