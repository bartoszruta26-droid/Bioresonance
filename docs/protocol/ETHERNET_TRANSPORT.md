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

Skrypt kompilacyjny ogranicza zasoby stosu: `UIP_CONF_UDP=0`, `UIP_CONF_MAX_CONNECTIONS=1`, `UIP_SOCKET_NUMPACKETS=1`, bufory UART RX/TX po 32 bajty. UDP jest wyłączone, więc DHCP i DNS są niedostępne; wymagany jest statyczny adres IP. Jeden klient TCP i pojedynczy bufor pakietu ograniczają przepustowość.

## Zachowanie serwera

- Jeden klient TCP jednocześnie.
- Przyjęcie połączenia przez `EthernetServer.accept()`.
- Limit 24 odebranych bajtów na jedno wywołanie `network_loop()`, aby ograniczyć czas obsługi.
- Parser ResoNet Protocol v1; odpowiedzi HELLO_RESP, PONG, STATUS_RESP.
- Nieznany dla roli klienta typ komunikatu otrzymuje NACK.
- Parser firmware przechowuje wyłącznie nagłówek i CRC dla żądań klienta o pustym payloadzie; znane typy odpowiedzi nie są akceptowane w kierunku klient → urządzenie.
- Nieznane typy z pustym payloadem otrzymują NACK; błędne ramki nie są przekazywane do handlera.
- Nieobsługiwane są stare, niewersjonowane pakiety terapii i konfiguracji.
- Brak komend uruchamiania PWM, IR LED, piezo lub wibratora.
- STATUS_RESP ustawia `output_inhibited=0`, ponieważ firmware nie ma niezależnego potwierdzenia fizycznego odcięcia wszystkich wyjść. Pole temperatury zewnętrznego efektora ma wartość sentinel `0x7FFF`. Nie należy interpretować tego statusu jako potwierdzenia bezpieczeństwa urządzenia.

## Wynik kompilacji programowej

Wariant z włączonym transportem kompiluje się dla Arduino Nano ATmega328P w konfiguracji skryptu: **29 442 B flash (95%) i 1 409 B RAM (68%)**, pozostawiając 639 B RAM na zmienne lokalne/stos. To wynik kompilacji, nie testu działania. Margines pamięci programu jest niewielki; każda większa funkcja wymaga ponownego pomiaru.

## Ograniczenia i dalsze prace

1. Biblioteka EthernetENC używa własnych buforów i stosu TCP; należy zmierzyć zużycie RAM/flash w kompilacji dla ATmega328P.
2. Timeout częściowej ramki korzysta z `Parser::hasPartialFrame()` i czasu ostatniego odebranego bajtu; po 2 sekundach bez kontynuacji parser jest resetowany.
3. Dodano testy hostowe parsera i zadanie CI kompilujące firmware; wynik CI wymaga osobnego potwierdzenia. Testy sprzętowe są poza zakresem tej iteracji.
4. Protokół v1 nie ma uwierzytelniania ani szyfrowania. Nie wystawiać portu poza zaufaną sieć.
5. Kod transportowy nie stanowi potwierdzenia bezpieczeństwa ani przydatności klinicznej urządzenia.


## Optymalizacja kompilacji serwerowej

Dla wariantu ENC28J60 build script automatycznie modyfikuje lokalną kopię przypiętej biblioteki EthernetENC 2.0.5: ustawia `UIP_ACTIVE_OPEN=0`. Firmware działa jako serwer TCP i nie inicjuje połączeń wychodzących, dlatego kod aktywnego otwierania TCP jest zbędny. Funkcja serwera i odpowiedzi na przychodzące połączenia pozostają włączone. Skrypt przerywa kompilację, jeśli nie odnajdzie oczekiwanego pliku lub nie może bezpiecznie zastosować zmiany.

Dodatkowo AVR jest kompilowany z `-mcall-prologues`, a UDP pozostaje wyłączone. CI wymaga obecnie nie więcej niż 28 800 B Flash dla wariantu Ethernet. Wynik tej optymalizacji należy potwierdzić kompilacją; nie jest to pomiar fizycznego sprzętu.
