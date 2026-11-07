# ResoNet Protocol (RNP) — status implementacji v1

> Status: częściowo zaimplementowany kodek i diagnostyczny klient TCP. To nie jest kompletne wdrożenie protokołu end-to-end ani protokół sterowania terapią.

## Implementowane elementy

Plik referencyjny dla desktopowego kodeka: `gui/include/resonet_protocol_v1.h`.

- Nagłówek: 9 bajtów: magic `RN` (2 bajty), wersja (1), typ (1), flags (1), sequence (2, big-endian), payload length (2, big-endian).
- Payload: maksymalnie 192 bajty.
- CRC: CRC-16/CCITT-FALSE (poly `0x1021`, init `0xFFFF`, bez refleksji, xorout `0x0000`), 2 bajty big-endian, liczone od początku ramki do końca payloadu.
- Kodek waliduje magic, wersję, zarezerwowane flagi, znane typy, długości właściwe dla typu, limit ramki i CRC.
- Desktopowy klient wykonuje HELLO i STATUS oraz sprawdza typ odpowiedzi, numer sekwencyjny i ograniczone limity czasu.

## Typy i długości payloadu klienta

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

## Poza zakresem / niezaimplementowane

- Firmware Nano nie implementuje obecnie zgodnego parsera RNP ani rzeczywistej obsługi ENC28J60.
- Brak kompletnej identyfikacji urządzenia, autoryzacji, ochrony przed powtórzeniem żądań i uzgodnionego modelu capability.
- Brak TLS lub równoważnego uwierzytelnionego zabezpieczenia łącza; ograniczenie adresów IPv4 do sieci prywatnej nie uwierzytelnia peer'a.
- ACK/NACK/ERROR są rozpoznawane przez kodek, ale nie tworzą kompletnej transakcyjnej ścieżki komend.
- Żadne polecenia nie mogą aktywować kanałów energii. Klient GUI nadal blokuje wysyłanie struktur terapii i poleceń tekstowych.

## Wymagane następne kroki

1. Uzgodnić wersjonowaną specyfikację wire-format i niezależne testy wektorów z implementacją firmware.
2. Dodać parser strumienia odporny na fragmentację, sklejenie ramek, timeout i limit długości.
3. Zaimplementować prawdziwy sterownik ENC28J60 i bezpieczny transport diagnostyczny — po zatwierdzeniu pinoutu SPI.
4. Dodać symulator peer'a i testy integracyjne kodu desktopowego z firmware parserem.
5. Przeprowadzić osobne przeglądy bezpieczeństwa i sprzętowe testy odcięcia energii. CRC nie zapewnia autentyczności ani poufności.

Samo zaliczenie CI nie potwierdza bezpieczeństwa elektrycznego, termicznego, EMC ani zgodności z wymaganiami dla wyrobów medycznych.
