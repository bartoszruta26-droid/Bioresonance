# Wspólny kontrakt obsługi błędów GUI / WebUI / TUI

**Wersja:** 1.0

Każdy błąd ma stabilny kod maszynowy, bezpieczny komunikat po polsku i informację, czy można bezpiecznie ponowić odczyt diagnostyczny. Platformy zachowują własny sposób prezentacji, lecz nie zmieniają znaczenia kodów. Nie wolno ujawniać tokenów ani sekretów w komunikatach lub logach. Błąd komunikacji pozostawia stan urządzenia jako nieznany do czasu uzyskania poprawnej odpowiedzi.

## Format WebUI
```json
{"success":false,"error":{"code":"DEVICE_COMMUNICATION_FAILED","message":"Nie udało się uzyskać odpowiedzi urządzenia.","retryable":true}}
```
Kod HTTP opisuje odpowiedź HTTP; obiekt `error` opisuje przyczynę aplikacyjną.

## Słownik
| Kod | Znaczenie | Ponowienie |
|---|---|---:|
| INVALID_REQUEST | Nieprawidłowe dane żądania | Nie |
| AUTH_REQUIRED | Brak prawidłowego tokenu | Nie |
| METHOD_NOT_ALLOWED | Niedozwolona metoda HTTP | Nie |
| INVALID_DEVICE_ADDRESS | Adres poza dozwolonym zakresem | Nie |
| NOT_CONNECTED | Brak aktywnego połączenia | Tak |
| TRANSPORT_TIMEOUT | Przekroczono limit czasu | Tak |
| TRANSPORT_UNAVAILABLE | Urządzenie lub transport niedostępny | Tak |
| CONNECTION_CLOSED | Połączenie zamknięte przed końcem odpowiedzi | Tak |
| DEVICE_COMMUNICATION_FAILED | Błąd wymiany z urządzeniem | Tak |
| DEVICE_REJECTED | Urządzenie odrzuciło żądanie | Nie |
| PROTOCOL_UNSUPPORTED_CAPABILITY | Urządzenie zgłosiło nieobsługiwaną funkcję | Nie |
| DEVICE_STATUS_INVALID | Nieprawidłowy status urządzenia | Nie |
| DEVICE_UNAVAILABLE | Urządzenie niedostępne lub niewykryte | Tak |
| OPERATION_UNSUPPORTED | Operacja nieobsługiwana przez bieżący protokół | Nie |
| PROTOCOL_INVALID_HEADER | Nieprawidłowy nagłówek | Nie |
| PROTOCOL_INVALID_LENGTH | Nieprawidłowa długość ramki | Nie |
| PROTOCOL_CRC_MISMATCH | Niezgodna suma kontrolna | Nie |
| PROTOCOL_SEQUENCE_MISMATCH | Niezgodny numer sekwencji | Nie |
| PROTOCOL_UNEXPECTED_TYPE | Nieoczekiwany typ odpowiedzi | Nie |
| PROTOCOL_UNSUPPORTED_CAPABILITY | Nieobsługiwana funkcja protokołu | Nie |
| PROTOCOL_INVALID_RESPONSE | Inna niezgodność odpowiedzi | Nie |
| INTERNAL_ERROR | Nieoczekiwany błąd aplikacji | Nie |
| FILE_READ_FAILED | Nie udało się odczytać pliku | Nie |
| FILE_WRITE_FAILED | Nie udało się zapisać pliku | Nie |
| FILE_INVALID_FORMAT | Nieprawidłowy format pliku | Nie |
| FILE_NO_VALID_RECORDS | Brak poprawnych rekordów danych | Nie |

## Wymagania platform
- **Desktop GUI:** centralny katalog `ui_error_catalog.h` mapuje kod na komunikat i możliwość ponowienia; szczegóły techniczne są dostępne osobno.
- **WebUI:** backend zwraca obiekt `error`; frontend nie zakłada typu tekstowego i nie renderuje treści błędu jako HTML.
- **TUI:** wyjątek przenosi `code` i `retryable`; terminal używa prefiksu `BŁĄD [KOD]`.
- **Retry:** wyłącznie dla odczytowych operacji diagnostycznych; bez automatycznych pętli ponawiania.

Kontrakt nie dodaje sterowania urządzeniem ani nie potwierdza bezpieczeństwa fizycznego.
