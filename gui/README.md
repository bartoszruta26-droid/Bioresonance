# ResoNet Desktop GUI — diagnostyka protokołu v1

Desktop GUI (C++17, SDL2, Dear ImGui) używa klienta TCP z negocjacją HELLO i odczytem STATUS v1. Połączenie jest uznawane za diagnostyczne dopiero po poprawnej odpowiedzi urządzenia.

## Funkcje
- HELLO_REQ / HELLO_RESP podczas łączenia
- STATUS_REQ / STATUS_RESP po połączeniu i cyklicznie
- Walidacja nagłówka, długości, sequence i CRC-16/CCITT-FALSE
- Jawne prezentowanie brakujących pomiarów i nieznanego stanu blokady wyjść

## Sterowanie
Klient nie wysyła surowych poleceń tekstowych ani pakietów terapii. API `sendCommand` i `sendTherapyPacket` jest fail-closed. Konfiguracja końcówek i sterowanie wyjściami nie są częścią protokołu v1.

## Budowanie — Linux
```bash
sudo apt-get install -y build-essential cmake libsdl2-dev
cmake -S gui -B build/gui -DCMAKE_BUILD_TYPE=Release
cmake --build build/gui --parallel
ctest --test-dir build/gui --output-on-failure
```

## Budowanie — Windows
Wymagane Visual Studio 2022 z obciążeniem C++ i SDL2 przez vcpkg. Szczegóły konfiguracji zależności są opisane w głównym pliku README.

## Granice bezpieczeństwa
CRC nie zapewnia uwierzytelniania, szyfrowania ani ochrony przed replay. Nie używaj protokołu w niezaufanej sieci. Odczyt diagnostyczny nie potwierdza bezpieczeństwa urządzenia ani wyjść.
