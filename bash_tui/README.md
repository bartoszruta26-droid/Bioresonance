# ResoNet TUI — klient diagnostyczny protokołu v1

TUI komunikuje się z Arduino Nano wyłącznie przez binarny ResoNet Protocol v1. Implementacja jest klientem tylko do odczytu.

## Wymagania
- Python 3.9+
- Dostęp TCP do urządzenia na porcie 5001
- Firmware z włączonym transportem EthernetENC i protokołem v1

## Uruchomienie
```bash
chmod +x bioresonance_tui.sh
./bioresonance_tui.sh 192.168.1.177
# lub
python3 resonet_tui.py 192.168.1.177 --port 5001
```

Po poprawnym HELLO klient udostępnia:
- `S` — STATUS_REQ / STATUS_RESP
- `P` — PING / PONG
- `Q` — zakończenie

Każda odpowiedź jest sprawdzana pod względem magic, wersji, flags, długości, sequence i CRC-16/CCITT-FALSE. Obsługiwane są częściowe odczyty TCP.

## Granice bezpieczeństwa
Klient nie implementuje konfiguracji kanałów, START/STOP, PWM ani żadnego polecenia sterującego wyjściami. Wartość `output_inhibited=0` jest prezentowana jako nieznana, a nie jako potwierdzenie bezpieczeństwa. CRC nie zapewnia uwierzytelniania ani szyfrowania — używaj wyłącznie w zaufanej sieci lokalnej.

## Obsługa błędów

Błędy są prezentowane jako `BŁĄD [KOD]` z polskim komunikatem. Kod klasyfikuje problem niezależnie od tekstu, a flaga ponowienia odróżnia przerwanie transportu od błędu ramki lub niezgodności protokołu. Szczegóły i słownik wspólny dla interfejsów opisano w [kontrakcie obsługi błędów](../docs/ERROR_HANDLING_CONTRACT.md).

## Testy hosta
```bash
python3 -m unittest discover -s bash_tui -p 'test_*.py'
```
