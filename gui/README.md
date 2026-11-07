# ResoNet Desktop GUI — Qt 6

Klient desktopowy dla Linux i Windows oparty na Qt 6 Widgets oraz Qt Charts.

## Moduły
- Pulpit, Pomiary, Kanały, Trening, Gry, Sesje, Historia, Raporty, Ustawienia.
- Źródła danych: symulator oraz import nagrania CSV; pochodzenie próbek jest zachowywane.
- Wykres szeregu czasowego z wyborem kanału, historia próbek i eksport CSV.
- Lokalny rejestr sesji w katalogu QStandardPaths::AppDataLocation.
- Tryb urządzenia jest jawnie ograniczony do diagnostyki; protokół v1 nie steruje wyjściami.

## Linux
```bash
sudo apt-get install -y build-essential cmake qt6-base-dev libqt6charts6-dev
cmake -S gui -B build/gui -DCMAKE_BUILD_TYPE=Release
cmake --build build/gui --parallel
ctest --test-dir build/gui --output-on-failure
```

## Windows
Wymagane Qt 6 z modułami Widgets i Charts oraz MSVC 2022:
```powershell
cmake -S gui -B build/gui -DCMAKE_PREFIX_PATH="$env:QT_ROOT_DIR"
cmake --build build/gui --config Release --parallel
ctest --test-dir build/gui -C Release --output-on-failure
```

Format importu CSV: timestamp,channel,value,unit,source.
Dane symulowane są demonstracyjne, nie stanowią pomiarów biologicznych ani diagnozy.
