# Warstwa transportowa ENC28J60 — wdrożenie diagnostyczne

## Biblioteka

Firmware używa API biblioteki EthernetENC. Zalecana, przypięta wersja: **2.0.5**. Biblioteka zapewnia interfejs zgodny z API Arduino Ethernet i obsługę ENC28J60 przez SPI. Wersję należy zainstalować w Arduino IDE Library Manager lub przypiąć w środowisku budowania.

## Domyślna blokada

`RESONET_ENABLE_ETHERNET` w `network_config.h` ma domyślną wartość `0`. W tym ustawieniu firmware nie inicjalizuje ENC28J60 i nie deklaruje połączenia. Jest to celowe: adres IP, brama, maska i MAC muszą zostać dopasowane do konkretnej sieci przed aktywacją.

Aby włączyć transport, ustaw `RESONET_ENABLE_ETHERNET 1` i zweryfikuj, że:
- statyczny adres IP jest wolny i należy do właściwej podsieci;
- MAC jest unikalny w segmencie;
- adresy DNS, bramy i maski są prawidłowe;
- TCP/5001 jest osiągalny tylko z zaufanej, odizolowanej sieci.

Wartości w pliku są przykładową konfiguracją prywatnej podsieci, a nie uniwersalnym ustawieniem.

## Zachowanie serwera

- Jeden klient TCP jednocześnie.
- Przyjęcie połączenia przez `EthernetServer.accept()`.
- Limit 24 odebranych bajtów na jedno wywołanie `network_loop()`, aby ograniczyć czas obsługi.
- Parser ResoNet Protocol v1; odpowiedzi HELLO_RESP, PONG, STATUS_RESP.
- Nieznany dla roli klienta typ komunikatu otrzymuje NACK.
- Niepoprawne ramki nie są przekazywane do handlera.
- Nieobsługiwane są stare, niewersjonowane pakiety terapii i konfiguracji.
- Brak komend uruchamiania PWM, IR LED, piezo lub wibratora.
- STATUS_RESP zawsze deklaruje `output_inhibited=1`; pomiar temperatury zewnętrznego efektora nie jest dostępny, więc pole temperatury ma wartość sentinel `0x7FFF`.

## Ograniczenia i dalsze prace

1. Biblioteka EthernetENC używa własnych buforów i stosu TCP; należy zmierzyć zużycie RAM/flash w kompilacji dla ATmega328P.
2. Obecny timeout częściowej ramki powinien zostać dopracowany przez dodanie jawnej metody `Parser::isIdle()` lub czasu ostatniego bajtu wewnątrz parsera. Nie należy resetować parsera wyłącznie na podstawie licznika odrzuceń.
3. Należy dodać testy hostowe parsera i testy kompilacji firmware w CI; testy sprzętowe są poza zakresem tej iteracji.
4. Protokół v1 nie ma uwierzytelniania ani szyfrowania. Nie wystawiać portu poza zaufaną sieć.
5. Kod transportowy nie stanowi potwierdzenia bezpieczeństwa ani przydatności klinicznej urządzenia.
