# ResoNet Nano — aplikacja Android (diagnostyka)

**Status:** prototyp diagnostyczny, klient tylko do odczytu.  
**Platforma:** Android · Kotlin · MVVM · StateFlow · Coroutines.  
**Transport:** TCP, ResoNet Protocol v1, port domyślny 5001.

## Zakres rzeczywiście obsługiwany

Aplikacja zapewnia szkielet natywnego interfejsu i klienta diagnostycznego. Klient:
- nawiązuje TCP i uznaje połączenie dopiero po poprawnym HELLO_RESP;
- sprawdza typ i numer sekwencyjny odpowiedzi;
- dekoduje binarne ramki ResoNet Protocol v1 i weryfikuje CRC;
- pobiera STATUS_RESP;
- udostępnia diagnostyczne informacje o urządzeniu i stanie protokołu.

Aktualny transport v1 jest tylko do odczytu. Nie obsługuje konfiguracji kanałów, START, STOP ani skanowania efektorów. Powiązane przyciski w interfejsie są wyłączone celowo. Nie należy ich ponownie aktywować bez zaprojektowania, implementacji i przetestowania autoryzowanego protokołu sterującego oraz niezależnych zabezpieczeń sprzętowych.

## Architektura kodu

| Plik | Odpowiedzialność |
|---|---|
| `MainActivity.kt` | Widok i obsługa cyklu życia UI |
| `MainViewModel.kt` | Stan UI i koordynacja diagnostyki |
| `ArduinoCommunication.kt` | Klient TCP i operacje protokołu |
| `ResoNetProtocolV1.kt` | Kodowanie/dekodowanie ramek v1 |
| `Models.kt`, `Types.kt` | Modele i typy domenowe |
| `FrequencyDatabase.kt` | Lokalna baza opisowa; zawartość wymaga weryfikacji źródła |
| `TherapySessionManager.kt` | Historyczny szkielet sesji; nie jest połączony z obsługą wyjść v1 |

## Budowanie i testowanie

Wymagania: JDK 17, Gradle 8.2.1 oraz Android SDK zgodny z konfiguracją projektu.

Z katalogu `android_app`:

```bash
gradle testDebugUnitTest assembleDebug --no-daemon
```

Testy jednostkowe znajdują się w `app/src/test`.

## Konfiguracja i połączenie

1. Zainstaluj wersję debug na urządzeniu testowym.
2. Wprowadź adres IPv4 urządzenia i port TCP.
3. Połącz się wyłącznie z urządzeniem w zaufanej sieci laboratoryjnej.
4. Zweryfikuj tożsamość/wersję w odpowiedzi HELLO i odczytaj status.
5. Traktuj brak pomiaru jako brak danych, a nie jako wartość zerową lub potwierdzenie bezpieczeństwa.

Samo zestawienie TCP nie potwierdza poprawności hardware, stanu wyjść ani bezpieczeństwa urządzenia. CRC wykrywa przypadkowe uszkodzenia, ale nie zapewnia uwierzytelnienia ani szyfrowania.

## Ograniczenia i bezpieczeństwo

- Projekt jest prototypem eksperymentalnym, a nie zwalidowanym wyrobem medycznym.
- Nie używać do ekspozycji człowieka ani do sterowania obciążeniami.
- Nie deklarować skuteczności klinicznej ani zgodności z IEC 60601 na podstawie testów aplikacji.
- Sterowanie wyjściami pozostaje wyłączone.
- Połączenia testować w odizolowanej, zaufanej sieci.
- Politykę cleartext, kopii zapasowych i zaufania do urządzenia należy ocenić przed wdrożeniem produkcyjnym.

## Status funkcji

| Funkcja | Status |
|---|---|
| TCP connect + HELLO v1 | Zaimplementowane; testy jednostkowe |
| Odczyt STATUS v1 | Zaimplementowany; testy jednostkowe |
| Weryfikacja CRC i sequence | Zaimplementowana; testy jednostkowe |
| Konfiguracja kanałów urządzenia | Niedostępna w v1 |
| START / STOP | Niedostępne w v1 |
| Skanowanie efektorów | Niedostępne w v1 |
| Pełny interfejs 8 kanałów | Nieukończony |
| Trwały harmonogram sesji | Nieukończony |
| Integracja end-to-end z firmware | Niepotwierdzona |

Dokumentacja opisuje wyłącznie aktualny zakres diagnostyczny i nie stanowi deklaracji gotowości sprzętowej ani medycznej.
