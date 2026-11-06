# ResoNet Protocol (RNP) — propozycja kontraktu v1

> Status: projekt specyfikacji. Nie jest jeszcze wdrożony w firmware ani klientach.

## Transport
- TCP dla niezawodnych komend i odpowiedzi.
- TLS lub równoważne uwierzytelnione szyfrowanie na łączach poza zaufaną magistralą.
- UART może przenosić te same ramki bez TCP.
- UDP nie przenosi komend aktywujących wyjścia bez osobnego mechanizmu niezawodności i autoryzacji.

## Ramka
Kodowanie binarne, pola wielobajtowe w kolejności network byte order:
1. magic: 4 bajty ASCII "RNET";
2. version: uint8;
3. message_type: uint8;
4. flags: uint16;
5. request_id: uint32;
6. device_id: 16 bajtów;
7. payload_length: uint16;
8. payload: payload_length bajtów;
9. crc32: uint32 nad polami od version do końca payload.

Maksymalny payload musi być ograniczony przez możliwości transportu i urządzenia. Odbiorca odrzuca nadmierną długość przed alokacją lub kopiowaniem.

## Zasady
- Każde żądanie ma request_id; ACK/NACK odnosi się do tego identyfikatora.
- Powtórzony request_id nie może wykonać operacji drugi raz.
- ACK oznacza przyjęcie żądania, niekoniecznie zakończenie operacji.
- Nieznana wersja, typ, flaga, długość lub CRC powodują odrzucenie ramki.
- Niepełna lub błędna ramka nie może zmieniać stanu wyjść.
- Po utracie zasilania lub łączności wyjścia przechodzą do sprzętowo wymuszonego stanu bezpiecznego. Wznowienie wymaga jawnego nowego polecenia.

## Typy komunikatów
HELLO, IDENTIFY, GET_STATUS, GET_CAPABILITIES, CONFIGURE_CHANNEL, START_SESSION, PAUSE_SESSION, RESUME_SESSION, STOP_SESSION, ACK, NACK, HEARTBEAT, FAULT, EMERGENCY_STOP.

## Bezpieczeństwo
CRC wykrywa przypadkowe uszkodzenia, ale nie zapewnia poufności ani autentyczności. Nie zastępuje TLS, MAC ani podpisu. Ta propozycja nie stanowi certyfikacji urządzenia medycznego.
