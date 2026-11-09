# ResoNet Desktop GUI — Qt 6

Klient desktopowy dla Linux i Windows oparty na Qt 6 Widgets oraz Qt Charts.

## Moduły
- Pulpit, Pomiary, Kanały, Trening, Gry, Sesje, Historia, Raporty, Ustawienia.
- Źródła danych: symulator, import CSV oraz diagnostyka Arduino Nano przez USB (115200 bit/s). Pochodzenie próbek jest zachowywane.
- Wykres szeregu czasowego z wyborem kanału, historia próbek i eksport CSV.
- Lokalny rejestr sesji w katalogu QStandardPaths::AppDataLocation.
- Tryb urządzenia jest jawnie ograniczony do diagnostyki; protokół v1 nie steruje wyjściami.

## Linux
```bash
sudo apt-get install -y build-essential cmake qt6-base-dev libqt6charts6-dev libqt6serialport6-dev
cmake -S gui -B build/gui -DCMAKE_BUILD_TYPE=Release
cmake --build build/gui --parallel
ctest --test-dir build/gui --output-on-failure
```

## Pierwsze testy Arduino Nano przez USB (Debian 13)

1. Zainstaluj narzędzia i skompiluj firmware:

```bash
sudo apt install -y arduino-cli
arduino-cli core update-index
arduino-cli core install arduino:avr
bash ResoNet_Nano/build_nano.sh
arduino-cli board list
# Ustaw port znaleziony w poprzednim poleceniu:
PORT=/dev/ttyUSB0
RESONET_UPLOAD_PORT="$PORT" bash ResoNet_Nano/build_nano.sh
```

Skrypt kompiluje firmware z ustawieniami pamięci i wysyła ten sam obraz na wskazany port. Po otwarciu portu aplikacja odczekuje 2,2 s na automatyczny reset Nano.

3. Dodaj konto do grupy portu szeregowego i zaloguj się ponownie:

```bash
sudo usermod -aG dialout "$USER"
```

4. Zamknij Arduino Serial Monitor i inne programy używające portu. Uruchom `./build/gui/bioresonance_gui`, wybierz źródło `Arduino — USB (diagnostyka)`, wybierz port i kliknij `Połącz USB`.

5. Sprawdź stan firmware, czas pracy, wolną pamięć i flagi błędów. Odłącz przewód i sprawdź, czy aplikacja zgłosi utratę połączenia.

Protokół USB działa z parametrami 115200 bit/s, 8N1. Udostępnia tylko zapytania STATUS v1: nie przesyła próbek biologicznych ani nie steruje wyjściami. W pierwszej sesji pozostaw wszystkie efektory odłączone. Wartości temperatury, stan blokady fizycznej i flagi czujników są jawnie oznaczane jako niezweryfikowane.

## Windows
Wymagane Qt 6 z modułami Widgets i Charts oraz MSVC 2022:
```powershell
cmake -S gui -B build/gui -DCMAKE_PREFIX_PATH="$env:QT_ROOT_DIR"
cmake --build build/gui --config Release --parallel
ctest --test-dir build/gui -C Release --output-on-failure
```

Format importu CSV: timestamp,channel,value,unit,source.
Dane symulowane są demonstracyjne, nie stanowią pomiarów biologicznych ani diagnozy.
