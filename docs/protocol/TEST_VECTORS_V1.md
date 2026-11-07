# Wektory testowe ResoNet Protocol v1 — wariant status-only

## STATUS_REQ, sequence=1

Nagłówek + payload: `52 4E 01 05 00 00 01 00 00`

CRC-16/CCITT-FALSE wyliczane jest z powyższych 9 bajtów. Pełna ramka jest weryfikowana przez wspólne testy hostowe (firmware i desktop), które porównują enkoder z dekoderem.

## STATUS_RESP

- Typ: `0x06`
- Payload: dokładnie 14 bajtów.
- Sequence odpowiedzi musi być identyczne z sequence żądania.
- Maksymalna długość ramki: 25 bajtów.

## Negatywne przypadki

1. Nieprawidłowy magic, wersja lub flags — odrzucić.
2. Typ inny niż STATUS_REQ w kierunku klient → Nano — odrzucić.
3. Typ inny niż STATUS_RESP w kierunku Nano → klient — odrzucić.
4. Payload STATUS_REQ różny od zera bajtów — odrzucić.
5. Payload STATUS_RESP różny od 14 bajtów — odrzucić.
6. Błędne CRC — odrzucić bez wywołania handlera.
7. Niekompletna ramka — zresetować parser po timeout.
8. Dwie ramki w jednym odczycie — obsłużyć niezależnie.
9. Ramka podzielona pomiędzy odczyty TCP — poprawnie złożyć.
10. Usunięte typy HELLO, PING, ACK, NACK i ERROR — odrzucić.
