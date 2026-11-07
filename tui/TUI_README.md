# C++ TUI — implementacja archiwalna

> **STATUS: LEGACY / NIE UŻYWAĆ DO ŁĄCZENIA Z URZĄDZENIEM.**
>
> Ten katalog zawiera historyczną implementację TUI w C++/ncurses. Kod korzysta z dawnego tekstowego modelu komend i pakietów sterujących, który nie jest zgodny z aktualnym, diagnostycznym ResoNet Protocol v1. Nie należy uruchamiać ani dystrybuować tej aplikacji jako aktualnego klienta ResoNet.

## Oficjalny interfejs terminalowy

Aktualnym klientem TUI jest Python:

- kod: `bash_tui/resonet_tui.py`
- uruchomienie: `bash_tui/bioresonance_tui.sh`
- testy: `bash_tui/test_resonet_tui.py`
- dokumentacja: `bash_tui/README.md`

Uruchomienie z katalogu głównego repozytorium:

```bash
python3 bash_tui/resonet_tui.py --help
```

Lub:

```bash
bash bash_tui/bioresonance_tui.sh --help
```

## Dlaczego C++ TUI jest archiwalne?

Historyczna implementacja zawiera ścieżki tekstowych komend i wysyłania pakietów terapii. Aktualny klient ResoNet Protocol v1 jest ograniczony do diagnostyki i nie udostępnia operacji konfiguracji wyjść, START ani STOP. Stary interfejs może więc prezentować lub sugerować operacje, których aktualny kontrakt komunikacyjny nie obsługuje.

## Zasady bezpieczeństwa

- Nie używać tego programu do sterowania wyjściami.
- Nie traktować lokalnych komunikatów UI jako potwierdzenia stanu urządzenia.
- Nie zakładać, że połączenie TCP oznacza zweryfikowaną gotowość sprzętu.
- Do diagnostyki używać wyłącznie aktualnego klienta Python i aktualnej specyfikacji protokołu.
- Urządzenie pozostaje prototypem eksperymentalnym; audyt interfejsu nie stanowi walidacji medycznej ani elektrycznej.

Kod C++ pozostaje w repozytorium wyłącznie jako materiał historyczny do czasu osobnej decyzji o jego usunięciu.
