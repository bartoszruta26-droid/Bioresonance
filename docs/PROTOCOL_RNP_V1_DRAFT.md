# ResoNet Protocol (RNP) — status implementacji v1

> Status: desktopowy kodek, parser strumieniowy firmware Nano i diagnostyczna obsługa TCP są zaimplementowane w ograniczonym zakresie. Nie jest to kompletne wdrożenie end-to-end, uwierzytelniony protokół ani protokół sterowania terapią.

## Format ramki

Wersjonowany format jest wspólny dla desktopowego klienta i firmware:

- Nagłówek: 9 bajtów: magic `RN` (2 bajty), wersja (1), typ (1), flags (1), sequence (2, big-endian), payload length (2, big-endian).
- Payload: maksymalnie 192 bajty w ogólnym kodeku; obecne żądania firmware mają pusty payload.
- CRC: CRC-16/CCITT-FALSE (poly `0x1021`, init `0xFFFF`, bez refleksji, xorout `0x0000`), 2 bajty big-endian, liczone od początku ramki do końca payloadu.
- Kodek desktopowy i firmware walidują magic, wersję, zarezerwowane flagi, typ, długość i CRC.

## Zaimplementowane elementy

### Desktop

- Referencyjny kodek: `gui/include/resonet_protocol_v1.h`.
- Klient TCP realizuje diagnostyczne HELLO i STATUS oraz sprawdza typ odpowiedzi, numer sekwencyjny i limity czasu.
- GUI nie wysyła poleceń terapii ani tekstowych poleceń sterujących.

### Pełny firmware Nano

- Kodek i parser: `ResoNet_Nano/protocol_v1.h/.cpp`.
- Integracja transportu: `ResoNet_Nano/network_system.cpp`; serwer TCP/5001 jest kompilowany warunkowo.
- Parser jest bezalokacyjny, ma stały bufor 11 B i akceptuje wyłącznie żądania `HELLO_REQ`, `PING` i `STATUS_REQ` z pustym payloadem.
- Parser odrzuca nieznane typy, typy odpowiedzi przesłane jako żądania, niezgodną wersję, zarezerwowane flagi, nieprawidłową długość i błędne CRC.
- Parser obsługuje fragmentację i sklejanie ramek w strumieniu TCP oraz odzyskiwanie synchronizacji po uszkodzonej ramce.
- Warstwa sieciowa ogranicza odbiór do 24 bajtów na przebieg pętli, odrzuca niekompletną ramkę po 2 s i zamyka bezczynne połączenie po 30 s.
- Transport Ethernet pozostaje domyślnie wyłączony w konfiguracji kompilacji. CI sprawdza kompilację wariantu wyłączonego oraz wariantu z włączonym ENC28J60.

### Minimalny obraz diagnostyczny Nano

- `ResoNet_Diagnostic_Nano` jest odrębnym, minimalnym szkicem do testów transportu i modelu diagnostycznego.
- Nie konfiguruje GPIO ani PWM i nie może fizycznie zasilać efektorów.
- Nie należy utożsamiać kompilacji obrazu z testem rzeczywistego modułu ENC28J60 lub testem sprzętowym.

## Typy komunikatów

| Typ | Wartość | Długość payloadu |
|---|---:|---:|
| HELLO_REQ | 0x01 | 0 |
| HELLO_RESP | 0x02 | 13 |
| PING | 0x03 | 0 |
| PONG | 0x04 | 0 |
| STATUS_REQ | 0x05 | 0 |
| STATUS_RESP | 0x06 | 14 |
| NACK | 0x7D | 2 |
| ERROR | 0x7E | 2 |
| ACK | 0x7F | 1 |

Parser firmware przyjmuje wyłącznie trzy typy żądań: HELLO_REQ, PING i STATUS_REQ. Pozostałe typy są typami odpowiedzi lub komunikatami poza zakresem wejściowej ścieżki diagnostycznej.

## Poza zakresem / nierozwiązane

- Brak uwierzytelniania peer'a, negocjacji kluczy, TLS/MAC i ochrony przed powtórzeniem żądań. Prywatny adres IPv4 nie jest mechanizmem uwierzytelniania.
- Brak pełnego modelu identyfikacji urządzenia i uzgadniania capabilities.
- ACK/NACK/ERROR nie tworzą kompletnej, transakcyjnej ścieżki poleceń.
- Brak testów sprzętowych na rzeczywistym Arduino Nano i ENC28J60, testów odporności na utratę pakietów oraz długotrwałych testów stabilności.
- Brak niezależnego, sprzętowego odcięcia energii potwierdzonego schematem i pomiarami.
- Żadne polecenia RNP nie mogą aktywować kanałów energii. Nie należy dodawać takiej funkcji w ramach diagnostycznego protokołu.

## Weryfikacja

Hostowe testy parsera obejmują wektor referencyjny CRC, poprawne ramki, fragmentację, sklejone ramki, błędne CRC, zarezerwowane flagi, nieznany typ i odzyskiwanie synchronizacji po uszkodzonej ramce. CI kompiluje także oba warianty firmware i kontroluje budżet pamięci.

Zaliczenie CI potwierdza kompilację i testy hostowe, ale nie potwierdza działania fizycznego sprzętu, bezpieczeństwa elektrycznego, termicznego, EMC ani zgodności z wymaganiami dla wyrobów medycznych.

## Następne kroki

1. Dodać testy strumienia oparte na fuzzingu i testy graniczne timeoutów.
2. Uruchomić testy integracyjne desktop ↔ firmware z symulatorem TCP, a następnie z rzeczywistym Nano/ENC28J60.
3. Uzgodnić i wdrożyć osobną architekturę uwierzytelniania przed dopuszczeniem komunikacji poza odizolowaną siecią laboratoryjną.
4. Wykonać niezależny przegląd sprzętowy: pinout, zasilanie, odcięcie energii, pomiary termiczne i EMC.
