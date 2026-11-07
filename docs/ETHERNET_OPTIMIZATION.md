# Arduino Nano + ENC28J60 — optymalizacja pamięci

## Wynik kompilacji

Pomiar z GitHub Actions, pełny firmware ATmega328P 16 MHz z EthernetENC 2.0.5 i TCP:

| Wariant | Flash | RAM globalny |
|---|---:|---:|
| Stan przed optymalizacją | 30 308 B | 1 353 B |
| Stan po optymalizacji | **27 942 B** | **1 315 B** |
| Zmiana | **−2 366 B** | **−38 B** |

Limity kontrolne: Flash 28 800 B, RAM globalny 1 536 B. W aktualnym buildzie pozostaje 2 778 B fizycznej pamięci Flash i 733 B RAM na stos oraz zmienne lokalne. Wynik dotyczy kompilacji, nie pomiaru na fizycznym urządzeniu.

## Zmiany

1. Protokół diagnostyczny ma jeden typ żądania `STATUS_REQ` i jedną odpowiedź `STATUS_RESP`. Usunięto HELLO, PING/PONG, ACK, NACK i ERROR.
2. Maksymalny payload ograniczono do 14 B, a maksymalną ramkę do 25 B. Parser Nano używa stałego bufora.
3. EthernetENC jest budowany jako serwer TCP: UDP i aktywne połączenia wychodzące są wyłączone, liczba połączeń i portów nasłuchu wynosi jeden.
4. Skrypt kompilacji tymczasowo zastępuje niewykorzystywane metody klienta Ethernet wariantami zwracającymi błąd. Zachowuje wymagane symbole wirtualne i odtwarza oryginalny plik biblioteki przez `trap`, również po nieudanej kompilacji.
5. Odbiór TCP odbywa się porcjami do 24 B, zamiast wywoływania `read()` dla każdego bajtu.
6. Modulacja AM/FM używa arytmetyki całkowitoliczbowej zamiast `sin()` i operacji zmiennoprzecinkowych.
7. Pomiar NTC używa tablicy w PROGMEM z interpolacją całkowitą, zamiast obliczeń logarytmicznych zmiennoprzecinkowych.
8. Jakości biofeedbacku i pewność są liczone jako wartości całkowite 0–100%. Niezweryfikowany pomiar GSR pozostaje niedostępny i nie steruje adaptacją.

## Weryfikacja

Aktualny przebieg CI: [Firmware protocol host checks](https://github.com/bartoszruta26-droid/Bioresonance/actions/runs/42416299169).

- pełny firmware Nano bez Ethernetu — kompilacja przeszła;
- pełny firmware Nano z ENC28J60/TCP — kompilacja i limity pamięci przeszły;
- minimalny firmware diagnostyczny — kompilacja i limit pamięci przeszły;
- testy hostowe protokołu, kalkulatora Timer1, matematyki modulacji i runtime diagnostycznego — przeszły.

## Ograniczenia

- Tablica NTC jest interpolacją dotychczasowych współczynników Steinharta–Harta. Wymaga kalibracji z konkretnym termistorem, rezystorem odniesienia i rzeczywistym układem dzielnika.
- Wyłączenie metod `EthernetClient::connect()` jest celowe: Nano przyjmuje połączenia, nie inicjuje ich. Metody zwracają błąd; nie należy używać tej kompilacji jako klienta TCP.
- Protokół diagnostyczny nie steruje wyjściami i nie zapewnia uwierzytelnienia. Port TCP powinien pozostać w zaufanej sieci lokalnej.
- CI nie zastępuje testu na płytce, oscyloskopie, obciążeniu zastępczym ani walidacji sprzętowych zabezpieczeń.
