# ResoNet-Nano GUI - Graficzny Interfejs Użytkownika

Aplikacja GUI dla systemu biorezonansu ResoNet-Nano, działająca na systemach Windows, Linux i macOS.

## Technologie

- **SDL2** - Biblioteka multimedialna zapewniająca obsługę okien i renderowanie
- **Dear ImGui** - Nowoczesny, natychmiastowy interfejs graficzny
- **C++17** - Standard C++ zapewniający przenośność kodu

## Funkcje

- 🖥️ **Panel sterowania końcówkami** - Konfiguracja 8 kanałów z różnymi typami aplikatorów
- 📊 **Przeglądarka częstotliwości** - Wyszukiwanie terapeutycznych częstotliwości z bazy danych
- 🔌 **Łączność sieciowa** - TCP/IP komunikacja z Arduino Nano przez Ethernet HAT
- 📝 **Logi zdarzeń** - Śledzenie wszystkich operacji w czasie rzeczywistym
- ⚙️ **Ustawienia terapii** - Kontrola częstotliwości, czasu, modulacji i intensywności

## Kompilacja

### Wymagania

#### Windows
- Visual Studio 2019 lub nowszy LUB MinGW-w64
- CMake 3.10+
- Git

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install build-essential cmake libsdl2-dev git
```

#### macOS
```bash
brew install cmake sdl2 git
```

### Budowanie

```bash
cd gui
mkdir build && cd build
cmake ..
cmake --build .
```

### Uruchamienie

```bash
# Windows
.\Debug\bioresonance_gui.exe

# Linux/macOS
./bioresonance_gui

# Z parametrami
./bioresonance_gui --ip 192.168.1.100 --port 5001
```

### Szczegóły Implementacji
- **Pliki źródłowe**: 
  - `src/gui_app.cpp` (612 linii) - Główna logika GUI
  - `src/network_client.cpp` (241 linii) - Komunikacja TCP/IP
  - `src/probe_manager.cpp` (149 linii) - Zarządzanie końcówkami
  - `src/frequency_loader.cpp` (125 linii) - Baza częstotliwości
  - `src/main.cpp` (293 linie) - Punkt wejścia
- **Pliki nagłówkowe**: 9 plików `.h` w katalogu `include/`
- **Technologie**: SDL2 + Dear ImGui
- **Standard**: C++17
- **Całkowita liczba linii**: ~2764 linii kodu C++

## Struktura projektu

```
gui/
├── CMakeLists.txt          # Konfiguracja budowania
├── include/
│   ├── types.h             # Wspólne typy danych
│   ├── network_client.h    # Klient sieciowy
│   ├── probe_manager.h     # Menadżer końcówek
│   ├── frequency_loader.h  # Ładowanie częstotliwości
│   └── gui_app.h           # Główna klasa aplikacji
└── src/
    ├── main.cpp            # Punkt wejścia
    ├── gui_app.cpp         # Implementacja GUI
    ├── network_client.cpp  # Komunikacja sieciowa
    ├── probe_manager.cpp   # Zarządzanie końcówkami
    └── frequency_loader.cpp # Baza częstotliwości
```

## Sterowanie

### Menu główne
- **Plik** - Połącz/Rozłącz/Wyjście
- **Widok** - Przełączanie paneli
- **Pomoc** - Informacje o programie

### Zakładki
1. **Panel główny** - Wybór i konfiguracja końcówek
2. **Częstotliwości** - Przeglądanie bazy terapeutycznej
3. **Status** - Informacje o połączeniu i logi

### Skróty klawiszowe
- `Ctrl+O` - Połącz z urządzeniem
- `Ctrl+D` - Rozłącz
- `Alt+F4` - Wyjście
- `Ctrl+C` - Zamknij aplikację (w terminalu)

## Kompatybilność

| System | Status | Uwagi |
|--------|--------|-------|
| Windows 10/11 | ✅ Testowany | Wymaga Winsock2 |
| Ubuntu 20.04+ | ✅ Testowany | X11 lub Wayland |
| macOS 11+ | ✅ Testowany | Metal backend |

## Rozwiązywanie problemów

### Problem: Nie można połączyć z urządzeniem
- Sprawdź czy Arduino jest podłączone do sieci
- Zweryfikuj adres IP w ustawieniach
- Upewnij się że port 5001 nie jest blokowany przez firewall

### Problem: Aplikacja nie uruchamia się
- Sprawdź czy biblioteki SDL2 są poprawnie zainstalowane
- Na Windows: zainstaluj [Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)
- Na Linux: `ldd ./bioresonance_gui` pokaże brakujące biblioteki

### Problem: Brak pliku frequencies.md
- Upewnij się że uruchamiasz aplikację z katalogu gui/build lub skopiuj plik frequencies.md do katalogu z aplikacją

## Porównanie z TUI

| Funkcja | TUI (ncurses) | GUI (ImGui) |
|---------|---------------|-------------|
| Interfejs | Terminalowy | Graficzny |
| Mysz | ❌ | ✅ |
| Skróty klawiszowe | ✅ | ✅ |
| Tryb pełnoekranowy | ✅ | ✅ |
| Okna dialogowe | ❌ | ✅ |
| Wykresy | ❌ | ✅ (możliwe) |
| Złożoność | Niska | Średnia |

## Licencja

MIT License - patrz główny plik LICENSE w repozytorium.

## Autor

ResoNet-Nano Team
