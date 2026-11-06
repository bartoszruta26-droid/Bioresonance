# Wektory testowe ResoNet Protocol v1

**Status:** referencyjne dane testowe; protokół jest specyfikacją projektową i nie jest jeszcze wdrożony.

## Wektor 1 — HELLO_REQ, sequence=1

- Header + payload (hex): 52 4E 01 01 00 00 01 00 00
- CRC-16/CCITT-FALSE: 70 02
- Pełna ramka (hex): 52 4E 01 01 00 00 01 00 00 70 02
- Oczekiwany wynik: poprawna ramka, HELLO_REQ, sequence 1, payload length 0.

## Negatywne przypadki parsera

1. Zmieniony magic: odrzucić.
2. Version != 0x01: odrzucić bez dispatch.
3. Flags != 0: odrzucić.
4. Payload length = 193: odrzucić przed zapisem do bufora.
5. Brak CRC lub ucięta ramka: timeout i odrzucenie.
6. Błędne CRC: odrzucić bez wywołania handlera.
7. Nieznany typ z poprawnym nagłówkiem i CRC: NACK/UNSUPPORTED_TYPE.
8. Dwie ramki w jednym odczycie: przetworzyć obie.
9. Jedna ramka podzielona na bajty: złożyć bez utraty danych.
10. Sequence odpowiedzi niezgodny z oczekiwanym: odrzucić.

## Wymagane testy własnościowe

- Parser nigdy nie odczytuje poza statycznym buforem.
- Parser nigdy nie wywołuje handlera przed walidacją CRC.
- Pamięć parsera pozostaje ograniczona dla dowolnego strumienia wejściowego.
- Błędne wejście nie może zmienić stanu urządzenia ani aktywować wyjścia.
