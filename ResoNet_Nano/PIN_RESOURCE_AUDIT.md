# Audyt zasobów Arduino Nano — mapa konfliktów

Status: audyt statyczny kodu, bez potwierdzenia schematem i pomiarami na płytce.  
Płytka odniesienia: klasyczne Arduino Nano / ATmega328P. Nie zakładać, że klony i warianty mają identyczne wyprowadzenia.

## Zasady

- Nie zmieniać numeracji pinów bez schematu połączeń lub potwierdzenia użytkownika.
- Detektor nie może używać wyjścia generatora jako źródła impulsu testowego.
- Timer ma jednego właściciela. Moduł pomocniczy nie może nadpisywać rejestrów timera innego generatora.
- Współdzielenie pinu przez dwa aktywne tory jest konfliktem, a nie możliwością równoległej pracy.
- Sam odczyt ADC podczas pracy generatora nie jest automatycznie wiarygodnym pomiarem impedancji ani prądu.

## Zidentyfikowane przypisania i konflikty

| Pin | Przypisania znalezione w kodzie | Ocena |
|---|---|---|
| D2 | `PIN_HELMHOLTZ_DETECT` | Wejście detekcji cewki |
| D3 | `IR_LED_PIN` / Timer2 OC2B oraz `PIN_IR_DETECT` | Konflikt wyjścia nośnej IR z wejściem detekcji. Detektor nie może konfigurować D3 jako wejścia. |
| D4 | `PIN_OTIC_ENABLE`, `PIN_VIBRATOR_ENABLE` | Dwa sygnały sterujące współdzielą pin. |
| D5 | `PIN_PPG_INT`, `PIN_PIEZO_PWM` | Konflikt wejścia PPG i sterowania piezo. |
| D6 | `PIN_ELECTRODE_ENABLE`, `IR_MODULATION_PIN`, `PIN_GSR_DRDY`, `PIN_PIEZO_FREQ` | Wielokrotny konflikt krytyczny; nie przełączać z detektora. |
| D7 | `PIN_BIO_DETECT`, `PIN_PIEZO_ENABLE` | Konflikt wejścia detekcji i sterowania piezo. |
| D8 | `PIN_WRAP_ENABLE`, `PIN_VIBRATOR_PWM`, `DEBUG_LED` | Konflikt sterowania efektorami i diody diagnostycznej. |
| D9 | `PIN_PWM_OUTPUT` / Timer1 OC1A | Zarezerwować wyłącznie dla głównego generatora PWM. |
| D10 | `PPG_SPI_CS` / `PIN_ENC28J60_CS` | Krytyczny konflikt: oba urządzenia mają ten sam CS. Detektor nie może wybierać PPG, gdyż wybierze jednocześnie ENC28J60. Odczyt PPG wyłączony do czasu wydzielenia osobnego CS. |
| A0 | `PIN_NTC_TEMP` | Wejście temperatury; nie przedstawiać jako GSR. |
| A1 | `PIN_HELMHOLTZ_SENSE` | Odczyt analogowy. |
| A2 | `PIN_OTIC_IMPEDANCE` | Odczyt analogowy. |
| A3 | `PIN_ELECTRODE_IMPEDANCE` | Odczyt analogowy. |
| A4 | `PIN_WRAP_TEMP` / SDA | Potencjalny konflikt z I²C na klasycznym Nano. |
| A5 | `PIN_WRAP_CURRENT` / SCL | Potencjalny konflikt z I²C na klasycznym Nano. |
| A6 | `PIN_IR_CURRENT_SENSE` | Wejście analogowe w klasycznym Nano; brak funkcji cyfrowego GPIO. |
| A7 | `PIN_AUDIO_DETECT` | Wejście analogowe w klasycznym Nano; brak funkcji cyfrowego GPIO. |

## Właściciele timerów

- Timer0: Arduino `millis()` / `micros()` / `delay()`; nie rekonfigurować.
- Timer1: główny generator PWM na D9 / OC1A.
- Timer2: nośna IR na D3 / OC2B; `OCR2A` jest używany jako TOP. Kod piezo, który nadpisywał `TCCR2A`, `TCCR2B` i `OCR2A`, niszczyłby konfigurację IR.

## Stan po ostatnich poprawkach

- Detekcja Helmholtza, OTIC, elektrod i Wrap jest pasywna — nie wstrzykuje impulsów przez D9 i nie przełącza linii ENABLE w ramach testu.
- Inicjalizacja detektora nie konfiguruje już współdzielonych D4/D6/D8 jako wyjść ani nie wymusza na nich stanu LOW. Linie te pozostają własnością sterowników po ustaleniu docelowej mapy sprzętowej.\n- Odczyt identyfikatora PPG przez SPI jest wyłączony, ponieważ `PPG_SPI_CS` i `PIN_ENC28J60_CS` wskazują D10. Wspólne CS wybierałoby równocześnie oba układy i mogłoby zakłócić Ethernet.
- Detekcja IR nie przełącza D3 ani D6. Odczyt A6 jest dostępny tylko, jeśli istnieje rzeczywisty tor pomiaru prądu.
- Ocena jakości PPG zwraca „niedostępna” (0), dopóki nie powstanie sterownik dostarczający próbki z czasami. Usunięto blokujące oczekiwanie 3 sekund.
- Detekcja wibratora nie zgłasza już fikcyjnego połączenia na podstawie stałej wartości 500.
- Piezo nie przejmuje Timer2, dopóki nie otrzyma niezależnego zasobu timera lub zewnętrznego sterownika.

## Dane wymagane przed remapowaniem

1. Schemat połączeń albo zdjęcia obu stron płytki i modułów.
2. Dokładny wariant Nano i modułu Ethernet.
3. Lista równocześnie wymaganych funkcji i efektorów.
4. Oznaczenia sterowników IR, piezo, wibratora, elektrod i torów pomiarowych.
5. Informacja, które wejścia pomiarowe są elektrycznie niezależne od torów wyjściowych.

Do czasu uzyskania tych danych konfliktów nie należy „naprawiać” przez arbitralne przenoszenie pinów. Nie deklarować zgodności ani bezpieczeństwa sprzętowego na podstawie tego audytu.
