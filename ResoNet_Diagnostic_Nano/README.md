# ResoNet Diagnostic Nano — obraz diagnostyczny z symulacją efektorów

Odrębny firmware dla Arduino Nano ATmega328P + ENC28J60. To obraz diagnostyczno-programistyczny, nie pełny firmware urządzenia i nie może go zastępować.

## Zawartość
- ENC28J60 i pojedynczy serwer TCP/5001; ResoNet v1: HELLO_REQ, PING, STATUS_REQ
- parser strumieniowy o stałym buforze 11 B z CRC-16/CCITT-FALSE
- wspólny model kanałów PWM dla IR LED, piezo i wibratora
- statyczna kolejka 8 zdarzeń, bez dynamicznej alokacji
- walidacja częstotliwości, wypełnienia i czasu trwania profilu
- obsługa CONFIGURE_PWM, START i STOP wyłącznie w modelu RAM
- brak logowania i detekcji urządzeń

## Granica sprzętowa — ważne
Warstwa efektorów jest wyłącznie modelem programowym. Nie zawiera numerów pinów, wywołań pinMode/analogWrite/digitalWrite, konfiguracji timerów ani dostępu do rejestrów. Zdarzenia aktualizują wyłącznie stan w RAM. Nie ma komend sieciowych do uruchamiania efektorów ani fizycznego wyjścia PWM. Pozwala to testować kolejkę i walidację profili bez energizowania IR LED, piezo lub wibratora.

Nie dodawaj bezpośredniego sterowania GPIO do tego obrazu. Sterowanie fizyczne wymaga osobnej, niezależnej architektury zabezpieczeń sprzętowych i walidacji. Ten wariant nie jest przeznaczony do terapii ani do podłączania obciążeń wykonawczych.

## Kolejka i model PWM
Plik effector_runtime.h udostępnia EffectorRuntime, trzy identyfikatory efektorów oraz zdarzenia EVENT_CONFIGURE_PWM, EVENT_START, EVENT_STOP. enqueue() odrzuca nieprawidłowe parametry i sygnalizuje przepełnienie; processOne() wykonuje maksymalnie jedno przejście stanu RAM. Profil wymaga częstotliwości większej od zera, wypełnienia 0–100% i czasu trwania większego od zera.

## Konfiguracja i budowanie
Przed kompilacją sprawdź statyczne IP, MAC, bramę i maskę w network_config.h. Wymagane: Arduino CLI, core arduino:avr, EthernetENC 2.0.5.

```bash
arduino-cli core install arduino:avr
arduino-cli lib install EthernetENC@2.0.5
bash ResoNet_Diagnostic_Nano/build.sh
```

Opcjonalny profil linkera:
```bash
RESONET_BUILD_DIR=/tmp/resonet-build RESONET_MAP_FILE=/tmp/resonet-diagnostic.map bash ResoNet_Diagnostic_Nano/build.sh
```

## Testy hosta
```bash
g++ -std=c++11 -Wall -Wextra -Werror -pedantic tests/host/diagnostic_protocol_test.cpp -o diagnostic_protocol_test && ./diagnostic_protocol_test
g++ -std=c++11 -Wall -Wextra -Werror -pedantic tests/host/effector_runtime_test.cpp -o effector_runtime_test && ./effector_runtime_test
```

Testy hosta nie są testami sprzętowymi. Protokół v1 nie zapewnia uwierzytelniania ani szyfrowania; używaj wyłącznie w zaufanej, odizolowanej sieci.
