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

1. Zainstaluj Arduino CLI i rdzeń AVR, podłącz Nano, a następnie sprawdź wykryty port:

```bash
sudo apt install -y arduino-cli
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli lib install EthernetENC@2.0.5
arduino-cli board list
```

2. Nadaj kontu uprawnienia do portu szeregowego i zaloguj się ponownie (albo uruchom nową sesję użytkownika):

```bash
sudo usermod -aG dialout "$USER"
```

3. Wgraj firmware. Zastąp przykładową ścieżkę portem wykrytym przez `arduino-cli board list`:

```bash
PORT=/dev/ttyUSB0
RESONET_UPLOAD_PORT="$PORT" bash ResoNet_Nano/build_nano.sh
```

Skrypt kompiluje firmware z ustawieniami pamięci i wgrywa ten sam obraz na płytkę. Domyślny profil to Nano z bootloaderem ATmega328P old; dla nowszego bootloadera użyj `RESONET_FQBN=arduino:avr:nano:cpu=atmega328`.

4. Zamknij Arduino Serial Monitor i inne programy używające portu. Zbuduj i uruchom aplikację:

```bash
sudo apt-get install -y build-essential cmake qt6-base-dev libqt6charts6-dev libqt6serialport6-dev
cmake -S gui -B build/gui -DCMAKE_BUILD_TYPE=Release
cmake --build build/gui --parallel
./build/gui/bioresonance_gui
```

Wybierz źródło `Arduino — USB (diagnostyka)`, port i kliknij `Połącz USB`. Aplikacja odczekuje 2,2 s na automatyczny reset Nano po otwarciu portu.

5. Sprawdź stan firmware, czas pracy, wolną pamięć i flagi błędów. Odłącz przewód i sprawdź, czy aplikacja zgłosi utratę połączenia. Ponownie podłącz przewód i sprawdź ponowne połączenie.

### Protokół pierwszego testu na żywo

- Test wykonuj najpierw na samej płytce Nano, bez podłączonych efektorów, elektrod, anten, generatorów ani obciążeń.
- Płytkę zasilaj wyłącznie przez USB komputera. Nie łącz równocześnie zewnętrznych źródeł zasilania bez sprawdzenia wspólnej masy i schematu połączeń.
- Po starcie oczekuj stanu firmware, czasu pracy i wolnej pamięci. Czas pracy powinien rosnąć przy kolejnych odpowiedziach.
- Sprawdź zachowanie po odłączeniu i ponownym podłączeniu USB oraz po ponownym uruchomieniu aplikacji.
- Zapisz wynik: model Nano/układ USB-UART, port, FQBN/bootloader, wersję firmware (commit), wynik połączenia, stan, wolną pamięć, timeouty i zachowanie po odłączeniu.

Nie uruchamiaj generatorów ani testów wyjść na pacjencie lub podłączonym osprzęcie. Aktualny transport USB jest tylko diagnostyczny; nie dostarcza próbek biofeedback i nie steruje wyjściami. Firmware pozostaje eksperymentalny, a status programowy nie jest potwierdzeniem fizycznego odłączenia wyjść.

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
