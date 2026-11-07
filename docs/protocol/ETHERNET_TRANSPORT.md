# Warstwa transportowa ENC28J60 — wdrożenie diagnostyczne

## Biblioteka

Firmware używa API biblioteki EthernetENC. Zalecana, przypięta wersja: **2.0.5**. Biblioteka zapewnia interfejs zgodny z API Arduino Ethernet i obsługę ENC28J60 przez SPI. Wersję należy zainstalować w Arduino IDE Library Manager lub przypiąć w środowisku budowania. Zalecana kompilacja: `bash ResoNet_Nano/build_nano.sh`.

## Domyślna blokada

`RESONET_ENABLE_ETHERNET` w `network_config.h` ma domyślną wartość `0`. W tym ustawieniu firmware nie inicjalizuje ENC28J60 i nie deklaruje połączenia. Jest to celowe: adres IP, brama, maska i MAC muszą zostać dopasowane do konkretnej sieci przed aktywacją.

Aby włączyć transport, ustaw `RESONET_ENABLE_ETHERNET 1` i zweryfikuj, że:
- statyczny adres IP jest wolny i należy do właściwej podsieci;
- MAC jest unikalny w segmencie;
- adresy DNS, bramy i maski są prawidłowe;
- TCP/5001 jest osiągalny tylko z zaufanej, odizolowanej sieci.

Wartości w pliku są przykładową konfiguracją prywatnej podsieci, a nie uniwersalnym ustawieniem.

Skrypt kompilacyjny ogranicza zasoby stosu: `UIP_CONF_UDP=0`, `UIP_CONF_MAX_CONNECTIONS=1`, `UIP_CONF_MAX_LISTENPORTS=1`, `UIP_CONF_BUFFER_SIZE=128`, `UIP_CONF_ARPTAB_SIZE=2`, `UIP_SOCKET_NUMPACKETS=1`, bufory UART RX/TX po 32 bajty. Bufor uIP 128 B jest dobrany do krótkich ramek diagnostycznych v1 (jedyna odpowiedź ma 25 B), kosztem przepustowości; nie należy go traktować jako konfiguracji do dużych transferów. UDP jest wyłączone, więc DHCP i DNS są niedostępne; wymagany jest statyczny adres IP. Jeden klient TCP i pojedynczy bufor pakietu ograniczają przepustowość.

## Zachowanie serwera

- Jeden klient TCP jednocześnie.
- Przyjęcie połączenia przez `EthernetServer.accept()`.
- Limit 24 odebranych bajtów na jedno wywołanie `network_loop()`, aby ograniczyć czas obsługi.
- Parser status-only ResoNet Protocol v1; jedyna odpowiedź to STATUS_RESP.
- Nieznany typ komunikatu jest odrzucany bez odpowiedzi.
- Parser firmware przechowuje wyłącznie nagłówek i CRC dla żądań klienta o pustym payloadzie; znane typy odpowiedzi nie są akceptowane w kierunku klient → urządzenie.
- Nieznane typy, nieprawidłowe długości i błędne CRC są odrzucane; handler otrzymuje wyłącznie poprawne STATUS_REQ.
- Nieobsługiwane są stare, niewersjonowane pakiety terapii i konfiguracji.
- Brak komend uruchamiania PWM, IR LED, piezo lub wibratora.
- STATUS_RESP ustawia `output_inhibited=0`, ponieważ firmware nie ma niezależnego potwierdzenia fizycznego odcięcia wszystkich wyjść. Pole temperatury zewnętrznego efektora ma wartość sentinel `0x7FFF`. Nie należy interpretować tego statusu jako potwierdzenia bezpieczeństwa urządzenia.

## Wynik bazowy przed optymalizacją

Poprzednia kompilacja wariantu EthernetENC przed wyłączeniem aktywnego otwierania TCP i przed dodatkowymi flagami oszczędzania kodu wynosiła **29 442 B flash (95%) i 1 409 B RAM (68%)**, pozostawiając 639 B RAM. To pomiar bazowy, a nie wynik aktualnego skryptu.

## Ograniczenia i dalsze prace

1. Biblioteka EthernetENC używa własnych buforów i stosu TCP; należy zmierzyć zużycie RAM/flash w kompilacji dla ATmega328P.
2. Timeout częściowej ramki korzysta z `Parser::hasPartialFrame()` i czasu ostatniego odebranego bajtu; po 2 sekundach bez kontynuacji parser jest resetowany.
3. Dodano testy hostowe parsera i zadanie CI kompilujące firmware; wynik CI wymaga osobnego potwierdzenia. Testy sprzętowe są poza zakresem tej iteracji.
4. Protokół v1 nie ma uwierzytelniania ani szyfrowania. Nie wystawiać portu poza zaufaną sieć.
5. Kod transportowy nie stanowi potwierdzenia bezpieczeństwa ani przydatności klinicznej urządzenia.


## Optymalizacja kompilacji serwerowej

Dla wariantu ENC28J60 build script automatycznie modyfikuje lokalną kopię przypiętej biblioteki EthernetENC 2.0.5: ustawia `UIP_ACTIVE_OPEN=0`. Firmware działa jako serwer TCP i nie inicjuje połączeń wychodzących, dlatego kod aktywnego otwierania TCP jest zbędny. Funkcja serwera i odpowiedzi na przychodzące połączenia pozostają włączone. Skrypt wykonuje kopię zapasową pliku konfiguracyjnego i przywraca oryginał po kompilacji (także po błędzie). Przerywa kompilację, jeśli nie odnajdzie oczekiwanej wersji biblioteki lub nie może bezpiecznie zastosować zmiany.

Dodatkowo AVR jest kompilowany z `-mcall-prologues`, a UDP pozostaje wyłączone. Ograniczenie bufora uIP do 128 B oraz listy nasłuchu do jednego portu zmniejsza przede wszystkim zużycie RAM. CI wymaga obecnie nie więcej niż 28 800 B Flash dla wariantu Ethernet, czyli co najmniej 1 920 B rezerwy względem limitu 30 720 B. **Nowy wynik liczbowy nie został jeszcze potwierdzony**; do czasu zielonej kompilacji nie należy uznawać celu pamięciowego za osiągnięty. Nie jest to pomiar fizycznego sprzętu.
