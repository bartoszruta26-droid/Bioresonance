# ResoNet WebUI — diagnostyka protokołu v1

WebUI udostępnia osobny, odczytowy panel diagnostyczny. Nie używa starego tekstowego API i nie wysyła konfiguracji, START/STOP ani poleceń wyjść.

## Wymagania
- Apache lub inny serwer PHP
- PHP 7.4+ z rozszerzeniem `sockets`
- Firmware ResoNet Protocol v1 dostępny pod TCP/5001
- HTTPS w przypadku dostępu przez przeglądarkę

## Instalacja
Skopiuj `index.html` (lub `diagnostics.html`) oraz `diagnostics.php` do tego samego katalogu serwera. Skonfiguruj zmienną środowiskową procesu PHP:

```bash
php -r 'echo bin2hex(random_bytes(32)), PHP_EOL;'
```

Wygenerowaną wartość ustaw jako `BIORESONANCE_API_TOKEN` w konfiguracji Apache/PHP-FPM. Nie zapisuj tokenu w repozytorium. Udostępnij panel przez HTTPS i ogranicz dostęp do zaufanych administratorów.

## Użycie
Otwórz `index.html` (lub `diagnostics.html`), podaj prywatny adres IPv4 urządzenia i token API. Panel obsługuje:
- HELLO_REQ / HELLO_RESP podczas każdej sesji
- STATUS_REQ / STATUS_RESP
- PING / PONG

Backend używa stałego portu TCP 5001, odrzuca adresy spoza prywatnych zakresów RFC1918, weryfikuje token przez `hash_equals`, ogranicza czas odczytu/zapisu i sprawdza wersję, flags, długość, sequence oraz CRC każdej odpowiedzi.

## Bezpieczeństwo
Token jest wprowadzany przez operatora i pozostaje w pamięci bieżącej strony; nie jest zapisywany w localStorage. Nie używaj niezabezpieczonego HTTP. CRC wykrywa przypadkowe uszkodzenia, ale nie uwierzytelnia urządzenia ani nie chroni przed replay. Panel nie oferuje żadnych funkcji sterowania wyjściami.

## Wyłączony stary interfejs
`api.php` zwraca HTTP 410 Gone i nie wykonuje żadnych połączeń z urządzeniem. `index.html` jest aktualnym, odczytowym panelem v1; `diagnostics.html` jest jego równoważnym wejściem.
