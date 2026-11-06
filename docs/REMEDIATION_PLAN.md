# Plan naprawczy — Bioresonance

Wszystkie zmiany prowadzone są na gałęzi audit/remediation; main pozostaje nietknięta.

## P1 — bezpieczeństwo i transport
1. Formalny protokół RNP z wersjonowaniem, długością ramki, endianowością i identyfikatorem żądania.
2. Rzeczywista obsługa ENC28J60; usunięcie funkcji stub i fałszywego statusu połączenia.
3. Serwer urządzenia i jednoznaczny kontrakt komend.
4. Walidacja zakresów, autoryzacja, limity czasu i ochrona przed powtórzeniem komend.
5. Sprzętowy stan bezpieczny oraz niezależne od MCU odcięcie energii.
6. Brak automatycznego wznowienia po utracie zasilania/łączności.
7. Analiza RAM, stosu, czasu wykonania i zachowania watchdog.

## P2 — spójność klientów
Wspólny model urządzenia, kanału, konfiguracji i sesji. Stany: DISCONNECTED, CONNECTING, IDLE, CONFIGURING, READY, STARTING, RUNNING, PAUSED, STOPPING, FAULT, SAFE_LOCKOUT. Klienci prezentują stan autorytatywny urządzenia.

## Raspberry Pi Pico
Oddzielny firmware RP2040 współdzielący specyfikację RNP, z osobnymi sterownikami i testami sprzętowymi. Nie kopiować kodu rejestrów ATmega328P.

## Testy i wydania
CI Windows/Linux dla Desktop GUI; testy jednostkowe protokołu i parserów; symulator urządzenia; testy integracyjne bez efektora; testy sprzętowe z obciążeniem i pomiarami.

## Desktop GUI — wymagania
Desktop GUI musi budować się na Windows 10/11 oraz współczesnym Linuxie. Wymagane CMake + C++17 + SDL2, Winsock2 na Windows i POSIX sockets na Linuxie. Każda zmiana w gui/ uruchamia macierz CI Windows/Linux.

## Ograniczenia
CI nie zastępuje testów fizycznego Arduino/Pico. Komunikacja, timery, wyjścia i odcięcie sprzętowe wymagają osobnych testów laboratoryjnych.
