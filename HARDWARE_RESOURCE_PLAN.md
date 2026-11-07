# Plan zasobów sprzętowych Nano i architektury efektorów

Status: propozycja projektowa do zatwierdzenia po inwentaryzacji modułów  
Data: 2026-11-07  
Baza: `main`

## Założenia

Nie ma wiarygodnego schematu ani potwierdzonej listy konkretnych płytek/modułów. Dlatego ten dokument rozdziela:
- zasoby, których funkcja wynika z samego ATmega328P;
- zasoby już zajęte według kodu;
- docelowe interfejsy logiczne;
- elementy wymagające potwierdzenia sprzętowego.

Nie jest to jeszcze schemat elektryczny ani zatwierdzona tabela połączeń.

## Proponowana polityka zasobów

| Zasób Nano | Polityka docelowa | Uwagi |
|---|---|---|
| D0/RX, D1/TX | UART/USB-Serial | Zachować do diagnostyki i programowania. |
| D2/INT0 | Wejście przerwania ogólnego / sygnał gotowości modułu | Tylko po uzgodnieniu priorytetów; nie współdzielić z niezależnymi wyjściami. |
| D3/INT1/OC2B | Fizyczny E-Stop albo dedykowany sygnał bezpieczeństwa | Nie współdzielić z IR ani detekcją. E-Stop powinien działać sprzętowo, niezależnie od MCU. |
| D4–D8 | Ograniczona liczba GPIO sterujących / wejściowych | Funkcje przydzielić dopiero po inwentaryzacji modułów; obecne aliasy są sprzeczne. |
| D9/OC1A, D10/OC1B/SS | Nie przydzielać równocześnie funkcji CS i PWM bez świadomej decyzji | Timer1 ma jednego właściciela. D10 jest obecnie CS ENC28J60. |
| D11–D13 | Magistrala SPI | MOSI/MISO/SCK zarezerwowane dla SPI. Nie używać D11 jako OC2A ani D13 jako diody statusowej w czasie transmisji. |
| A0–A3 | Wejścia analogowe pomiarowe | Podział na pomiary dopiero po określeniu torów analogowych i ich kondycjonowania. |
| A4/A5 | I²C SDA/SCL | Nie przydzielać jako zwykłych wejść pomiarowych przy aktywnym I²C. |
| A6/A7 | Dodatkowe wejścia analogowe (Nano z ATmega328P SMD) | Brak funkcji cyfrowej; wykorzystanie zależy od konkretnej wersji płytki. |

## Konsekwencja dla generatorów

ATmega328P nie ma wystarczającej liczby niezależnych wyjść sprzętowego PWM, aby bez kolizji obsłużyć jednocześnie dotychczasowy PWM, IR, piezo i nowe generatory kanałów elektrodowych przy zachowaniu SPI, I²C, UART i wejścia awaryjnego.

**Rekomendacja:** Nano ma zarządzać i nadzorować zewnętrzne układy generujące. Nie należy próbować budować wszystkich przebiegów na timerach MCU.

- IR: zewnętrzny, dedykowany generator/sterownik LED z kontrolą częstotliwości i wypełnienia, jeśli wymagane parametry nie mieszczą się w wolnych zasobach MCU.
- Kanał elektrody 1 i kanał elektrody 2: dwa logicznie niezależne kanały sterowania z oddzielnymi stanami, konfiguracją, pomiarem, diagnostyką i blokadą. Każdy kanał ma interfejs do osobnego stopnia generatora; nie oznacza to bezpośredniego podłączenia elektrody do GPIO.
- Antena szerokopasmowa: odrębny moduł RF/generator z pomiarem sprzężenia/parametrów, sterowany przez Nano. Nie używać zwykłego PWM jako generatora szerokopasmowego.
- Piezo i wibrator: sterowniki zewnętrzne albo ekspander/układ PWM; nie mogą samodzielnie nadpisywać rejestrów timerów należących do innych modułów.

## Model logiczny dwóch kanałów elektrodowych

Każdy kanał powinien przechowywać co najmniej:
- identyfikator kanału;
- stan: DISABLED, STANDBY, CONFIGURED, RUNNING, FAULT;
- stan blokady sprzętowej i programowej;
- identyfikator/wersję zewnętrznego generatora;
- parametry żądane i parametry odczytane ze sprzętu, rozdzielone;
- status komunikacji;
- status czujników napięcia i prądu (jeśli tor pomiarowy je zapewnia);
- status ciągłości/obciążenia, jeśli układ pomiarowy potrafi to wiarygodnie wykryć;
- kod błędu, znacznik czasu i licznik błędów;
- informację, czy pomiar jest aktualny i wiarygodny.

Stan RUNNING nie może być ustawiany wyłącznie na podstawie odebrania polecenia START. Wymaga potwierdzenia sprzętowego i spełnienia warunków blokady. Po resecie, błędzie komunikacji, watchdogu lub utracie zasilania stan musi przejść do DISABLED/FAULT, a sprzętowy tor wyjściowy pozostać zablokowany.

## Interfejs sterownika zewnętrznego

Wspólny interfejs programowy powinien oddzielać logikę od konkretnego układu:
- `begin()` — wykrycie i inicjalizacja;
- `readIdentity()` — identyfikacja układu i rewizji;
- `configure()` — zapis konfiguracji po walidacji;
- `readBack()` — odczyt rzeczywistej konfiguracji;
- `enable()` / `disable()` — żądanie pracy / bezpieczne wyłączenie;
- `readMeasurements()` — odczyty wraz ze znacznikami ważności;
- `readFaults()` — diagnostyka;
- `clearFault()` — tylko dla błędów, które można bezpiecznie skasować;
- `heartbeat()` — nadzór komunikacji.

Sterownik ma mieć implementację symulowaną do testów hosta. Implementacja sprzętowa pozostaje nieaktywna do czasu wybrania konkretnego układu i zatwierdzenia jego dokumentacji.

## Bezpieczeństwo toru elektrodowego

To urządzenie ma być rozwijane jako sprzęt mogący mieć kontakt z człowiekiem. Z tego powodu:
- nie wpisujemy do firmware arbitralnych napięć, prądów, częstotliwości, energii ani kształtów przebiegu;
- nie uruchamiamy wyjścia na człowieku;
- testy integracyjne wykonujemy na obciążeniu zastępczym i odpowiednim stanowisku pomiarowym;
- ograniczenie energii/prądu i odłączenie muszą być realizowane sprzętowo, niezależnie od kodu Nano;
- izolacja, prądy upływu, pojedyncze uszkodzenia, EMC i ocena ryzyka wymagają osobnej walidacji;
- nie deklarujemy zgodności z IEC 60601-1 ani normami szczegółowymi bez pełnego procesu oceny.

## Brakujące dane, które blokują finalną mapę pinów

Nie trzeba mieć gotowego schematu. Wystarczy zebrać:
1. dokładny model/zdjęcie każdej płytki i modułu (ENC28J60, czujniki, sterowniki, generatory);
2. informację, które efektory mają fizycznie pozostać;
3. liczbę kanałów pomiarowych i sygnałów statusowych każdego modułu;
4. napięcia zasilania i poziomy logiczne modułów;
5. dokumentację lub oznaczenia układów scalonych na modułach.

Dopiero na tej podstawie można przypisać konkretne CS, adresy I²C, linie enable/ready/fault i wejścia pomiarowe.

## Etapy implementacji po audycie

1. Uporządkować centralny rejestr zasobów i usunąć duplikaty pinów w nagłówkach.
2. Wydzielić właścicieli timerów; zakazać bezpośredniej rekonfiguracji timerów w detektorze efektorów.
3. Dodać host-testowalny model dwóch kanałów elektrodowych bez aktywnego wyjścia.
4. Dodać abstrakcję zewnętrznego generatora i sterownik symulowany.
5. Dodać model pomiarów/diagnostyki z flagami ważności, błędami i timeoutami.
6. Dodać abstrakcję pojedynczego modułu anteny szerokopasmowej.
7. Rozszerzyć protokół o wersjonowane komendy i statusy, zachowując domyślną blokadę wyjść.
8. Zintegrować TUI, WebUI, Desktop GUI i Android.
9. Uruchomić kompilacje, testy jednostkowe, testy protokołu i testy sprzętowe na obciążeniach zastępczych.

## Kryterium zakończenia

Nie uznajemy modułu za gotowy tylko dlatego, że się kompiluje. Każdy moduł wymaga testów granicznych, błędów komunikacji, resetu, utraty zasilania, wartości nieprawidłowych i przejścia do stanu bezpiecznego. Testy sprzętowe muszą mieć raport z rzeczywistych pomiarów i wskazanie użytego obciążenia zastępczego.
