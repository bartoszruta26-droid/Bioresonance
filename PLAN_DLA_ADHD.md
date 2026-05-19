# 🎯 ResoNet-Nano: Plan Pracy dla ADHD - Mikro-Kroki

> **💡 Jak używać tego planu:**
> - Każde zadanie zajmuje **5-15 minut**
> - Odhaczaj wykonane zadania ✅
> - Nie musisz robić wszystkiego naraz - **jeden mikro-krok dziennie to sukces!**
> - Jeśli coś jest za trudne - przeskocz do łatwiejszego
> - Świętuj małe zwycięstwa! 🎉

---

## 📅 Tydzień 1: Przygotowanie i Organizacja

### Dzień 1: Poznanie projektu
- [ ] **Zadanie 1.1** (3 min): Otwórz folder `/workspace` w menedżerze plików
- [ ] **Zadanie 1.2** (5 min): Przeczytaj tylko nagłówki z `README.md`
- [ ] **Zadanie 1.3** (2 min): Zrób zdjęcie/zrzut ekranu struktury folderów
- [ ] **Zadanie 1.4** (5 min): Napisz w notatniku 3 zdania: "Co ten projekt robi?"

### Dzień 2: Sprawdzenie co już działa
- [ ] **Zadanie 2.1** (5 min): Otwórz plik `IMPLEMENTATION_SUMMARY.md`
- [ ] **Zadanie 2.2** (3 min): Znajdź sekcję z ✅ (zrobione) i ⚠️ (do zrobienia)
- [ ] **Zadanie 2.3** (5 min): Policz ile jest ✅ a ile ⚠️ - zapisz liczby
- [ ] **Zadanie 2.4** (2 min): Pochwal się sobie: "Już wiem co jest zrobione!"

### Dzień 3: Przygotowanie środowiska - KROK 1
- [ ] **Zadanie 3.1** (5 min): Otwórz terminal
- [ ] **Zadanie 3.2** (3 min): Wpisz `cd /workspace` i naciśnij Enter
- [ ] **Zadanie 3.3** (5 min): Wpisz `ls -la` i zobacz listę plików
- [ ] **Zadanie 3.4** (2 min): Zrób screenshot terminala

### Dzień 4: Przygotowanie środowiska - KROK 2
- [ ] **Zadanie 4.1** (5 min): Sprawdź czy masz git: `git --version`
- [ ] **Zadanie 4.2** (3 min): Jeśli nie ma - zainstaluj: `sudo apt install git -y`
- [ ] **Zadanie 4.3** (5 min): Sprawdź status git: `git status`
- [ ] **Zadanie 4.4** (2 min): Zapisz wynik w notatniku

### Dzień 5: Pierwszy mikro-błąd do naprawy
- [ ] **Zadanie 5.1** (5 min): Otwórz plik `bash_tui/bioresonance_tui.sh`
- [ ] **Zadanie 5.2** (3 min): Znajdź linię z `load_presets()` (Ctrl+F)
- [ ] **Zadanie 5.3** (5 min): Dodaj komentarz `# TODO: Sprawdzić działanie` nad funkcją
- [ ] **Zadanie 5.4** (2 min): Zapisz plik (Ctrl+S)

### Dzień 6: Nauka o presettach
- [ ] **Zadanie 6.1** (5 min): Otwórz plik `therapeutic_presets.md`
- [ ] **Zadanie 6.2** (5 min): Przeczytaj pierwsze 5 presetów
- [ ] **Zadanie 6.3** (3 min): Wybierz jeden który Cię interesuje
- [ ] **Zadanie 6.4** (2 min): Zapisz jego nazwę w notatniku

### Dzień 7: Odpoczynek i refleksja
- [ ] **Zadanie 7.1** (5 min): Otwórz wszystkie otwarte pliki
- [ ] **Zadanie 7.2** (5 min): Zobacz co zrobiłeś w tym tygodniu
- [ ] **Zadanie 7.3** (3 min): Napisz: "Jestem z siebie dumny bo..."
- [ ] **Zadanie 7.4** (2 min): Zaplanuj kolejny tydzień

---

## 📅 Tydzień 2: Praca z bash_tui (Najłatwiejszy moduł)

### Dzień 8: Uruchomienie TUI
- [ ] **Zadanie 8.1** (3 min): Otwórz terminal w `/workspace/bash_tui`
- [ ] **Zadanie 8.2** (3 min): Wpisz `chmod +x bioresonance_tui.sh`
- [ ] **Zadanie 8.3** (5 min): Wpisz `./bioresonance_tui.sh` 
- [ ] **Zadanie 8.4** (4 min): Zrób screenshot działania

### Dzień 9: Testowanie menu presetów
- [ ] **Zadanie 9.1** (3 min): Uruchom `bioresonance_tui.sh`
- [ ] **Zadanie 9.2** (5 min): Naciśnij klawisz **P** (presety)
- [ ] **Zadanie 9.3** (3 min): Zobacz listę presetów
- [ ] **Zadanie 9.4** (4 min): Wyjdź z programu (q lub Ctrl+C)

### Dzień 10: Dodanie nowego presetu - PRZYGOTOWANIE
- [ ] **Zadanie 10.1** (5 min): Otwórz `therapeutic_presets.md`
- [ ] **Zadanie 10.2** (5 min): Skopiuj cały wpis jednego presetu
- [ ] **Zadanie 10.3** (3 min): Wklej go na końcu pliku
- [ ] **Zadanie 10.4** (2 min): Zmień nazwę na "MOJ_PRESET_TEST"

### Dzień 11: Dodanie nowego presetu - EDYCJA
- [ ] **Zadanie 11.1** (5 min): Zmień częstotliwość na 100Hz
- [ ] **Zadanie 11.2** (5 min): Zmień czas trwania na 5 minut
- [ ] **Zadanie 11.3** (3 min): Dodaj opis "Testowy preset ADHD"
- [ ] **Zadanie 11.4** (2 min): Zapisz plik

### Dzień 12: Sprawdzenie czy preset działa
- [ ] **Zadanie 12.1** (3 min): Uruchom `bioresonance_tui.sh`
- [ ] **Zadanie 12.2** (5 min): Naciśnij **P** i znajdź swój preset
- [ ] **Zadanie 12.3** (5 min): Sprawdź czy wyświetla się poprawnie
- [ ] **Zadanie 12.4** (2 min): Jeśli tak - zrób radośniejszy taniec! 💃

### Dzień 13: Naprawa błędu w TUI
- [ ] **Zadanie 13.1** (5 min): Otwórz `bash_tui/bioresonance_tui.sh`
- [ ] **Zadanie 13.2** (5 min): Znajdź funkcję `show_preset_details()`
- [ ] **Zadanie 13.3** (3 min): Dodaj `echo "DEBUG: Checking preset"` na początku
- [ ] **Zadanie 13.4** (2 min): Zapisz i przetestuj

### Dzień 14: Podsumowanie bash_tui
- [ ] **Zadanie 14.1** (5 min): Wypisz 3 rzeczy które działają
- [ ] **Zadanie 14.2** (5 min): Wypisz 1 rzecz która nie działa
- [ ] **Zadanie 14.3** (3 min): Zrób commit: `git add . && git commit -m "Test TUI"`
- [ ] **Zadanie 14.4** (2 min): Pochwal się na Discord/GitHub

---

## 📅 Tydzień 3: Częstotliwości i Baza Danych

### Dzień 15: Poznanie frequencies.md
- [ ] **Zadanie 15.1** (3 min): Otwórz plik `frequencies.md`
- [ ] **Zadanie 15.2** (5 min): Zobacz ile ma linii: `wc -l frequencies.md`
- [ ] **Zadanie 15.3** (5 min): Znajdź częstotliwość dla "ból głowy"
- [ ] **Zadanie 15.4** (2 min): Zapisz ją w notatniku

### Dzień 16: Dodanie nowej częstotliwości
- [ ] **Zadanie 16.1** (5 min): Otwórz `frequencies.md`
- [ ] **Zadanie 16.2** (5 min): Znajdź ostatnią częstotliwość
- [ ] **Zadanie 16.3** (3 min): Dodaj nową: `| ADHD_FOCUS | 432Hz | Koncentracja |`
- [ ] **Zadanie 16.4** (2 min): Zapisz plik

### Dzień 17: Sprawdzenie formatowania
- [ ] **Zadanie 17.1** (5 min): Otwórz `frequencies.md` w edytorze
- [ ] **Zadanie 17.2** (5 min): Sprawdź czy wszystkie kolumny są wyrównane
- [ ] **Zadanie 17.3** (3 min): Popraw jedną krzywą kolumnę
- [ ] **Zadanie 17.4** (2 min): Zapisz zmiany

### Dzień 18: Integracja z bash_tui
- [ ] **Zadanie 18.1** (5 min): Otwórz `bash_tui/bioresonance_tui.sh`
- [ ] **Zadanie 18.2** (5 min): Znajdź gdzie ładuje częstotliwości
- [ ] **Zadanie 18.3** (3 min): Dodaj `echo "Ładowanie frequencies.md"`
- [ ] **Zadanie 18.4** (2 min): Przetestuj uruchomienie

### Dzień 19: Filtrowanie częstotliwości
- [ ] **Zadanie 19.1** (5 min): W `bioresonance_tui.sh` znajdź listę częstotliwości
- [ ] **Zadanie 19.2** (5 min): Dodaj filtr po nazwie (grep)
- [ ] **Zadanie 19.3** (3 min): Przetestuj filtrowanie
- [ ] **Zadanie 19.4** (2 min): Zapisz zmiany

### Dzień 20: Eksport częstotliwości
- [ ] **Zadanie 20.1** (5 min): Stwórz nowy plik `moje_czestotliwosci.txt`
- [ ] **Zadanie 20.2** (5 min): Wypisz 5 ulubionych częstotliwości
- [ ] **Zadanie 20.3** (3 min): Dodaj opisy dlaczego je wybrałeś
- [ ] **Zadanie 20.4** (2 min): Zapisz plik

### Dzień 21: Weekendowy przegląd
- [ ] **Zadanie 21.1** (10 min): Przejdź przez wszystkie zmiany z tygodnia
- [ ] **Zadanie 21.2** (5 min): Zrób backup: `cp frequencies.md frequencies_backup.md`
- [ ] **Zadanie 21.3** (5 min): Zrób screenshot postępów
- [ ] **Zadanie 21.4** (5 min): Nagrodź się czymś miłym! 🍕

---

## 📅 Tydzień 4: Arduino Nano - Pierwsze Kroki

### Dzień 22: Poznanie struktury Arduino
- [ ] **Zadanie 22.1** (3 min): Otwórz folder `ResoNet_Nano/`
- [ ] **Zadanie 22.2** (5 min): Policz ile jest plików `.cpp`
- [ ] **Zadanie 22.3** (5 min): Otwórz `ResoNet_Nano.ino`
- [ ] **Zadanie 22.4** (2 min): Znajdź funkcję `setup()`

### Dzień 23: Analiza pwm_engine.cpp
- [ ] **Zadanie 23.1** (5 min): Otwórz `pwm_engine.cpp`
- [ ] **Zadanie 23.2** (5 min): Przeczytaj komentarze na górze pliku
- [ ] **Zadanie 23.3** (3 min): Znajdź funkcję `setFrequency()`
- [ ] **Zadanie 23.4** (2 min): Zapisz w notatniku co robi

### Dzień 24: Dodanie komentarza
- [ ] **Zadanie 24.1** (5 min): Otwórz `pwm_engine.cpp`
- [ ] **Zadanie 24.2** (5 min): Znajdź linijkę z `ICR1 =`
- [ ] **Zadanie 24.3** (3 min): Dodaj komentarz wyjaśniający co to jest
- [ ] **Zadanie 24.4** (2 min): Zapisz plik

### Dzień 25: Bezpieczeństwo - safety_system.cpp
- [ ] **Zadanie 25.1** (5 min): Otwórz `safety_system.cpp`
- [ ] **Zadanie 25.2** (5 min): Znajdź funkcję `checkTemperature()`
- [ ] **Zadanie 25.3** (3 min): Dodaj `// TODO: Testować na hardware`
- [ ] **Zadanie 25.4** (2 min): Zapisz zmiany

### Dzień 26: Sieć - network_system.cpp
- [ ] **Zadanie 26.1** (5 min): Otwórz `network_system.cpp`
- [ ] **Zadanie 26.2** (5 min): Znajdź port UDP (szukaj "5000")
- [ ] **Zadanie 26.3** (3 min): Zmień komentarz o porcie
- [ ] **Zadanie 26.4** (2 min): Zapisz plik

### Dzień 27: Logowanie - logging_system.cpp
- [ ] **Zadanie 27.1** (5 min): Otwórz `logging_system.cpp`
- [ ] **Zadanie 27.2** (5 min): Znajdź funkcję `logEvent()`
- [ ] **Zadanie 27.3** (3 min): Dodaj nowy poziom logu "DEBUG_ADHD"
- [ ] **Zadanie 27.4** (2 min): Zapisz zmiany

### Dzień 28: Kompilacja online (bez hardware)
- [ ] **Zadanie 28.1** (5 min): Wejdź na https://www.tinkercad.com/
- [ ] **Zadanie 28.2** (10 min): Załóż darmowe konto
- [ ] **Zadanie 28.3** (5 min): Stwórz nowy projekt Circuits
- [ ] **Zadanie 28.4** (5 min): Wrzuć kod Arduino i zobacz czy kompiluje

---

## 📅 Tydzień 5: WebUI - Strona WWW

### Dzień 29: Otwarcie WebUI
- [ ] **Zadanie 29.1** (3 min): Otwórz folder `webui/`
- [ ] **Zadanie 29.2** (5 min): Otwórz `index.html` w przeglądarce
- [ ] **Zadanie 29.3** (5 min): Kliknij kilka przycisków
- [ ] **Zadanie 29.4** (2 min): Zrób screenshot

### Dzień 30: Analiza HTML
- [ ] **Zadanie 30.1** (5 min): Otwórz `index.html` w edytorze
- [ ] **Zadanie 30.2** (5 min): Znajdź `<title>` i zmień na "Moje Biorezonans"
- [ ] **Zadanie 30.3** (3 min): Zapisz plik
- [ ] **Zadanie 30.4** (2 min): Odśwież przeglądarkę

### Dzień 31: Dodanie własnego przycisku
- [ ] **Zadanie 31.1** (5 min): W `index.html` znajdź sekcję z przyciskami
- [ ] **Zadanie 31.2** (5 min): Skopiuj jeden przycisk `<button>`
- [ ] **Zadanie 31.3** (3 min): Wklej i zmień tekst na "TEST ADHD"
- [ ] **Zadanie 31.4** (2 min): Zapisz i odśwież

### Dzień 32: JavaScript - prosta zmiana
- [ ] **Zadanie 32.1** (5 min): W `index.html` znajdź `<script>`
- [ ] **Zadanie 32.2** (5 min): Dodaj `console.log("ADHD mode on");`
- [ ] **Zadanie 32.3** (3 min): Otwórz DevTools (F12) i zobacz konsolę
- [ ] **Zadanie 32.4** (2 min): Zapisz zmiany

### Dzień 33: API PHP
- [ ] **Zadanie 33.1** (5 min): Otwórz `webui/api.php`
- [ ] **Zadanie 33.2** (5 min): Znajdź funkcję `getFrequencies()`
- [ ] **Zadanie 33.3** (3 min): Dodaj komentarz jak to działa
- [ ] **Zadanie 33.4** (2 min): Zapisz plik

### Dzień 34: Stylowanie CSS
- [ ] **Zadanie 34.1** (5 min): W `index.html` znajdź `<style>`
- [ ] **Zadanie 34.2** (5 min): Zmień kolor tła na swój ulubiony
- [ ] **Zadanie 34.3** (3 min): Zapisz i odśwież
- [ ] **Zadanie 34.4** (2 min): Ciesz się kolorem! 🎨

### Dzień 35: Testowanie na telefonie
- [ ] **Zadanie 35.1** (5 min): Skopiuj plik `index.html` na telefon
- [ ] **Zadanie 35.2** (5 min): Otwórz w przeglądarce mobilnej
- [ ] **Zadanie 35.3** (5 min): Sprawdź czy wygląda OK
- [ ] **Zadanie 35.4** (5 min): Zrób screenshot z telefonu

---

## 📅 Tydzień 6: Android App - Kotlin

### Dzień 36: Poznanie struktury Android
- [ ] **Zadanie 36.1** (3 min): Otwórz folder `android_app/`
- [ ] **Zadanie 36.2** (5 min): Otwórz `app/` i zobacz co tam jest
- [ ] **Zadanie 36.3** (5 min): Znajdź plik `.kt` (Kotlin)
- [ ] **Zadanie 36.4** (2 min): Otwórz pierwszy plik `.kt`

### Dzień 37: FrequencyDatabase.kt
- [ ] **Zadanie 37.1** (5 min): Otwórz `FrequencyDatabase.kt`
- [ ] **Zadanie 37.2** (5 min): Znajdź listę częstotliwości
- [ ] **Zadanie 37.3** (3 min): Dodaj swoją częstotliwość
- [ ] **Zadanie 37.4** (2 min): Zapisz zmiany

### Dzień 38: TherapyPresets
- [ ] **Zadanie 38.1** (5 min): Otwórz plik z presetami
- [ ] **Zadanie 38.2** (5 min): Znajdź preset "BONE_HEALING"
- [ ] **Zadanie 38.3** (3 min): Skopiuj go jako "ADHD_FOCUS"
- [ ] **Zadanie 38.4** (2 min): Zmień parametry

### Dzień 39: MainActivity
- [ ] **Zadanie 39.1** (5 min): Otwórz `MainActivity.kt`
- [ ] **Zadanie 39.2** (5 min): Znajdź `onCreate()`
- [ ] **Zadanie 39.3** (3 min): Dodaj `Log.d("ADHD", "App started")`
- [ ] **Zadanie 39.4** (2 min): Zapisz plik

### Dzień 40: Build.gradle
- [ ] **Zadanie 40.1** (5 min): Otwórz `build.gradle`
- [ ] **Zadanie 40.2** (5 min): Znajdź wersję Androida
- [ ] **Zadanie 40.3** (3 min): Dodaj komentarz o wymaganiach
- [ ] **Zadanie 40.4** (2 min): Zapisz zmiany

### Dzień 41: README Androida
- [ ] **Zadanie 41.1** (5 min): Otwórz `android_app/README.md`
- [ ] **Zadanie 41.2** (5 min): Dodaj sekcję "🧠 Tryb ADHD"
- [ ] **Zadanie 41.3** (3 min): Opisz jak używać w trybie skupienia
- [ ] **Zadanie 41.4** (2 min): Zapisz plik

### Dzień 42: Kompilacja online
- [ ] **Zadanie 42.1** (5 min): Wejdź na https://replit.com/
- [ ] **Zadanie 42.2** (10 min): Załóż konto i stwórz projekt Android
- [ ] **Zadanie 42.3** (5 min): Spróbuj wrzucić kod
- [ ] **Zadanie 42.4** (5 min): Zobacz czy kompiluje

---

## 📅 Tydzień 7: GUI Desktop - C++ ImGui

### Dzień 43: Struktura GUI
- [ ] **Zadanie 43.1** (3 min): Otwórz folder `gui/`
- [ ] **Zadanie 43.2** (5 min): Otwórz `CMakeLists.txt`
- [ ] **Zadanie 43.3** (5 min): Znajdź nazwę projektu
- [ ] **Zadanie 43.4** (2 min): Zmień na "ResoNet-ADHD"

### Dzień 44: Główny plik src
- [ ] **Zadanie 44.1** (5 min): Otwórz folder `src/`
- [ ] **Zadanie 44.2** (5 min): Otwórz główny plik `.cpp`
- [ ] **Zadanie 44.3** (3 min): Znajdź pętlę główną
- [ ] **Zadanie 44.4** (2 min): Dodaj komentarz

### Dzień 45: Include header
- [ ] **Zadanie 45.1** (5 min): Otwórz folder `include/`
- [ ] **Zadanie 45.2** (5 min): Otwórz plik nagłówkowy
- [ ] **Zadanie 45.3** (3 min): Dodaj nową deklarację funkcji
- [ ] **Zadanie 45.4** (2 min): Zapisz zmiany

### Dzień 46: imgui_lib
- [ ] **Zadanie 46.1** (5 min): Otwórz folder `imgui_lib/`
- [ ] **Zadanie 46.2** (5 min): Policz pliki
- [ ] **Zadanie 46.3** (3 min): Otwórz jeden i zobacz co to jest
- [ ] **Zadanie 46.4** (2 min): Zamknij i idź na kawę ☕

### Dzień 47: CMake konfiguracja
- [ ] **Zadanie 47.1** (5 min): Otwórz `CMakeLists.txt`
- [ ] **Zadanie 47.2** (5 min): Znajdź sekcję z bibliotekami
- [ ] **Zadanie 47.3** (3 min): Dodaj komentarz co każda robi
- [ ] **Zadanie 47.4** (2 min): Zapisz plik

### Dzień 48: README GUI
- [ ] **Zadanie 48.1** (5 min): Otwórz `gui/README.md`
- [ ] **Zadanie 48.2** (5 min): Dodaj sekcję "Budowa dla początkujących"
- [ ] **Zadanie 48.3** (3 min): Napisz 3 kroki instalacji
- [ ] **Zadanie 48.4** (2 min): Zapisz zmiany

### Dzień 49: Próba kompilacji
- [ ] **Zadanie 49.1** (5 min): Otwórz terminal w `gui/`
- [ ] **Zadanie 49.2** (5 min): Wpisz `mkdir build && cd build`
- [ ] **Zadanie 49.3** (5 min): Wpisz `cmake ..`
- [ ] **Zadanie 49.4** (5 min): Zobacz co się dzieje (nie musi działać!)

---

## 📅 Tydzień 8: Dokumentacja i Porządki

### Dzień 50: Aktualizacja IMPLEMENTATION_SUMMARY
- [ ] **Zadanie 50.1** (5 min): Otwórz `IMPLEMENTATION_SUMMARY.md`
- [ ] **Zadanie 50.2** (5 min): Znajdź coś co zrobiłeś
- [ ] **Zadanie 50.3** (3 min): Zmień ⚠️ na ✅ przy tej rzeczy
- [ ] **Zadanie 50.4** (2 min): Zapisz i ciesz się postępem!

### Dzień 51: Tworzenie checklisty osobistej
- [ ] **Zadanie 51.1** (5 min): Stwórz plik `MOJE_POSTEPY.md`
- [ ] **Zadanie 51.2** (10 min): Wypisz wszystko co już umiesz
- [ ] **Zadanie 51.3** (5 min): Dodaj daty kiedy czegoś się nauczyłeś
- [ ] **Zadanie 51.4** (5 min): Pochwal się sobie!

### Dzień 52: Czyszczenie plików tymczasowych
- [ ] **Zadanie 52.1** (5 min): Znajdź pliki `.backup` lub `.old`
- [ ] **Zadanie 52.2** (5 min): Usuń jeden niepotrzebny plik
- [ ] **Zadanie 52.3** (3 min): Sprawdź `git status`
- [ ] **Zadanie 52.4** (2 min): Zrób commit czyszczenia

### Dzień 53: Dodanie emoji do kodu
- [ ] **Zadanie 53.1** (5 min): Otwórz dowolny plik `.sh` lub `.cpp`
- [ ] **Zadanie 53.2** (5 min): Dodaj emoji w komentarzach 🎯
- [ ] **Zadanie 53.3** (3 min): Zapisz zmiany
- [ ] **Zadanie 53.4** (2 min): Uśmiechnij się 😊

### Dzień 54: Sprawdzenie .gitignore
- [ ] **Zadanie 54.1** (5 min): Otwórz `.gitignore`
- [ ] **Zadanie 54.2** (5 min): Zrozum co jest ignorowane
- [ ] **Zadanie 54.3** (3 min): Dodaj komentarz dlaczego
- [ ] **Zadanie 54.4** (2 min): Zapisz plik

### Dzień 55: Tworzenie szybkiego startu
- [ ] **Zadanie 55.1** (5 min): Stwórz plik `SZYBKI_START_ADHD.md`
- [ ] **Zadanie 55.2** (10 min): Napisz 5 najprostszych kroków
- [ ] **Zadanie 55.3** (5 min): Dodaj emoji i kolory
- [ ] **Zadanie 55.4** (5 min): Zapisz dla siebie na przyszłość

### Dzień 56: Wielkie podsumowanie 8 tygodni
- [ ] **Zadanie 56.1** (10 min): Otwórz wszystkie swoje pliki
- [ ] **Zadanie 56.2** (10 min): Policz ile zadań odhaczyłeś ✅
- [ ] **Zadanie 56.3** (10 min): Napisz list do siebie: "Jestem dumny bo..."
- [ ] **Zadanie 56.4** (15 min): Zrób coś fajnego dla siebie! 🎉

---

## 🔌 ELEKTRONIKA - Ścieżka Hardware

> **⚠️ UWAGA BEZPIECZEŃSTWO:** Pracujesz z urządzeniem które będzie dotykać twojej skóry. Zachowaj szczególną ostrożność!

## 📅 Tydzień 9: Elektronika - Przygotowanie i Bezpieczeństwo

### Dzień 57: Poznanie komponentów
- [ ] **Zadanie 57.1** (5 min): Wypisz wszystkie części z dokumentacji (Arduino, przewody, itp.)
- [ ] **Zadanie 57.2** (5 min): Zrób zdjęcie/wyszukaj w internecie jak wygląda Arduino Nano
- [ ] **Zadanie 57.3** (3 min): Zapisz w notatniku do czego służy każda część
- [ ] **Zadanie 57.4** (2 min): Pochwal się że już wiesz co to jest mikrokontroler! 🎉

### Dzień 58: Bezpieczeństwo pracy z elektroniką
- [ ] **Zadanie 58.1** (5 min): Przeczytaj o napięciu bezpiecznym dla ludzi (<50V DC)
- [ ] **Zadanie 58.2** (5 min): Zapisz zasadę: "Nigdy nie podłączaj do sieci 230V!"
- [ ] **Zadanie 58.3** (3 min): Sprawdź czy masz apteczkę pierwszą pomoc
- [ ] **Zadanie 58.4** (2 min): Przygotuj suche, nieprzewodzące miejsce do pracy

### Dzień 59: Narzędzia - multimetr
- [ ] **Zadanie 59.1** (5 min): Dowiedz się co to jest multimetr
- [ ] **Zadanie 59.2** (5 min): Obejrzyj film na YouTube "jak używać multimetra"
- [ ] **Zadanie 59.3** (3 min): Zapisz 3 funkcje multimetra (napięcie, prąd, rezystancja)
- [ ] **Zadanie 59.4** (2 min): Jeśli masz multimetr - obejrzyj go i sprawdź baterię

### Dzień 60: Narzędzia - lutownica (TEORIA)
- [ ] **Zadanie 60.1** (5 min): Dowiedz się czym jest lutowanie
- [ ] **Zadanie 60.2** (5 min): Obejrzyj film "bezpieczne lutowanie dla początkujących"
- [ ] **Zadanie 60.3** (3 min): Zapisz zasady: wentylacja, podstawka, nie dotykać grota!
- [ ] **Zadanie 60.4** (2 min): Zaplanuj gdzie będziesz lutować (blisko okna!)

### Dzień 61: Źródła zasilania
- [ ] **Zadanie 61.1** (5 min): Dowiedz się co to jest bateria Li-Ion 18650
- [ ] **Zadanie 61.2** (5 min): Zapisz napięcie tej baterii (3.7V nominalnie)
- [ ] **Zadanie 61.3** (3 min): Dowiedz się co to jest moduł ładowania TP4056
- [ ] **Zadanie 61.4** (2 min): Zrozum zasadę: "Nie zwarć baterii!" 🔋

### Dzień 62: Przewody i złącza
- [ ] **Zadanie 62.1** (5 min): Poznaj różnice między przewodami sygnałowymi a zasilającymi
- [ ] **Zadanie 62.2** (5 min): Dowiedz się co to są zworki (jumpers)
- [ ] **Zadanie 62.3** (3 min): Zapisz kolory: czerwony = plus, czarny = minus
- [ ] **Zadanie 62.4** (2 min): Jeśli masz przewody - posortuj je po kolorach

### Dzień 63: Planowanie układu
- [ ] **Zadanie 63.1** (10 min): Narysuj odręcznie schemat połączeń
- [ ] **Zadanie 63.2** (5 min): Zaznacz na schemacie Arduino Nano
- [ ] **Zadanie 63.3** (3 min): Zaznacz gdzie będą elektrody
- [ ] **Zadanie 63.4** (2 min): Schowaj rysunek w widocznym miejscu

---

## 📅 Tydzień 10: Elektronika - Lutowanie i Połączenia

### Dzień 64: Ćwiczenie lutowania - ZŁOM
- [ ] **Zadanie 64.1** (5 min): Znajdź stary sprzęt do rozebrania (stara płytka PCB)
- [ ] **Zadanie 64.2** (5 min): Rozlutuj jeden element ze starej płytki
- [ ] **Zadanie 64.3** (5 min): Wylutuj go całkowicie
- [ ] **Zadanie 64.4** (5 min): Pochwal się pierwszym lutowaniem! 🔥

### Dzień 65: Przygotowanie stanowiska
- [ ] **Zadanie 65.1** (5 min): Oczyść stół z materiałów palnych
- [ ] **Zadanie 65.2** (5 min): Postaw szklankę z wodą obok
- [ ] **Zadanie 65.3** (3 min): Przygotuj gąbkę do czyszczenia grota
- [ ] **Zadanie 65.4** (2 min): Włącz wentylację/otwórz okno

### Dzień 66: Pierwsze luty - ćwiczenie
- [ ] **Zadanie 66.1** (5 min): Weź kawałek przewodu i płytkę perforowaną
- [ ] **Zadanie 66.2** (5 min): Rozgrzej lutownicę (poczekaj 2-3 minuty)
- [ ] **Zadanie 66.3** (5 min): Przylutuj jeden koniec przewodu
- [ ] **Zadanie 66.4** (5 min): Oceń czy lut jest błyszczący i gładki

### Dzień 67: Arduino Nano - pinout
- [ ] **Zadanie 67.1** (5 min): Wyszukaj w Google "Arduino Nano pinout"
- [ ] **Zadanie 67.2** (5 min): Zapisz które piny to GND (masa)
- [ ] **Zadanie 67.3** (3 min): Zapisz które piny to 5V i 3.3V
- [ ] **Zadanie 67.4** (2 min): Wydrukuj lub zapisz pinout przy stanowisku

### Dzień 68: Podłączenie zasilania Arduino
- [ ] **Zadanie 68.1** (5 min): Przygotuj przewód czerwony i czarny
- [ ] **Zadanie 68.2** (5 min): Przylutuj czerwony do pinu 5V (TYLKO DO TESTÓW!)
- [ ] **Zadanie 68.3** (5 min): Przylutuj czarny do GND
- [ ] **Zadanie 68.4** (5 min): NIE PODŁĄCZAJ JESZCZE ZASILANIA!

### Dzień 69: Sprawdzenie połączeń
- [ ] **Zadanie 69.1** (5 min): Weź multimetr (lub poproś kogoś kto ma)
- [ ] **Zadanie 69.2** (5 min): Ustaw na ciągłość (symbol diody/dźwięku)
- [ ] **Zadanie 69.3** (5 min): Sprawdź czy nie ma zwarcia między 5V a GND
- [ ] **Zadanie 69.4** (5 min): Jeśli multimetr piszczy - popraw luty!

### Dzień 70: Izolowanie połączeń
- [ ] **Zadanie 70.1** (5 min): Przygotuj rurki termokurczliwe lub taśmę izolacyjną
- [ ] **Zadanie 70.2** (5 min): Załóż rurkę na każdy lut
- [ ] **Zadanie 70.3** (5 min): Podgrzej zapalniczką (ostrożnie!) lub użyj taśmy
- [ ] **Zadanie 70.4** (5 min): Sprawdź czy nic nie wystaje

---

## 📅 Tydzień 11: Elektronika - Moduł PWM i Elektrody

### Dzień 71: Zrozumienie PWM
- [ ] **Zadanie 71.1** (5 min): Dowiedz się co to jest PWM (modulacja szerokości impulsu)
- [ ] **Zadanie 71.2** (5 min): Obejrzyj animację jak działa PWM
- [ ] **Zadanie 71.3** (3 min): Zapisz że PWM imituje różne napięcia
- [ ] **Zadanie 71.4** (2 min): Zrozum że to generuje "częstotliwość"

### Dzień 72: Pin PWM w Arduino
- [ ] **Zadanie 72.1** (5 min): Sprawdź które piny Arduino mają PWM (oznaczone ~)
- [ ] **Zadanie 72.2** (5 min): Wybierz jeden pin PWM (np. D3 lub D11)
- [ ] **Zadanie 72.3** (3 min): Zapisz który pin wybrałeś
- [ ] **Zadanie 72.4** (2 min): Przygotuj przewód do tego pinu

### Dzień 73: Lutowanie wyjścia PWM
- [ ] **Zadanie 73.1** (5 min): Przylutuj przewód do wybranego pinu PWM
- [ ] **Zadanie 73.2** (5 min): Drugi koniec podłącz do śrubki/złącza
- [ ] **Zadanie 73.3** (5 min): Zaizoluj połączenie
- [ ] **Zadanie 73.4** (5 min): Opisz markerem "WYJŚCIE SYGNAŁU"

### Dzień 74: Przygotowanie elektrod
- [ ] **Zadanie 74.1** (5 min): Dowiedz się jakie elektrody są bezpieczne (żelowe, karbonowe)
- [ ] **Zadanie 74.2** (5 min): NIGDY nie używaj gołych drutów miedzianych na skórę!
- [ ] **Zadanie 74.3** (3 min): Zamów lub znajdź elektrody medyczne
- [ ] **Zadanie 74.4** (2 min): Przygotuj przewody z wtyczkami do elektrod

### Dzień 75: Bezpieczne podłączenie elektrod
- [ ] **Zadanie 75.1** (5 min): Użyj złączy typu "krokodylki" lub wtyczek
- [ ] **Zadanie 75.2** (5 min): Podłącz jeden przewód do wyjścia PWM
- [ ] **Zadanie 75.3** (5 min): Drugi przewód podłącz do GND
- [ ] **Zadanie 75.4** (5 min): Sprawdź czy połączenia są pewne

### Dzień 76: Rezystor ograniczający prąd
- [ ] **Zadanie 76.1** (5 min): Dowiedz się dlaczego potrzebny jest rezystor
- [ ] **Zadanie 76.2** (5 min): Oblicz minimalną rezystancję (R = U/I, max 20mA)
- [ ] **Zadanie 76.3** (3 min): Dobierz rezystor np. 220Ω lub 330Ω
- [ ] **Zadanie 76.4** (2 min): Wlutuj rezystor szeregowo z wyjściem

### Dzień 77: Test bez podłączania do ciała
- [ ] **Zadanie 77.1** (5 min): Przygotuj diodę LED i rezystor 220Ω
- [ ] **Zadanie 77.2** (5 min): Podłącz do wyjścia PWM i GND
- [ ] **Zadanie 77.3** (5 min): Wgraj prosty kod z PWM
- [ ] **Zadanie 77.4** (5 min): Zobacz czy dioda świeci/sygnalizuje

---

## 📅 Tydzień 12: Elektronika - Testy i Pomiar

### Dzień 78: Pierwszy program testowy
- [ ] **Zadanie 78.1** (5 min): Otwórz Arduino IDE
- [ ] **Zadanie 78.2** (5 min): Wklej prosty kod: `analogWrite(3, 128)`
- [ ] **Zadanie 78.3** (5 min): Skompiluj i wgraj do Arduino
- [ ] **Zadanie 78.4** (5 min): Zobacz czy dioda z dnia 77 reaguje

### Dzień 79: Pomiar napięcia multimetrem
- [ ] **Zadanie 79.1** (5 min): Ustaw multimetr na pomiar napięcia DC
- [ ] **Zadanie 79.2** (5 min): Podłącz sondy do wyjścia i GND
- [ ] **Zadanie 79.3** (5 min): Odczytaj wartość (powinno być ~2.5V przy PWM 50%)
- [ ] **Zadanie 79.4** (5 min): Zapisz wynik w notatniku

### Dzień 80: Pomiar częstotliwości
- [ ] **Zadanie 80.1** (5 min): Jeśli multimetr ma Hz - ustaw na pomiar częstotliwości
- [ ] **Zadanie 80.2** (5 min): Podłącz do wyjścia PWM
- [ ] **Zadanie 80.3** (5 min): Odczytaj częstotliwość (Arduino PWM to ~490Hz)
- [ ] **Zadanie 80.4** (5 min): Porównaj z oczekiwaniami z kodu

### Dzień 81: Regulacja wypełnienia PWM
- [ ] **Zadanie 81.1** (5 min): Zmień w kodzie wartość z 128 na 64
- [ ] **Zadanie 81.2** (5 min): Wgraj i zmierz napięcie
- [ ] **Zadanie 81.3** (5 min): Zmień na 255 i zmierz ponownie
- [ ] **Zadanie 81.4** (5 min): Zapisz zależność: większa wartość = większe napięcie

### Dzień 82: Test z potencjometrem
- [ ] **Zadanie 82.1** (5 min): Podłącz potencjometr do analogowego pinu A0
- [ ] **Zadanie 82.2** (5 min): Napisz kod czytający A0 i sterujący PWM
- [ ] **Zadanie 82.3** (5 min): Kręć potencjometrem i obserwuj diodę
- [ ] **Zadanie 82.4** (5 min): Ciesz się że sterujesz analogowo! 🎛️

### Dzień 83: Bezpieczny test na sobie (OSTROŻNIE!)
- [ ] **Zadanie 83.1** (10 min): PRZECZYTAJ jeszcze raz o bezpieczeństwie
- [ ] **Zadanie 83.2** (5 min): Upewnij się że używasz ZASILANIA BATERYJNEGO (nie USB!)
- [ ] **Zadanie 83.3** (5 min): Dotknij elektrodami suchą skórę na przedramieniu
- [ ] **Zadanie 83.4** (5 min): Powinieneś czuć delikatne mrowienie (jeśli nie - OK!)

### Dzień 84: Dokumentacja układu
- [ ] **Zadanie 84.1** (10 min): Zrób zdjęcie całego układu
- [ ] **Zadanie 84.2** (5 min): Opisz każde połączenie na zdjęciu
- [ ] **Zadanie 84.3** (5 min): Zapisz jakie wartości rezystorów użyłeś
- [ ] **Zadanie 84.4** (5 min): Stwórz plik `ELEKTRONIKA_INSTRUKCJA.md`

---

## 📅 Tydzień 13: Integracja Hardware + Software

### Dzień 85: Podłączenie pod bash_tui
- [ ] **Zadanie 85.1** (5 min): Otwórz `bash_tui/bioresonance_tui.sh`
- [ ] **Zadanie 85.2** (5 min): Znajdź gdzie wysyła dane do Arduino
- [ ] **Zadanie 85.3** (3 min): Dodaj komunikat "Łączenie z hardware..."
- [ ] **Zadanie 85.4** (2 min): Zapisz zmiany

### Dzień 86: Komunikacja Serial
- [ ] **Zadanie 86.1** (5 min): Dowiedz się co to jest komunikacja UART/Serial
- [ ] **Zadanie 86.2** (5 min): Sprawdź który port to Arduino (`ls /dev/ttyUSB*`)
- [ ] **Zadanie 86.3** (3 min): Zapisz nazwę portu (np. `/dev/ttyUSB0`)
- [ ] **Zadanie 86.4** (2 min): Dodaj numer portu do konfiguracji TUI

### Dzień 87: Wysłanie pierwszej komendy
- [ ] **Zadanie 87.1** (5 min): W TUI dodaj opcję "Test hardware"
- [ ] **Zadanie 87.2** (5 min): Niech wysyła komendę "FREQ:1000" przez Serial
- [ ] **Zadanie 87.3** (5 min): W Arduino dodaj odbiór tej komendy
- [ ] **Zadanie 87.4** (5 min): Zobacz czy dioda reaguje

### Dzień 88: Odbiór danych z Arduino
- [ ] **Zadanie 88.1** (5 min): W Arduino dodaj wysyłanie statusu "OK"
- [ ] **Zadanie 88.2** (5 min): W bash_tui dodaj odbiór odpowiedzi
- [ ] **Zadanie 88.3** (3 min): Wyświetl "Hardware odpowiada!" w TUI
- [ ] **Zadanie 88.4** (2 min): Przetestuj całość

### Dzień 89: Presety na hardware
- [ ] **Zadanie 89.1** (5 min): Wybierz jeden preset z listy
- [ ] **Zadanie 89.2** (5 min): W TUI po wybraniu presetu wyślij freq do Arduino
- [ ] **Zadanie 89.3** (5 min): Arduino niech ustawi odpowiednie PWM
- [ ] **Zadanie 89.4** (5 min): Sprawdź na diodzie/ciele czy działa

### Dzień 90: Timer w hardware
- [ ] **Zadanie 90.1** (5 min): W Arduino dodaj odliczanie czasu terapii
- [ ] **Zadanie 90.2** (5 min): Po upływie czasu wyłącz PWM automatycznie
- [ ] **Zadanie 90.3** (3 min): Wyślij informację "KONIEC" do TUI
- [ ] **Zadanie 90.4** (2 min): TUI niech wyświetli "Terapia zakończona"

### Dzień 91: Bezpieczne wyłączanie awaryjne
- [ ] **Zadanie 91.1** (5 min): Dodaj przycisk fizyczny do układu (STOP)
- [ ] **Zadanie 91.2** (5 min): Podłącz do pinu z przerwaniami
- [ ] **Zadanie 91.3** (5 min): Naciśnij przycisk - PWM ma natychmiast zgasnąć
- [ ] **Zadanie 91.4** (5 min): Przetestuj działanie awaryjne

### Dzień 92: Wielkie podsumowanie elektroniki
- [ ] **Zadanie 92.1** (10 min): Zrób zdjęcie działającego urządzenia
- [ ] **Zadanie 92.2** (10 min): Napisz listę WSZYSTKICH rzeczy których się nauczyłeś
- [ ] **Zadanie 92.3** (5 min): Zaktualizuj `IMPLEMENTATION_SUMMARY.md` o hardware ✅
- [ ] **Zadanie 92.4** (5 min): **WIELKA NAGRODA** za opanowanie elektroniki! 🏆

---

## 📅 Tydzień 14: Finał i Prezentacja

### Dzień 93: Przygotowanie prezentacji
- [ ] **Zadanie 93.1** (10 min): Stwórz 5 slajdów o projekcie
- [ ] **Zadanie 93.2** (5 min): Slajd 1: Co to jest ResoNet-Nano
- [ ] **Zadanie 93.3** (5 min): Slajd 2: Twoje postępy z ADHD
- [ ] **Zadanie 93.4** (5 min): Slajd 3: Zdjęcia hardware

### Dzień 94: Nagranie demo
- [ ] **Zadanie 94.1** (10 min): Nagraj telefonem działające TUI
- [ ] **Zadanie 94.2** (10 min): Nagraj działający hardware
- [ ] **Zadanie 94.3** (5 min): Zmontuj w 1-minutowe wideo
- [ ] **Zadanie 94.4** (5 min): Dodaj napisy

### Dzień 95: Publikacja na GitHub
- [ ] **Zadanie 95.1** (10 min): Wrzuć wszystkie zdjęcia do repo
- [ ] **Zadanie 95.2** (5 min): Zaktualizuj README o sekcję hardware
- [ ] **Zadanie 95.3** (5 min): Dodaj link do wideo demo
- [ ] **Zadanie 95.4** (5 min): Opublikuj jako release v1.0

### Dzień 96: Podziękowania dla siebie
- [ ] **Zadanie 96.1** (15 min): Napisz list do siebie z początku projektu
- [ ] **Zadanie 96.2** (5 min): Wypisz 10 umiejętności które zdobyłeś
- [ ] **Zadanie 96.3** (5 min): Zrób certyfikat ukończenia dla siebie
- [ ] **Zadanie 96.4** (5 min): Powiedz sobie "JESTEM Z SIEBIE DUMNY"

### Dzień 97: Planowanie kolejnych kroków
- [ ] **Zadanie 97.1** (10 min): Zastanów się co chcesz dodać dalej
- [ ] **Zadanie 97.2** (5 min): Stwórz listę "Chcę kiedyś zrobić..."
- [ ] **Zadanie 97.3** (5 min): Wybierz JEDNĄ rzecz na następny miesiąc
- [ ] **Zadanie 97.4** (5 min): Nie spiesz się - odpocznij!

### Dzień 98: Pomoc innym
- [ ] **Zadanie 98.1** (10 min): Napisz poradnik "Jak zacząć z ADHD"
- [ ] **Zadanie 98.2** (5 min): Udostępnij na Reddit r/ADHD
- [ ] **Zadanie 98.3** (5 min): Odpowiedz na jedno pytanie innego osoby
- [ ] **Zadanie 98.4** (5 min): Ciesz się że pomagasz! 🤝

### Dzień 99: Refleksja
- [ ] **Zadanie 99.1** (15 min): Przeczytaj cały PLAN_DLA_ADHD.md od początku
- [ ] **Zadanie 99.2** (5 min): Zobacz ile checkboxów masz odhaczonych
- [ ] **Zadanie 99.3** (5 min): Zauważ jak bardzo urósłeś
- [ ] **Zadanie 99.4** (5 min): Przytul siebie (naprawdę!)

### Dzień 100: ŚWIĘTOWANIE! 🎉
- [ ] **Zadanie 100.1** (30 min): Zrób coś specjalnego dla siebie
- [ ] **Zadanie 100.2** (10 min): Zjedz ulubione jedzenie
- [ ] **Zadanie 100.3** (10 min): Kup sobie mały prezent
- [ ] **Zadanie 100.4** (∞ min): **BĄDŹ DUMNY!** Ukończyłeś projekt! 🌟

---

## 📅 Tydzień 9+: Rozszerzenia i Fun Features

### Pomysły na kolejne mikro-kroki:

#### Funkcje Specjalne ADHD
- [ ] Dodaj tryb "FOCUS_MODE" z konkretnymi częstotliwościami
- [ ] Stwórz preset "PORANNE_PRZEBUDZENIE"
- [ ] Dodaj timer przypominający o przerwach
- [ ] Zrób listę "ULUBIONE_CZESTOTLIWOSCI"
- [ ] Dodaj dźwiękowe powiadomienia (gdy hardware gotowy)

#### Ulepszenia Interfejsu
- [ ] Zmień kolory w WebUI na bardziej kontrastowe
- [ ] Dodaj większe przyciski w Android
- [ ] Stwórz tryb "UPROSZCZONY" z mniej opcjami
- [ ] Dodaj wizualny timer odliczający
- [ ] Zrób dark mode wszędzie

#### Dokumentacja
- [ ] Nagraj 30-sekundowe wideo z działania
- [ ] Stwórz diagram jak to działa (może być ręcznie!)
- [ ] Napisz FAQ dla osób z ADHD
- [ ] Dodaj tłumaczenie na angielski
- [ ] Stwórz galerię screenshotów

#### Hardware (jak już masz Arduino)
- [ ] Podłącz Arduino do komputera
- [ ] Wgraj firmware przez USB
- [ ] Testuj diodę LED zamiast anteny
- [ ] Mierz multimetrem wyjście
- [ ] Podłącz prawdziwą cewkę

---

## 🏆 System Nagród

### Po każdym zadaniu:
- ✅ Odhacz na liście
- ✅ Powiedz sobie "Dobra robota!"
- ✅ Zrób 3 głębokie oddechy

### Po każdym tygodniu:
- 🎁 Kup sobie małą nagrodę (kawa, słodycz, gra)
- 🎁 Zrób zdjęcie postępów
- 🎁 Opowiedz komuś o swoich osiągnięciach

### Po każdym miesiącu:
- 🎉 Większa nagroda (książka, gadżet, wyjście)
- 🎉 Zaktualizuj portfolio/GitHub
- 🎉 Pomóż komuś innemu zacząć

---

## 💡 Wskazówki dla ADHD

### Kiedy brakuje motywacji:
1. **Zasada 2 minut**: "Zrobię tylko przez 2 minuty" - często zostaniesz dłużej
2. **Body doubling**: Włącz film na YouTube z kimś kto pracuje
3. **Zmień zadanie**: Jeśli jedno nie idzie, zrób inne z listy
4. **Nagroda natychmiastowa**: Daj sobie coś miłego PO każdym zadaniu

### Kiedy przytłacza cię skala:
1. **Patrz tylko na DZIŚ**: Nie myśl o całym projekcie
2. **Jedno pudełko naraz**: Fokus na jeden folder/moduł
3. **Timer**: Ustaw 10 minut i rób dopóki nie zadzwoni
4. **Perfekcjonizm jest wrogiem**: Byle działo, nie musi być idealne

### Kiedy zapominasz co robiłeś:
1. **Git commits**: Rób małe commity z opisem
2. **Notatnik obok**: Zapisuj co właśnie zrobiłeś
3. **Screenshoty**: Rób zdjęcia ekranu po każdej sesji
4. **MOJE_POSTEPY.md**: Prowadź ten plik na bieżąco

---

## 📞 Gdzie szukać pomocy?

- **GitHub Issues**: Pytania o projekt
- **Discord**: Społeczność open source hardware
- **Reddit**: r/ADHD_Programmers
- **Lokalny makerspace**: Warsztaty hardware

---

## ✨ Pamiętaj!

> **"Nie musisz widzieć całych schodów. Wystarczy zrobić pierwszy krok."** - Martin Luther King Jr.

Każdy, nawet najmniejszy krok się liczy. Jeden dzień z mikro-zadaniem to więcej niż zero. Jesteś wystarczająco dobry dokładnie taki jaki jesteś! 🌟

---

*Ten plan został stworzony specjalnie dla osób z ADHD. Możesz go modyfikować, pomijać dni, zmieniać kolejność - najważniejsze że DZIAŁASZ!*
