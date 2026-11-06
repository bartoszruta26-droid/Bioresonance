# Raport audytu technicznego — ResoNet Nano / Bioresonance

**Data:** 2026-11-06  
**Zakres:** przegląd statyczny gałęzi `main` przez GitHub API; struktura repozytorium, wybrane pliki Kotlin/C++/Arduino, konfiguracja CI i testy.  
**Ograniczenie:** nie uruchomiono kompilacji ani testów sprzętowych w tym audycie. Wnioski statyczne nie zastępują testów HIL ani walidacji bezpieczeństwa urządzenia.

## 1. Werdykt

**Status: NOT READY — nie zatwierdzać do sterowania wyjściami ani zastosowań medycznych.**

Repozytorium ma wieloplatformowy szkielet i ostatnie poprawki dotyczące synchronizacji gniazd, ograniczenia czasu ramek i obsługi cyklu życia Androida. Jednak warstwa sieciowa firmware jest atrapą, a Android świadomie blokuje operacje sterujące. Są to poprawne bezpieczne ograniczenia, lecz oznaczają brak gotowości funkcjonalnej end-to-end.

## 2. Ustalenia krytyczne

### AUD-001 — Brak rzeczywistego stosu sieciowego ENC28J60
**Plik:** `ResoNet_Nano/network_system.cpp`  
Funkcja `enc28j60_init()` jest opisana jako stub, ustawia jedynie pin CS i zwraca `true`. `network_init()` na tej podstawie ustawia `NET_STATUS_CONNECTED`. `network_loop()` nie odbiera pakietów. `network_send_status()` jedynie zwiększa licznik, bez transmisji.
**Skutek:** status połączenia jest niewiarygodny; sterowanie sieciowe nie działa.
**Priorytet:** P0.

### AUD-002 — Brak zgodnego serwera TCP
**Pliki:** `ResoNet_Nano/network_system.cpp`, `android_app/app/src/main/java/com/resonet/nano/ArduinoCommunication.kt`  
Android łączy się domyślnie z TCP/5001, ale firmware nie uruchamia serwera TCP. Klient Android prawidłowo dokumentuje tę rozbieżność i odmawia komend `configureChannel`, `startTherapy`, `stopTherapy`.
**Skutek:** nie istnieje działający, zweryfikowany tor komunikacji aplikacja–firmware.
**Priorytet:** P0.

### AUD-003 — Menedżer sesji ignoruje błędy operacji sprzętowych
**Plik:** `android_app/app/src/main/java/com/resonet/nano/TherapySessionManager.kt`  
Wyniki `configureChannel`, `startTherapy` i `stopTherapy` nie są sprawdzane. Menedżer może przejść do odliczania i stanu Completed mimo niepowodzenia operacji. Obsługa pauzy/wznowienia anuluje zadanie i rozpoczyna preset od początku.
**Skutek:** stan prezentowany użytkownikowi może rozjechać się ze stanem urządzenia.
**Priorytet:** P0 dla sterowania, P1 dla poprawności UI.

### AUD-004 — Ochrona temperaturowa nie mierzy temperatury urządzenia
**Plik:** `ResoNet_Nano/safety_system.cpp`  
Funkcja `measure_temperature()` wykorzystuje wewnętrzne źródło ADC mikrokontrolera jako przybliżenie. Nie jest to pomiar temperatury cewki, obudowy ani elementu mocy. Wynik ma charakter estymacyjny i jest wymuszany do minimum 25°C.
**Skutek:** zabezpieczenie nie może wykryć przegrzania elementów wykonawczych.
**Priorytet:** P0.

### AUD-005 — Watchdog nie jest niezależnym odłączeniem energii
**Plik:** `ResoNet_Nano/safety_system.cpp`, `ResoNet_Nano/ResoNet_Nano.ino`  
Timeout warstwy ustawia flagę krytyczną/latch; natomiast właściwe wyłączenie PWM zależy od obsługi w pętli głównej. Brakuje potwierdzonego, niezależnego sprzętowego toru odłączenia energii.
**Skutek:** nie można traktować obecnej logiki jako bezpiecznego wyłącznika awaryjnego.
**Priorytet:** P0.

### AUD-006 — Zakres PWM nie został wykazany jako osiągalny
**Plik:** `ResoNet_Nano/pwm_engine.cpp`  
Dobór preskalera i ograniczanie ICR1 do zakresu 100–65535 oznacza, że częstotliwość zadana może być ograniczana lub różnić się od rzeczywistej. Algorytm nie zwraca zmierzonej/obliczonej częstotliwości rzeczywistej ani nie odrzuca wartości nieosiągalnych. Należy wykonać analizę całego zakresu, bez założeń, że 0,1 Hz–500 kHz jest obsługiwane.
**Priorytet:** P0.

## 3. Ustalenia wysokie

### AUD-007 — Obsługa ramek Androida opiera się na ciszy po danych
**Plik:** `ArduinoCommunication.kt`  
Klient ma limity rozmiaru, czas całkowity, rygorystyczne dekodowanie UTF-8, mutex i walidację komend. Jednak ramka legacy jest kończona po timeout bezczynności, a nie na podstawie jawnego framingu protokołu. To rozwiązanie przejściowe, podatne na ucięcie odpowiedzi, jeśli serwer wysyła dane wolniej.
**Priorytet:** P1. W protokole docelowym użyć długości ramki lub jawnego delimiter+escaping i jednoznacznego ACK/NACK.

### AUD-008 — Lifecycle close() omija mutex operacji
**Plik:** `ArduinoCommunication.kt`  
Metoda synchroniczna `close()` może zamykać socket spoza `operationMutex`, co ma przerwać blokujące I/O podczas teardown. To jest celowe, ale wymaga testów wyścigów: connect vs close, close vs sendCommand, dwa równoległe disconnect/close oraz publikacja stanu po zamknięciu.
**Priorytet:** P1.

### AUD-009 — Nieprawidłowa definicja temperatury w statusie
**Plik:** `ArduinoCommunication.kt`  
Parser wymaga pola `Temperature`, ale status Arduino pokazuje estymację temperatury MCU. Model powinien odróżniać brak czujnika, nieważny pomiar i zmierzoną temperaturę konkretnego podzespołu. Nie można prezentować estymacji jako temperatury systemu/wyjścia.
**Priorytet:** P1.

### AUD-010 — Testy Androida nie pokrywają scenariuszy awarii
**Plik:** `android_app/app/src/test/java/com/resonet/nano/ArduinoCommunicationTest.kt`  
Widoczne testy sprawdzają LF, limit odpowiedzi i odrzucenie wstrzyknięcia przez CR/LF. Brakuje testów parsowania statusu, niekompletnych/za dużych ramek, wolnego strumienia, disconnect podczas I/O, timeoutu całkowitego i wyścigów lifecycle.
**Priorytet:** P1.

### AUD-011 — Manifest Androida dopuszcza cleartext i backup
**Plik:** `android_app/app/src/main/AndroidManifest.xml`  
`usesCleartextTraffic=true` dopuszcza niezabezpieczony ruch, a `allowBackup=true` wymaga jawnej polityki wykluczeń. Nie wykazano uwierzytelniania peerów ani ochrony integralności transmisji.
**Priorytet:** P1 przed wdrożeniem w sieciach nieufnych.

## 4. Ustalenia średnie

### AUD-012 — Dane częstotliwości Androida są przykładowe
**Plik:** `MainViewModel.kt`  
`loadFrequencyDatabase()` ładuje osiem rekordów z tekstu mock zamiast rzeczywistego pliku assets/frequencies.md.
**Priorytet:** P2.

### AUD-013 — Harmonogram nie jest trwały
**Plik:** `TherapySessionManager.kt`  
`ScheduleManager` przechowuje dane w mutableListOf w pamięci procesu. Po zamknięciu aplikacji wpisy znikną; brak trwałego źródła danych i egzekutora harmonogramu.
**Priorytet:** P2.

### AUD-014 — Dokumentacja deklaruje funkcjonalności nieukończone
**Plik:** `IMPLEMENTATION_SUMMARY.md`  
Dokument sam wskazuje braki w C++ TUI, WebUI, harmonogramach i offline. Główne README opisuje pełniejszą architekturę niż faktycznie potwierdza kod firmware.
**Priorytet:** P2. Wprowadzić macierz statusu: implemented/tested, implemented/not-tested, stub, planned.

### AUD-015 — Niepełne pokrycie CI
**Plik:** `.github/workflows/desktop-gui.yml`  
CI obejmuje GUI, PHP/JS, Android i testy. Nie ma widocznego zadania budującego firmware z przypiętym toolchainem Arduino/PlatformIO ani testów protokołu firmware. Ostatnie commity nie zwróciły powiązanych wyników workflow w przeglądzie.
**Priorytet:** P1.

## 5. Pozytywne elementy

- Klient Android używa `Mutex` do serializacji operacji na strumieniu.
- Waliduje puste komendy, CR/LF i limit długości.
- Ogranicza wielkość odpowiedzi i czas odczytu.
- Wymusza prawidłowe UTF-8 i odrzuca NUL.
- Używa `CopyOnWriteArrayList` dla słuchaczy.
- Ostatnie poprawki desktopu dotyczą przerywania blokującego I/O podczas rozłączenia.
- Test desktopowy obejmuje przerwanie odbioru przez disconnect.
- Android ma podstawowy test lokalnego serwera TCP.

## 6. Plan naprawczy — kolejność obowiązkowa

### Etap 0 — ograniczenie ryzyka
1. Utrzymać wyłączone komendy start/stop i konfigurację wyjść w Androidzie.
2. Oznaczyć wszystkie statusy połączenia firmware jako niezweryfikowane do czasu prawdziwego link check.
3. W dokumentacji i UI oznaczyć urządzenie jako prototyp eksperymentalny.
4. Nie używać urządzenia do ekspozycji człowieka; nie deklarować zgodności IEC 60601-1 ani skuteczności klinicznej.

### Etap 1 — jeden formalny protokół
1. Zdefiniować wersjonowany protokół: nagłówek, długość, typ, sequence ID, CRC, ACK/NACK.
2. Określić maksymalne długości, timeouty, zachowanie po utracie łącza i zasady ponowienia.
3. Zdefiniować stany maszyny sterującej i autoryzację każdej komendy.
4. Wspólną specyfikację wykorzystać w Androidzie, desktopie, WebUI i firmware.
5. Dodać testy zgodności oparte o wektory testowe.

### Etap 2 — sieć firmware
1. Zintegrować realną, sprawdzoną bibliotekę ENC28J60.
2. Odczytywać rzeczywisty stan linku i konfiguracji IP.
3. Wprowadzić odbiór, parsowanie i walidację pakietów bez blokowania pętli.
4. Dodać obsługę błędów, timeoutów, przepełnień buforów i restartu.
5. Nie deklarować CONNECTED na podstawie samego powodzenia stubu.

### Etap 3 — Android
1. Wprowadzić jawne stany: DISCONNECTED, CONNECTING, CONNECTED_UNVERIFIED, READY, BUSY, FAULT, SAFE_LOCKOUT.
2. Każda operacja ma sprawdzać ACK i wynik.
3. Dodać trwałe, poprawne wznawianie sesji albo usunąć wznowienie, które zaczyna od zera.
4. Zapewnić best-effort STOP i niezależny sprzętowy fail-safe.
5. Zastąpić mock bazy częstotliwości rzeczywistym, wersjonowanym źródłem danych.

### Etap 4 — bezpieczeństwo sprzętowe
1. Zewnętrzne czujniki temperatury w krytycznych punktach.
2. Niezależny tor odcięcia energii, działający także przy zawieszeniu MCU.
3. Fizyczny E-stop z diagnostyką obwodu i bezpiecznym stanem domyślnym.
4. Walidacja prądu, napięcia, temperatury i stanu izolacji.
5. Analiza awarii, testy pojedynczych uszkodzeń i walidacja laboratoryjna.
6. Niezależna ocena zgodności z wymaganiami właściwymi dla docelowego wyrobu.

### Etap 5 — jakość i CI
1. Dodać build firmware i testy jednostkowe dla parsera/protokołu.
2. Dodać testy fuzz dla parserów ramek i komend.
3. Dodać testy awarii połączenia i restartów.
4. Dodać testy HIL dla wyjść na obciążeniu pomiarowym, bez człowieka.
5. Ustawić kryteria wydania: wszystkie testy przechodzą, brak błędów P0/P1, artefakty podpisane i wersjonowane.

## 7. Kryteria akceptacji

Repozytorium nie powinno zostać uznane za gotowe, dopóki:
- firmware realnie inicjalizuje ENC28J60;
- Android i firmware wymieniają zgodne, wersjonowane ramki;
- błędy i timeouty zawsze prowadzą do bezpiecznego stanu;
- UI nigdy nie raportuje sukcesu bez ACK;
- stan wyjść można potwierdzić niezależnym pomiarem;
- fizyczny tor awaryjny działa niezależnie od aplikacji i głównej pętli;
- wszystkie platformy przechodzą CI;
- dokumentacja odpowiada temu, co faktycznie zaimplementowano.

---
**Podsumowanie:** Największym problemem nie jest obecnie sama synchronizacja socketów, lecz brak rzeczywistego, wspólnego toru komunikacyjnego i niezależnie zweryfikowanej warstwy bezpieczeństwa. Najpierw należy doprowadzić do prawdziwej komunikacji i jawnych stanów, potem walidować sterowanie na stanowisku laboratoryjnym. Nie należy włączać wyjść na podstawie samego testu aplikacji.
