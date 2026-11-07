# ResoNet Nano — protokół diagnostyczny v1

**Zakres:** pojedyncze, tylko do odczytu żądanie statusu. Protokół nie zawiera poleceń sterowania generatorami ani efektorami.

## Komendy

| Kierunek | Typ | Wartość | Payload |
|---|---|---:|---|
| Klient → Nano | STATUS_REQ | 0x05 | 0 bajtów |
| Nano → klient | STATUS_RESP | 0x06 | 14 bajtów |

HELLO, PING/PONG, ACK, NACK i ERROR zostały usunięte. Weryfikacja połączenia odbywa się przez STATUS_REQ/STATUS_RESP. Błędne, nieznane lub nieobsługiwane ramki są odrzucane bez odpowiedzi i bez zmiany stanu wyjść.

## Format ramki

Liczby wielobajtowe są big-endian.

| Offset | Bajty | Pole |
|---:|---:|---|
| 0 | 2 | magic: 0x52 0x4E („RN”) |
| 2 | 1 | version: 0x01 |
| 3 | 1 | type: 0x05 lub 0x06 |
| 4 | 1 | flags: 0 |
| 5 | 2 | sequence |
| 7 | 2 | payload_length: 0 lub 14 |
| 9 | 0/14 | payload |
| 9+N | 2 | CRC-16/CCITT-FALSE, big-endian |

CRC-16/CCITT-FALSE: polynomial 0x1021, init 0xFFFF, refin/refout false, xorout 0.

## STATUS_RESP — 14 bajtów

| Offset payloadu | Bajty | Pole |
|---:|---:|---|
| 0 | 1 | stan: 0x01 IDLE, 0x04 FAULT, 0x05 SAFE_LOCKOUT |
| 1 | 2 | fault_flags: bit 0 fault, bit 1 warning |
| 3 | 4 | uptime_seconds |
| 7 | 2 | free_memory_bytes |
| 9 | 2 | temperatura w setnych °C; 0x7FFF = brak wiarygodnego pomiaru |
| 11 | 2 | sensor_valid_flags; obecnie zarezerwowane, zero |
| 13 | 1 | output_inhibited; zero oznacza „brak potwierdzenia”, a nie gwarancję bezpieczeństwa |

Odpowiedź musi zawierać ten sam numer sequence co żądanie. Klient utrzymuje najwyżej jedno żądanie w toku. Licznik sequence może zawijać się z 65535 do 0.

## Ograniczenia i bezpieczeństwo

- TCP/5001, jeden klient jednocześnie, statyczny adres IP.
- Maksymalny payload 14 B, maksymalna odpowiedź 25 B.
- Parser firmware używa statycznego bufora i nie alokuje pamięci dynamicznie.
- Timeout niekompletnej ramki: 2 s; bezczynność klienta: 30 s.
- CRC wykrywa uszkodzenia przypadkowe, ale nie uwierzytelnia klienta i nie chroni przed replay.
- Nie wystawiać portu poza zaufaną sieć lokalną.
- Protokół diagnostyczny nie steruje wyjściami. Brak odpowiedzi lub błąd parsera nie może uruchamiać generatora.
- Wartość output_inhibited nie potwierdza niezależnego sprzętowego odcięcia.
- Implementacja nie stanowi potwierdzenia zgodności z IEC 60601-1 ani przydatności klinicznej.

## Zgodność

Firmware, klient desktop i klient Android muszą akceptować wyłącznie STATUS_REQ/STATUS_RESP. Wspólne testy sprawdzają CRC, numer sequence, długość payloadu, odrzucanie usuniętych komend oraz niepoprawnych ramek.
