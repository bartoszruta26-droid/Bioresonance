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


## Zmiana założeń funkcjonalnych — decyzja użytkownika (2026-11-07)

Poniższa decyzja zastępuje wcześniejsze założenie, że Nano ma przede wszystkim sterować zewnętrznymi generatorami:

**Arduino Nano jest na tym etapie generatorem sygnałów i urządzeniem pomiarowym, a nie nadrzędnym sterownikiem efektorów.**

Zakres odpowiedzialności Nano:
- generowanie przebiegów w granicach rzeczywistych możliwości timerów i wyjść;
- wykonywanie pomiarów analogowych i cyfrowych;
- akwizycja próbek, podstawowa filtracja i diagnostyka jakości pomiaru;
- przekazywanie konfiguracji, próbek i wyników do TUI, WebUI, Desktop GUI i Android;
- bezpośrednie sterowanie wyjściami generatora tylko w zakresie zatwierdzonym dla danego toru.

Nie implementować obecnie nadrzędnego modelu sterowania zewnętrznymi efektorami ani rozbudowanego protokołu poleceń START/STOP dla nich. Zachować jednak blokady, diagnostykę oraz możliwość bezpiecznego wyłączenia generowanych sygnałów.

### GSR / EDA — wymaganie pomiarowe

Dodać niezależny podsystem pomiaru aktywności elektrodermalnej (EDA/GSR), z dwoma odrębnymi trybami:

1. **GSR-DC** — pomiar przewodności/rezystancji skóry przy kontrolowanym, ograniczonym sygnale stałym. Wyniki raportować jako surowe próbki, napięcie, prąd (jeśli mierzony), rezystancję i przewodność wraz z flagami jakości.
2. **GSR-AC** — pomiar odpowiedzi na kontrolowany sygnał przemienny. Częstotliwość ma być parametrem konfiguracji eksperymentu, ale dopuszczalny zakres zostanie ustalony dopiero po określeniu analogowego toru pomiarowego, szybkości próbkowania i filtracji.

Wyniki GSR-AC:
- częstotliwość pobudzenia;
- amplituda napięcia pobudzenia i odpowiedzi;
- amplituda prądu, jeśli tor pomiarowy zapewnia wiarygodny pomiar;
- impedancja pozorna lub admitancja wyłącznie wtedy, gdy znane są warunki kalibracji;
- faza tylko wtedy, gdy układ akwizycji umożliwia synchroniczny pomiar napięcia i prądu;
- surowe próbki, znaczniki czasu, zakres ADC, status kalibracji i jakość pomiaru.

Nie utożsamiać samego pomiaru amplitudy z pełnym pomiarem impedancji zespolonej. Dla rozdzielenia składowej rezystancyjnej i pojemnościowej potrzebny jest odpowiedni analogowy tor pomiarowy i detekcja synchroniczna / pomiar fazy.

### Ograniczenia ATmega328P

Dla klasycznego Nano z ATmega328P przyjąć 10-bitowy ADC i ograniczoną szybkość akwizycji. Standardowe `analogRead()` jest zbyt wolne do precyzyjnej, szerokopasmowej analizy impedancji. Można rozpocząć od wolnego GSR-DC i eksperymentalnego, wąskopasmowego GSR-AC, ale projekt powinien przewidywać zewnętrzny przetwornik ADC lub dedykowany analogowy układ pomiarowy, jeżeli wymagane będą lepsza rozdzielczość, stabilność, synchronizacja lub szersze pasmo.

### Podział modułów firmware

- `signal_generator` — abstrakcja generowania przebiegów, bez deklarowania nierealnych zakresów;
- `gsr_dc` — pomiar DC i obliczanie GSR;
- `gsr_ac` — sekwencja pomiarowa AC, synchronizacja i ekstrakcja parametrów;
- `measurement_engine` — akwizycja, znaczniki czasu, filtracja i buforowanie próbek;
- `calibration` — kalibracja toru i identyfikacja nieaktualnych współczynników;
- `diagnostics` — nasycenie ADC, odłączenie czujnika, przekroczenie zakresu, szum, timeout;
- `protocol` — konfiguracja pomiaru, strumień próbek i wyniki dla interfejsów użytkownika.

### Bezpieczeństwo pomiaru

Ponieważ elektrody mogą mieć kontakt ze skórą człowieka, GSR musi być osobnym, ograniczonym energetycznie torem pomiarowym. Nie podłączać elektrod bezpośrednio do GPIO ani nie używać wyjścia generatora ogólnego jako źródła pobudzenia skóry. Wymagane są: ograniczenie prądu sprzętowe, zabezpieczenie przed pojedynczym uszkodzeniem, kontrola napięcia i prądu, odpowiednia izolacja oraz walidacja toru. Do czasu walidacji pomiary wykonywać na obciążeniu zastępczym. Parametrów pobudzenia nie ustalać arbitralnie w firmware.

### Kolejność dalszych prac

1. Utrzymać audyt zasobów i poprawić generator IR bez aktywowania konfliktowego pinu.
2. Oddzielić generowanie sygnałów od sterowania efektorami.
3. Zaprojektować i przetestować model danych GSR-DC oraz GSR-AC na danych syntetycznych.
4. Zidentyfikować istniejący moduł GSR i jego interfejs; nie zakładać, że samo wykrywanie GSR oznacza gotowy tor pomiarowy.
5. Dodać akwizycję i diagnostykę, a następnie kalibrację.
6. Rozszerzyć protokół o konfigurację pomiaru i ramki próbek.
7. Zintegrować wyniki z TUI, WebUI, Desktop GUI i Android.
8. Testy sprzętowe prowadzić najpierw na obciążeniach zastępczych; osobno raportować testy hostowe, kompilację i testy fizyczne.


## Doprecyzowanie wymogu wyjść fizycznych — decyzja użytkownika (2026-11-07)

Mikrokontrolery mają **wytwarzać rzeczywiste sygnały na fizycznych wyprowadzeniach**, a nie wyłącznie symulować generację lub przekazywać polecenia do komputera. Komputer jest interfejsem konfiguracji i odbioru danych; źródłem przebiegu pozostaje mikrokontroler.

Rozróżnić trzy poziomy:
1. **Generacja cyfrowa na pinie MCU** — rzeczywisty przebieg logiczny/PWM na wyjściu mikrokontrolera.
2. **Tor analogowy / stopień mocy** — zewnętrzny układ przekształcający sygnał logiczny w wymagany przebieg elektryczny, prądowy lub napięciowy na obciążeniu.
3. **Pomiar** — odczyt rzeczywistej odpowiedzi obciążenia/czujnika przez wejście analogowe lub przetwornik pomiarowy.

Nano ma wykonywać poziom 1 oraz akwizycję poziomu 3. Poziom 2 zależy od konkretnego toru i jego zabezpieczeń. Nie traktować wyjścia GPIO/PWM jako gotowego wyjścia elektrodowego ani wyjścia zdolnego bezpośrednio zasilać obciążenie.

### Aktualny stan wyjść w kodzie

- Generator PWM na D9/OC1A konfiguruje sprzętowy Timer1 i może wystawiać rzeczywisty przebieg logiczny. Nie jest to samo w sobie wyjście analogowe ani bezpieczny stopień elektrodowy.
- Generator IR wskazuje D3/OC2B, ale D3 jest równocześnie PIN_EMERGENCY_STOP i PIN_IR_DETECT. Nie wolno aktywować fizycznego wyjścia IR na tym pinie do czasu zatwierdzenia rozdzielnej mapy pinów i sprzętowego E-Stop.
- piezo_set_tone() bezpośrednio nadpisuje Timer2 i nie generuje poprawnej, konfigurowalnej częstotliwości audio. Ta funkcja nie może współdzielić Timer2 z generatorem IR.
- ir_led_init() nie jest wywoływane w setup(), więc sam moduł IR nie jest obecnie inicjalizowany przez główny firmware.
- Aktualny read_gsr() zwraca analogRead(A0), mimo że A0 jest również opisane jako wejście czujnika temperatury. Nie jest to potwierdzony pomiar GSR. evaluate_gsr_quality() wykonuje jedynie statystykę tego surowego odczytu. Nie wolno przedstawiać tej wartości jako przewodności skóry ani rezystancji.
- Detekcja adresu I2C 0x48 nie identyfikuje sama w sobie układu ani nie implementuje odczytu pomiarowego. Konieczna jest identyfikacja konkretnego modułu/układu przed napisaniem sterownika.

### Kryteria fizycznego generatora

Każdy generator musi udostępniać:
- parametry żądane i rzeczywiście wyliczone (częstotliwość, wypełnienie, rozdzielczość);
- stan wyjścia: wyłączone / skonfigurowane / aktywne / błąd;
- jednoznacznego właściciela timera i pinu;
- wyłączenie sprzętowego wyjścia przy inicjalizacji, błędzie, resecie i nieprawidłowej konfiguracji;
- testy obliczeń częstotliwości i granic zakresu;
- pomiar oscyloskopem lub analizatorem logicznym dla testu fizycznego.

Nie deklarować testu fizycznego na podstawie samej kompilacji lub testu hostowego. Dla torów elektrodowych wymagane są dodatkowo pomiar prądu/napięcia na obciążeniu zastępczym i walidacja zabezpieczeń przed kontaktem z człowiekiem.


## Doprecyzowanie topologii wyjść — decyzja użytkownika (2026-11-07)

Przyjęta topologia logiczna:

- **IR PWM:** wyjście GPIO/PWM mikrokontrolera jest bezpośrednim źródłem sygnału sterującego wejściem toru IR. Fizyczny sygnał PWM ma być generowany przez mikrokontroler, a nie przez komputer.
- **Tor elektrodowy:** wyjście GPIO mikrokontrolera jest bezpośrednim źródłem sygnału sterującego wejściem izolatora galwanicznego. Za izolatorem znajduje się odrębny tor wyjściowy połączony z elektrodami.
- **GSR:** pozostaje osobnym torem pomiarowym; nie wolno utożsamiać GPIO generatora z wejściem pomiarowym GSR.

Sformułowanie „GPIO jako bezpośrednie wyjście elektrodowe” oznacza w tej architekturze bezpośrednie źródło przebiegu logicznego dla toru izolowanego, a nie podłączenie skóry/elektrod do pinu ATmega328P. Izolator zapewnia separację galwaniczną, ale sam nie stanowi ogranicznika prądu, napięcia ani energii i nie zastępuje zabezpieczeń wyjścia.

### Warunki implementacji

1. Firmware ma generować rzeczywisty przebieg na wskazanym GPIO/PWM i raportować parametry wyliczone dla sprzętu.
2. Wejście izolatora ma być sterowane z dedykowanego pinu, którego nie współdzieli się z E-Stop, SPI, I²C ani inną funkcją krytyczną.
3. Po stronie elektrod wymagany jest niezależny, sprzętowy stopień wyjściowy z ograniczeniem prądu i energii, ograniczeniem napięcia odpowiednim do zatwierdzonego zastosowania oraz sprzętowym stanem domyślnie wyłączonym.
4. E-Stop i odcięcie energii muszą działać niezależnie od programu i stanu GPIO; reset, zawieszenie MCU, utrata komunikacji lub zasilania nie mogą aktywować toru.
5. Testy firmware i toru wykonuje się najpierw na obciążeniu zastępczym. Bez identyfikacji izolatora i stopnia wyjściowego nie przypisujemy pinów ani parametrów elektrycznych.
6. Nie wolno włączać wyjścia elektrodowego na człowieku na podstawie samej poprawnej kompilacji, przebiegu logicznego na GPIO lub obecności izolatora.

### Konsekwencja dla aktualnej mapy pinów

Obecny D3 nie może być jednocześnie wyjściem IR PWM i wejściem E-Stop/detekcji IR. Zanim IR zostanie aktywowany, należy rozdzielić te funkcje. Wyjście sterujące izolatora elektrodowego również wymaga osobnego, wolnego pinu i jawnego właściciela w centralnej mapie zasobów. Nie należy wybierać pinu na podstawie samej nazwy w kodzie — trzeba uwzględnić rzeczywiste użycie peryferiów i konfliktów.
