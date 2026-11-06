# Integracja firmware — ResoNet Protocol v1

## Stan implementacji

Wprowadzono niezależny moduł `protocol_v1.h/.cpp` do kodowania ramek, obliczania CRC-16/CCITT-FALSE i przyrostowego parsowania strumienia bajtów.

## Ograniczenia projektu

- Parser nie używa sterty, `String`, STL ani blokujących operacji.
- Pojedyncza instancja parsera przechowuje maksymalnie jedną ramkę (203 bajty).
- Handler otrzymuje widok payloadu ważny wyłącznie podczas wywołania zwrotnego; nie wolno zachowywać wskaźnika po powrocie.
- Parser wywołuje handler wyłącznie po walidacji wersji, flag, typu, długości, CRC.
- Moduł nie zawiera typów ani funkcji uruchamiania efektorów.
- Nieznany typ jest odrzucany lokalnie; warstwa transportowa może w przyszłości wygenerować NACK, ale musi zrobić to jawnie.
- `rejectedFrames()` jest 16-bitowym licznikiem diagnostycznym; może się zawinąć.

## Integracja z siecią

Moduł jest obecnie podłączony do diagnostycznego serwera TCP w `network_system.cpp`. Transport ENC28J60 jest domyślnie wyłączony w `network_config.h`; przed aktywacją trzeba skonfigurować adresację i przypiąć EthernetENC 2.0.5.

Następny etap transportowy powinien:
1. dodać rzeczywistą bibliotekę ENC28J60 i jawnie skonfigurowany adres sieciowy;
2. utworzyć jedno gniazdo TCP diagnostyczne;
3. przekazywać każdy odebrany bajt do `Parser::feed()`;
4. obsłużyć wyłącznie HELLO_REQ, PING i STATUS_REQ;
5. odpowiedzi budować przez `encode()`, z tym samym numerem sequence;
6. nie wywoływać żadnych funkcji PWM, IR LED, piezo ani wibratora;
7. rozłączać klienta po błędach/timeoutach bez blokowania pętli bezpieczeństwa.

## Weryfikacja

Nie przeprowadzono testów sprzętowych. Samo zapisanie plików nie oznacza, że firmware zostało skompilowane lub zweryfikowane na docelowym toolchainie.
