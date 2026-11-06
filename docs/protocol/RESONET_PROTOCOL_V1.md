# ResoNet Nano — protokół komunikacyjny v1

**Status:** częściowo wdrożony — parser i diagnostyczny transport firmware oraz klient Android; adaptery desktop/WebUI/TUI i pełna zgodność międzyplatformowa pozostają do wykonania.  
**Zakres:** diagnostyka, identyfikacja urządzenia i kontrola łączności.  
**Sterowanie wyjściami:** celowo poza zakresem v1.

## 1. Zasady nadrzędne

1. Połączenie TCP nie oznacza gotowości urządzenia.
2. Urządzenie jest rozpoznane dopiero po poprawnym HELLO_RESP i zgodnej wersji protokołu.
3. Każde żądanie diagnostyczne otrzymuje odpowiedź z tym samym numerem sekwencyjnym.
4. Nieznany typ, wersja, flaga, długość, CRC lub payload powoduje odrzucenie ramki.
5. Błąd parsera nie może zmienić stanu wyjść. V1 nie zawiera poleceń aktywacji efektorów.
6. CRC wykrywa przypadkowe uszkodzenia, ale nie uwierzytelnia nadawcy ani nie chroni przed replay. Nie używać przez niezaufaną sieć.

## 2. Transport i ograniczenia

- Transport: TCP; port konfigurowalny. Firmware udostępnia diagnostyczny serwer na porcie 5001, gdy `RESONET_ENABLE_ETHERNET=1` i skonfigurowano adresację.
- TCP jest strumieniem bajtów: odczyt nie jest równoznaczny z ramką.
- Parser musi obsługiwać podział ramki między odczyty oraz wiele ramek w jednym odczycie.
- Maksymalny payload: 192 bajty.
- Maksymalna ramka: 203 bajty (9 nagłówka + 192 payloadu + 2 CRC).
- Bufor parsera firmware musi być statyczny i ograniczony.
- Zalecane timeouty: 3 s na TCP connect, 2 s na odpowiedź diagnostyczną.

## 3. Format ramki

Wszystkie liczby wielobajtowe są kodowane big-endian.

| Offset | Rozmiar | Pole | Znaczenie |
|---:|---:|---|---|
| 0 | 2 | magic | 0x52 0x4E (ASCII RN) |
| 2 | 1 | version | 0x01 |
| 3 | 1 | type | typ komunikatu |
| 4 | 1 | flags | w v1 wyłącznie 0x00 |
| 5 | 2 | sequence | licznik żądania 0–65535 |
| 7 | 2 | payload_length | 0–192 |
| 9 | N | payload | dane zależne od typu |
| 9+N | 2 | crc16 | CRC z offsetów 0 do końca payloadu |

CRC-16/CCITT-FALSE:
- polynomial: 0x1021
- initial value: 0xFFFF
- refin/refout: false
- xorout: 0x0000
- zapis CRC: big-endian.

Parser sprawdza magic, wersję, flags i długość przed przyjęciem payloadu. Po zebraniu całej ramki sprawdza CRC. Handler nie może zostać wywołany przed poprawną walidacją.

## 4. Typy komunikatów v1

| Typ | Wartość | Kierunek | Payload |
|---|---:|---|---|
| HELLO_REQ | 0x01 | klient → urządzenie | pusty |
| HELLO_RESP | 0x02 | urządzenie → klient | identyfikator i wersja firmware |
| PING | 0x03 | klient → urządzenie | pusty |
| PONG | 0x04 | urządzenie → klient | pusty |
| STATUS_REQ | 0x05 | klient → urządzenie | pusty |
| STATUS_RESP | 0x06 | urządzenie → klient | status diagnostyczny |
| NACK | 0x7D | urządzenie → klient | typ i kod odrzucenia |
| ERROR | 0x7E | urządzenie → klient | typ i kod błędu |
| ACK | 0x7F | urządzenie → klient | typ potwierdzanego komunikatu |

### 4.1 HELLO_RESP

| Pole | Rozmiar | Opis |
|---|---:|---|
| device_id | 8 | stały identyfikator urządzenia, bez danych osobowych |
| fw_major | 1 | wersja główna |
| fw_minor | 1 | wersja mniejsza |
| fw_patch | 1 | poprawka |
| capabilities | 2 | bitmapa możliwości; w v1 zarezerwowane, zero |

Łącznie 13 bajtów.

### 4.2 STATUS_RESP

| Pole | Rozmiar | Kodowanie |
|---|---:|---|
| system_state | 1 | enum poniżej |
| fault_flags | 2 | bitmapa: bit 0 = fault, bit 1 = warning; pozostałe bity zarezerwowane |
| uptime_seconds | 4 | uint32 |
| free_memory_bytes | 2 | uint16 |
| measured_temperature_centi_c | 2 | int16, setne °C; 0x7FFF = brak wiarygodnego pomiaru |
| sensor_valid_flags | 2 | bitmapa potwierdzonych pomiarów |
| output_inhibited | 1 | 1 = blokada aktywna; 0 nie dowodzi bezpieczeństwa |

Stany: 0x00 BOOT, 0x01 IDLE, 0x02 DIAGNOSTIC, 0x03 BUSY, 0x04 FAULT, 0x05 SAFE_LOCKOUT. Stan RUNNING i komendy sterujące nie należą do v1.

### 4.3 ERROR / NACK

Payload: offending_type (1 bajt), error_code (1 bajt). Kody:
- 0x01 UNSUPPORTED_VERSION
- 0x02 UNSUPPORTED_TYPE
- 0x03 INVALID_FLAGS
- 0x04 INVALID_LENGTH
- 0x05 INVALID_PAYLOAD
- 0x06 BAD_STATE
- 0x07 BUSY
- 0x08 INTERNAL_ERROR

Nie wysyłać kluczy, danych osobowych ani szczegółów wewnętrznych.

## 5. Numeracja i ponowienia

- Klient zwiększa sequence dla każdego nowego żądania; zawijanie 65535 → 0 jest dozwolone.
- Odpowiedź musi mieć identyczny sequence.
- Akceptacja wymaga zgodności typu, sequence, długości i CRC.
- Jedno żądanie w toku na połączenie; bez multipleksowania w v1.
- Po timeout nie zakładać, że żądanie nie zostało przetworzone. Dla diagnostycznego GET można ponowić z nowym sequence.
- Nieznany sequence oznacza odrzucenie odpowiedzi i zapis zdarzenia.

## 6. Parser strumienia

Zalecana maszyna stanów:
1. SEEK_MAGIC_0
2. SEEK_MAGIC_1
3. READ_FIXED_HEADER
4. VALIDATE_HEADER
5. READ_PAYLOAD_AND_CRC
6. VALIDATE_CRC
7. DISPATCH
8. RESET

Po błędnym nagłówku wrócić do wyszukiwania magic, zachowując możliwość użycia ostatniego bajtu 0x52 jako początku kolejnej sygnatury. Po błędnym CRC odrzucić ramkę bez wywołania handlera.

W firmware:
- statyczny bufor o ograniczonym rozmiarze;
- brak dynamicznej alokacji w parserze;
- handler nie uruchamia wyjść;
- sieć nie może blokować zadania bezpieczeństwa;
- CONNECTED oznacza potwierdzony interfejs sieciowy, a DIAGNOSTIC_READY wymaga poprawnego HELLO i STATUS.

## 7. Stany klienta

- DISCONNECTED — brak gniazda.
- CONNECTING — zestawianie TCP.
- CONNECTED_UNVERIFIED — TCP zestawione, brak poprawnego HELLO.
- IDENTIFIED — poprawne HELLO i rozpoznana wersja.
- DIAGNOSTIC_READY — diagnostyka odpowiada poprawnie; nie oznacza gotowości sterowania.
- FAULT — błąd protokołu lub urządzenia.
- SAFE_LOCKOUT — urządzenie zgłasza blokadę bezpieczeństwa.

Żaden stan klienta nie upoważnia samodzielnie do sterowania wyjściem.

## 8. Bezpieczeństwo i granice zastosowania

V1 służy wyłącznie diagnostyce w kontrolowanym środowisku. CRC nie zastępuje uwierzytelniania, szyfrowania, ochrony integralności ani ochrony przed replay. Przed jakimkolwiek sterowaniem wymagane będą: uwierzytelnienie urządzenia i klienta, świeżość komunikatów, autoryzacja, fail-safe sprzętowy, analiza zagrożeń i testy laboratoryjne. Sama implementacja protokołu nie potwierdza bezpieczeństwa ani skuteczności medycznej urządzenia.

## 9. Plan zgodności

1. Utworzyć wspólne wektory ramek i CRC.
2. Zaimplementować parser niezależny od sterowania w firmware.
3. Zaimplementować identyczne framing i walidację w Kotlinie.
4. Dodać adaptery desktop/WebUI po rozpoznaniu istniejących transportów.
5. Testować podział i łączenie odczytów TCP, błędne długości, CRC, typy, sequence i timeouty.
6. Dopiero po testach zgodności wdrażać serwer TCP i odczyt stanu linku.
