# ResoNet Diagnostic Nano — minimalny obraz diagnostyczny

To odrębny, minimalny firmware dla Arduino Nano ATmega328P + ENC28J60. Nie jest pełnym firmware urządzenia i nie może go zastępować w zastosowaniach wymagających podsystemów bezpieczeństwa.

## Co zawiera
- ENC28J60 i pojedynczy serwer TCP/5001
- ResoNet Protocol v1: HELLO_REQ, PING, STATUS_REQ
- parser strumieniowy o stałym buforze 11 B
- walidację magic, wersji, flags, długości i CRC-16/CCITT-FALSE
- timeout niekompletnej ramki i bezczynności klienta
- odpowiedzi diagnostyczne bez funkcji sterowania

## Czego nie zawiera
Brak PWM, IR LED, piezo, wibratora, detekcji efektorów, konfiguracji terapii, systemu logowania, kolejki zdarzeń i kontrolera bezpieczeństwa. Obraz nie steruje wyjściami i nie raportuje potwierdzonego odcięcia fizycznych wyjść. STATUS ma stan DIAGNOSTIC, temperaturę sentinel 0x7FFF i output_inhibited=0 (stan nieznany).

## Konfiguracja
Przed kompilacją sprawdź statyczne IP, MAC, bramę i maskę w `network_config.h`. Domyślnie jest to przykładowa sieć prywatna 192.168.1.0/24. Zmień ją przed podłączeniem do rzeczywistej sieci.

## Budowanie
Wymagane: Arduino CLI, core `arduino:avr`, EthernetENC 2.0.5.

```bash
arduino-cli core install arduino:avr
arduino-cli lib install EthernetENC@2.0.5
bash ResoNet_Diagnostic_Nano/build.sh

# Opcjonalnie: zachowaj ELF i mapę linkera do analizy zajętości Flash
RESONET_BUILD_DIR=/tmp/resonet-build RESONET_MAP_FILE=/tmp/resonet-diagnostic.map \\
  bash ResoNet_Diagnostic_Nano/build.sh
```

Skrypt tymczasowo ustawia `UIP_ACTIVE_OPEN=0` w lokalnej kopii EthernetENC, wykonuje kopię zapasową i przywraca oryginalny plik po zakończeniu kompilacji. Wykorzystuje domyślne optymalizacje platformy AVR (`-Os`, LTO i usuwanie nieużytych sekcji) oraz `-mcall-prologues`, `-maccumulate-args`, `-fshort-enums` i linkowanie `--relax`. Ogranicza uIP do jednego połączenia, jednego portu nasłuchującego, dwóch wpisów ARP, jednego bufora pakietu i bufora IP/TCP 128 B. To wystarcza dla ramek diagnostycznych do 25 B; większe transfery i wyższa przepustowość nie są celem tego obrazu.

## Testy hosta
```bash
g++ -std=c++11 -Wall -Wextra -Werror -pedantic tests/host/diagnostic_protocol_test.cpp -o diagnostic_protocol_test
./diagnostic_protocol_test
```

CI publikuje mapę linkera i log kompilacji, aby można było wskazać konkretne symbole zajmujące Flash. Limit odbioru obrazu minimalnego wynosi 20 000 B Flash i 1 200 B RAM.\n\nTesty hosta i kompilacja AVR nie są testami fizycznego sprzętu. Protokół v1 nie zapewnia uwierzytelniania ani szyfrowania; używaj wyłącznie w zaufanej, odizolowanej sieci.
