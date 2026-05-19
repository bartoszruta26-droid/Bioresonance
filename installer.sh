#!/bin/bash

# Skrypt instalacyjny z menu
# Autor: Assistant
# Wersja: 2.0 - Z aktualizacją GitHub i poprawioną instalacją

# Kolory dla wyjścia
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Konfiguracja GitHub
GITHUB_REPO="https://github.com/bartoszruta26-droid/Bioresonance"
GITHUB_BRANCH="main"
INSTALL_DIR="/opt/bioresonance"

# Funkcja do czyszczenia ekranu
clear_screen() {
    clear
}

# Funkcja do wyświetlania nagłówka
show_header() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}       Skrypt Instalacyjny${NC}"
    echo -e "${CYAN}       Bioresonance v2.0${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
}

# Funkcja do wyświetlania menu
show_menu() {
    clear_screen
    show_header
    echo -e "${YELLOW}Wybierz opcję:${NC}"
    echo ""
    echo -e "  ${GREEN}1)${NC} Instalacja zależności systemowych"
    echo -e "  ${GREEN}2)${NC} Pobierz projekt z GitHub"
    echo -e "  ${GREEN}3)${NC} Instrukcja Bashtui"
    echo -e "  ${GREEN}4)${NC} Instrukcja Cpptui"
    echo -e "  ${GREEN}5)${NC} Instalacja i kompilacja GUI"
    echo -e "  ${GREEN}6)${NC} Instalacja WebUI (Apache + PHP)"
    echo -e "  ${GREEN}7)${NC} Instalacja AndroidApp"
    echo -e "  ${GREEN}8)${NC} Weryfikacja instalacji"
    echo -e "  ${GREEN}9)${NC} Wyjście"
    echo ""
    echo -n -e "${BLUE}Podaj numer opcji [1-9]: ${NC}"
}

# Funkcja do instalacji zależności
install_dependencies() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instalacja zależności systemowych...${NC}"
    echo ""
    
    # Sprawdzenie menedżera pakietów
    if command -v apt &> /dev/null; then
        echo -e "${BLUE}Wykryto apt. Instalowanie zależności...${NC}"
        sudo apt update
        sudo apt install -y build-essential git curl wget nodejs npm python3 python3-pip \
            apache2 php php-sockets php-curl libsdl2-dev cmake xorg-dev
    elif command -v yum &> /dev/null; then
        echo -e "${BLUE}Wykryto yum. Instalowanie zależności...${NC}"
        sudo yum update -y
        sudo yum install -y gcc gcc-c++ git curl wget nodejs npm python3 python3-pip \
            httpd php php-sockets php-curl SDL2-devel cmake
    elif command -v dnf &> /dev/null; then
        echo -e "${BLUE}Wykryto dnf. Instalowanie zależności...${NC}"
        sudo dnf update -y
        sudo dnf install -y gcc gcc-c++ git curl wget nodejs npm python3 python3-pip \
            httpd php php-sockets php-curl SDL2-devel cmake
    elif command -v pacman &> /dev/null; then
        echo -e "${BLUE}Wykryto pacman. Instalowanie zależności...${NC}"
        sudo pacman -Syu --noconfirm
        sudo pacman -S --noconfirm base-devel git curl wget nodejs npm python python-pip \
            apache php sdl2 cmake
    else
        echo -e "${RED}Nie wykryto znanego menedżera pakietów.${NC}"
        echo "Proszę ręcznie zainstalować niezbędne zależności."
    fi
    
    echo ""
    echo -e "${GREEN}Zakończono instalację zależności systemowych.${NC}"
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Funkcja do pobierania projektu z GitHub
download_from_github() {
    clear_screen
    show_header
    echo -e "${YELLOW}Pobieranie projektu z GitHub...${NC}"
    echo ""
    echo -e "${CYAN}Repozytorium: ${GITHUB_REPO}${NC}"
    echo -e "${CYAN}Gałąź: ${GITHUB_BRANCH}${NC}"
    echo -e "${CYAN}Katalog instalacji: ${INSTALL_DIR}${NC}"
    echo ""
    
    # Sprawdź czy katalog istnieje
    if [ -d "$INSTALL_DIR" ]; then
        echo -e "${YELLOW}Katalog ${INSTALL_DIR} już istnieje.${NC}"
        echo -n "Czy chcesz go usunąć i pobrać projekt ponownie? (t/n): "
        read -r response
        if [[ "$response" =~ ^[Tt]$ ]]; then
            sudo rm -rf "$INSTALL_DIR"
        else
            echo -e "${BLUE}Przechodzenie do istniejącego katalogu...${NC}"
            cd "$INSTALL_DIR" || return
            echo -e "${GREEN}Projekt jest już dostępny w ${INSTALL_DIR}${NC}"
            echo -n "Naciśnij Enter, aby kontynuować..."
            read
            return
        fi
    fi
    
    # Pobierz projekt - bez sudo, aby zachować uprawnienia użytkownika
    echo -e "${BLUE}Klonowanie repozytorium...${NC}"
    
    # Najpierw utwórz katalog z uprawnieniami użytkownika
    if ! sudo mkdir -p "$INSTALL_DIR"; then
        echo -e "${RED}Nie udało się utworzyć katalogu ${INSTALL_DIR}${NC}"
        return 1
    fi
    
    # Zmień właściciela na bieżącego użytkownika
    if ! sudo chown "$(id -un):$(id -gn)" "$INSTALL_DIR"; then
        echo -e "${RED}Nie udało się zmienić właściciela katalogu${NC}"
        sudo rm -rf "$INSTALL_DIR"
        return 1
    fi
    
    # Klonuj bez sudo - użytkownik będzie miał pełne prawa do zapisu
    if git clone "$GITHUB_REPO" "$INSTALL_DIR"; then
        echo ""
        echo -e "${GREEN}Pomyślnie pobrano projekt z GitHub!${NC}"
        echo ""
        echo -e "${CYAN}Struktura projektu:${NC}"
        ls -la "$INSTALL_DIR"
        echo ""
        echo -e "${BLUE}Dostępne komponenty:${NC}"
        echo "  - gui/          : Aplikacja GUI (C++/SDL2/ImGui)"
        echo "  - webui/        : Interfejs Web (PHP/Apache)"
        echo "  - android_app/  : Aplikacja Android (Kotlin)"
        echo "  - bash_tui/     : Interfejs TUI (Bash)"
        echo "  - tui/          : Interfejs TUI (C++)"
        echo "  - ResoNet_Nano/ : Firmware Arduino"
    else
        echo -e "${RED}Błąd podczas pobierania projektu z GitHub!${NC}"
        echo "Sprawdź połączenie z internetem i upewnij się, że repozytorium jest dostępne."
        # Wyczyść nieudany katalog
        sudo rm -rf "$INSTALL_DIR"
        return 1
    fi
    
    echo ""
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Funkcja z instrukcją Bashtui
instruction_bashtui() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instrukcja Bashtui${NC}"
    echo ""
    echo -e "${CYAN}Bashtui${NC} to biblioteka do tworzenia interfejsów TUI w Bashu."
    echo ""
    echo "Kroki instalacji:"
    echo "  1. Sklonuj repozytorium:"
    echo "     git clone https://github.com/username/bashtui.git"
    echo ""
    echo "  2. Przejdź do katalogu:"
    echo "     cd bashtui"
    echo ""
    echo "  3. Uruchom skrypt instalacyjny:"
    echo "     ./install.sh"
    echo ""
    echo "  4. Dodaj do PATH (opcjonalnie):"
    echo "     export PATH=\$PATH:/path/to/bashtui"
    echo ""
    echo "Przykład użycia:"
    echo '  source bashtui.sh'
    echo '  tui_menu "Title" "Option 1" "Option 2"'
    echo ""
    echo -e "${BLUE}Więcej informacji: https://github.com/username/bashtui${NC}"
    echo ""
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Funkcja z instrukcją Cpptui
instruction_cpptui() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instrukcja Cpptui${NC}"
    echo ""
    echo -e "${CYAN}Cpptui${NC} to biblioteka do tworzenia interfejsów TUI w C++."
    echo ""
    echo "Kroki instalacji:"
    echo "  1. Zainstaluj zależności:"
    echo "     sudo apt install libncurses5-dev libncursesw5-dev"
    echo ""
    echo "  2. Sklonuj repozytorium:"
    echo "     git clone https://github.com/username/cpptui.git"
    echo ""
    echo "  3. Przejdź do katalogu:"
    echo "     cd cpptui"
    echo ""
    echo "  4. Stwórz katalog build i przejdź do niego:"
    echo "     mkdir build && cd build"
    echo ""
    echo "  5. Uruchom cmake i make:"
    echo "     cmake .."
    echo "     make"
    echo ""
    echo "  6. Zainstaluj:"
    echo "     sudo make install"
    echo ""
    echo "Przykład użycia:"
    echo '  #include <cpptui.h>'
    echo '  int main() { /* kod aplikacji */ }'
    echo ""
    echo -e "${BLUE}Więcej informacji: https://github.com/username/cpptui${NC}"
    echo ""
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Funkcja do instalacji GUI - kompilacja z plików GitHub
install_gui() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instalacja i kompilacja GUI...${NC}"
    echo ""
    
    # Sprawdź czy katalog z projektem istnieje
    if [ ! -d "$INSTALL_DIR" ]; then
        echo -e "${RED}Projekt nie został pobrany! Najpierw wybierz opcję 2 (Pobierz projekt z GitHub).${NC}"
        echo ""
        echo -n "Czy chcesz pobrać projekt teraz? (t/n): "
        read -r response
        if [[ "$response" =~ ^[Tt]$ ]]; then
            download_from_github
        else
            echo -e "${YELLOW}Anulowano instalację GUI.${NC}"
            echo -n "Naciśnij Enter, aby kontynuować..."
            read
            return
        fi
    fi
    
    cd "$INSTALL_DIR/gui" || return
    
    echo -e "${BLUE}Sprawdzanie zależności dla GUI...${NC}"
    
    # Sprawdź czy SDL2 jest zainstalowany
    if ! pkg-config --exists sdl2 2>/dev/null; then
        echo -e "${YELLOW}SDL2 nie jest zainstalowane. Instalowanie...${NC}"
        if command -v apt &> /dev/null; then
            sudo apt update && sudo apt install -y libsdl2-dev cmake xorg-dev
        elif command -v yum &> /dev/null; then
            sudo yum install -y SDL2-devel cmake
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y SDL2-devel cmake
        elif command -v pacman &> /dev/null; then
            sudo pacman -S --noconfirm sdl2 cmake
        fi
    fi
    
    echo ""
    echo -e "${BLUE}Kompilowanie GUI z plików GitHub...${NC}"
    echo ""
    
    # Utwórz katalog build
    mkdir -p build
    cd build || return
    
    # Uruchom cmake i make
    echo -e "${CYAN}Uruchamianie cmake...${NC}"
    if cmake ..; then
        echo -e "${CYAN}Kompilowanie...${NC}"
        if cmake --build . -j$(nproc); then
            echo ""
            echo -e "${GREEN}Pomyślnie skompilowano aplikację GUI!${NC}"
            echo ""
            echo -e "${CYAN}Lokalizacja pliku wykonywalnego:${NC}"
            echo "  $INSTALL_DIR/gui/build/bioresonance_gui"
            echo ""
            echo -e "${BLUE}Aby uruchomić:${NC}"
            echo "  cd $INSTALL_DIR/gui/build"
            echo "  ./bioresonance_gui"
        else
            echo -e "${RED}Błąd podczas kompilacji!${NC}"
            echo "Sprawdź logi powyżej, aby uzyskać więcej informacji."
            echo -n "Naciśnij Enter, aby kontynuować..."
            read
            return 1
        fi
    else
        echo -e "${RED}Błąd podczas konfiguracji cmake!${NC}"
        echo "Sprawdź logi powyżej, aby uzyskać więcej informacji."
        echo -n "Naciśnij Enter, aby kontynuować..."
        read
        return 1
    fi
    
    echo ""
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Funkcja do instalacji WebUI - Apache + PHP na bazie plików GitHub
install_webui() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instalacja WebUI (Apache + PHP)...${NC}"
    echo ""
    
    # Sprawdź czy katalog z projektem istnieje
    if [ ! -d "$INSTALL_DIR" ]; then
        echo -e "${RED}Projekt nie został pobrany! Najpierw wybierz opcję 2 (Pobierz projekt z GitHub).${NC}"
        echo ""
        echo -n "Czy chcesz pobrać projekt teraz? (t/n): "
        read -r response
        if [[ "$response" =~ ^[Tt]$ ]]; then
            download_from_github
        else
            echo -e "${YELLOW}Anulowano instalację WebUI.${NC}"
            echo -n "Naciśnij Enter, aby kontynuować..."
            read
            return
        fi
    fi
    
    echo -e "${BLUE}Instalowanie Apache i PHP z wymaganymi rozszerzeniami...${NC}"
    echo ""
    
    # Instalacja Apache i PHP w zależności od dystrybucji
    if command -v apt &> /dev/null; then
        echo -e "${CYAN}Wykryto apt. Instalowanie Apache2 i PHP...${NC}"
        sudo apt update
        if ! sudo apt install -y apache2 php php-sockets php-curl libapache2-mod-php; then
            echo -e "${RED}Błąd podczas instalacji Apache/PHP!${NC}"
            echo -n "Naciśnij Enter, aby kontynuować..."
            read
            return 1
        fi
    elif command -v yum &> /dev/null; then
        echo -e "${CYAN}Wykryto yum. Instalowanie HTTPD i PHP...${NC}"
        sudo yum install -y httpd php php-sockets php-curl
    elif command -v dnf &> /dev/null; then
        echo -e "${CYAN}Wykryto dnf. Instalowanie HTTPD i PHP...${NC}"
        sudo dnf install -y httpd php php-sockets php-curl
    elif command -v pacman &> /dev/null; then
        echo -e "${CYAN}Wykryto pacman. Instalowanie Apache i PHP...${NC}"
        sudo pacman -S --noconfirm apache php
    else
        echo -e "${RED}Nie wykryto znanego menedżera pakietów.${NC}"
        echo "Proszę ręcznie zainstalować Apache i PHP."
        echo -n "Naciśnij Enter, aby kontynuować..."
        read
        return
    fi
    
    echo ""
    echo -e "${BLUE}Konfigurowanie WebUI z plików GitHub...${NC}"
    echo ""
    
    # Skopiuj pliki WebUI do katalogu Apache
    WEBUI_SRC="$INSTALL_DIR/webui"
    if [ -d "$WEBUI_SRC" ]; then
        echo -e "${CYAN}Kopiowanie plików WebUI do katalogu serwera...${NC}"
        
        if command -v apt &> /dev/null; then
            APACHE_DIR="/var/www/html"
        else
            APACHE_DIR="/var/www/html"
        fi
        
        sudo mkdir -p "$APACHE_DIR/bioresonance"
        sudo cp -r "$WEBUI_SRC"/* "$APACHE_DIR/bioresonance/"
        sudo chown -R www-data:www-data "$APACHE_DIR/bioresonance" 2>/dev/null || \
            sudo chown -R apache:apache "$APACHE_DIR/bioresonance" 2>/dev/null || true
        sudo chmod -R 755 "$APACHE_DIR/bioresonance"
        
        echo -e "${GREEN}Pliki WebUI skopiowane do: $APACHE_DIR/bioresonance${NC}"
    else
        echo -e "${RED}Katalog webui nie znaleziony w projekcie!${NC}"
        echo -n "Naciśnij Enter, aby kontynuować..."
        read
        return 1
    fi
    
    # Sprawdź czy PHP sockets są włączone
    echo -e "${CYAN}Sprawdzanie rozszerzenia php-sockets...${NC}"
    if php -m | grep -q sockets; then
        echo -e "${GREEN}Rozszerzenie sockets jest włączone.${NC}"
    else
        echo -e "${YELLOW}Rozszerzenie sockets nie jest włączone. Spróbuj zainstalować php-sockets.${NC}"
    fi
    
    # Restart Apache
    echo -e "${CYAN}Restartowanie serwera Apache...${NC}"
    if command -v systemctl &> /dev/null; then
        sudo systemctl restart apache2 2>/dev/null || sudo systemctl restart httpd 2>/dev/null || true
        sudo systemctl enable apache2 2>/dev/null || sudo systemctl enable httpd 2>/dev/null || true
    fi
    
    echo ""
    echo -e "${GREEN}Zakończono instalację WebUI!${NC}"
    echo ""
    echo -e "${CYAN}WebUI jest dostępny pod adresem:${NC}"
    echo "  http://localhost/bioresonance/"
    echo ""
    echo -e "${BLUE}Pliki źródłowe pochodzą z:${NC}"
    echo "  $GITHUB_REPO"
    echo ""
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Funkcja do instalacji AndroidApp - na bazie plików GitHub z automatyczną kompilacją
install_androidapp() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instalacja AndroidApp (na bazie plików GitHub)...${NC}"
    echo ""
    
    # Sprawdź czy katalog z projektem istnieje
    if [ ! -d "$INSTALL_DIR" ]; then
        echo -e "${RED}Projekt nie został pobrany! Najpierw wybierz opcję 2 (Pobierz projekt z GitHub).${NC}"
        echo ""
        echo -n "Czy chcesz pobrać projekt teraz? (t/n): "
        read -r response
        if [[ "$response" =~ ^[Tt]$ ]]; then
            download_from_github
        else
            echo -e "${YELLOW}Anulowano instalację AndroidApp.${NC}"
            echo -n "Naciśnij Enter, aby kontynuować..."
            read
            return
        fi
    fi
    
    echo -e "${BLUE}Konfigurowanie środowiska dla Android App...${NC}"
    echo ""
    
    # Sprawdzenie czy JAVA jest zainstalowana
    if ! command -v java &> /dev/null; then
        echo -e "${YELLOW}Java nie jest zainstalowana. Instalowanie...${NC}"
        if command -v apt &> /dev/null; then
            sudo apt install -y openjdk-17-jdk
        elif command -v yum &> /dev/null; then
            sudo yum install -y java-17-openjdk-devel
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y java-17-openjdk-devel
        elif command -v pacman &> /dev/null; then
            sudo pacman -S --noconfirm jdk17-openjdk
        fi
    else
        echo -e "${GREEN}Java jest już zainstalowana: $(java -version 2>&1 | head -n 1)${NC}"
    fi
    
    echo ""
    echo -e "${CYAN}Projekt Android App znajduje się w:${NC}"
    echo "  $INSTALL_DIR/android_app/"
    echo ""
    
    # Sprawdź czy ANDROID_HOME lub ANDROID_SDK_ROOT jest ustawione
    if [ -z "$ANDROID_HOME" ] && [ -z "$ANDROID_SDK_ROOT" ]; then
        echo -e "${YELLOW}Zmienne środowiskowe Android SDK nie są ustawione.${NC}"
        echo ""
        echo -e "${BLUE}Aby zbudować aplikację Android, musisz mieć zainstalowane Android SDK.${NC}"
        echo ""
        echo "Opcje:"
        echo "  1. Zainstaluj Android Studio (zalecane)"
        echo "     https://developer.android.com/studio"
        echo ""
        echo "  2. Zainstaluj cmdline-tools ręcznie:"
        echo "     mkdir -p ~/Android/Sdk/cmdline-tools"
        echo "     cd ~/Android/Sdk/cmdline-tools"
        echo "     wget https://dl.google.com/android/repository/commandlinetools-linux-*.zip"
        echo "     unzip commandlinetools-linux-*.zip"
        echo "     mv cmdline-tools latest"
        echo ""
        echo "  3. Dodaj do ~/.bashrc:"
        echo "     export ANDROID_HOME=\$HOME/Android/Sdk"
        echo "     export PATH=\$PATH:\$ANDROID_HOME/cmdline-tools/latest/bin"
        echo "     export PATH=\$PATH:\$ANDROID_HOME/platform-tools"
        echo "     # Ścieżka może wymagać aktualizacji w zależności od wersji build-tools"
        echo "     export PATH=\$PATH:\$ANDROID_HOME/build-tools/*"
        echo ""
        
        # Spróbuj wykryć typową lokalizację Android SDK
        if [ -d "$HOME/Android/Sdk" ]; then
            echo -e "${GREEN}Wykryto Android SDK w: $HOME/Android/Sdk${NC}"
            export ANDROID_HOME="$HOME/Android/Sdk"
            export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"
        elif [ -d "/opt/android-sdk" ]; then
            echo -e "${GREEN}Wykryto Android SDK w: /opt/android-sdk${NC}"
            export ANDROID_HOME="/opt/android-sdk"
            export PATH="$PATH:$ANDROID_HOME/tools:$ANDROID_HOME/platform-tools"
        else
            echo -e "${YELLOW}Nie wykryto Android SDK. Kontynuowanie bez automatycznej kompilacji.${NC}"
            echo ""
            echo -e "${BLUE}Możesz ręcznie zbudować aplikację po skonfigurowaniu SDK:${NC}"
            echo "  cd $INSTALL_DIR/android_app/"
            echo "  ./gradlew assembleDebug"
            echo ""
            echo -n "Naciśnij Enter, aby kontynuować..."
            read
            return
        fi
    fi
    
    # Sprawdź czy gradlew istnieje, jeśli nie - utwórz wrapper
    ANDROID_DIR="$INSTALL_DIR/android_app"
    cd "$ANDROID_DIR" || return
    
    if [ ! -f "gradlew" ]; then
        echo -e "${CYAN}Tworzenie Gradle Wrapper...${NC}"
        
        # Sprawdź czy gradle jest zainstalowane systemowo
        if command -v gradle &> /dev/null; then
            echo -e "${BLUE}Wykryto systemowe Gradle. Generowanie wrapper...${NC}"
            gradle wrapper --gradle-version 8.0
        else
            echo -e "${YELLOW}Gradle nie jest zainstalowane. Pobieranie Gradle Wrapper...${NC}"
            
            # Pobierz gradle-wrapper.jar z lepszą obsługą błędów
            mkdir -p gradle/wrapper
            echo -e "${CYAN}Pobieranie gradle-wrapper.jar...${NC}"
            
            # Spróbuj pobrać z głównego repozytorium Gradle
            if ! curl -L --connect-timeout 10 --max-time 60 \
                -o gradle/wrapper/gradle-wrapper.jar \
                "https://raw.githubusercontent.com/gradle/gradle/master/gradle/wrapper/gradle-wrapper.jar" 2>/dev/null; then
                
                echo -e "${YELLOW}Nie udało się pobrać z GitHub. Próba alternatywnego źródła...${NC}"
                
                # Alternatywne źródło - services.gradle.org
                if ! curl -L --connect-timeout 10 --max-time 60 \
                    -o gradle/wrapper/gradle-wrapper.jar \
                    "https://services.gradle.org/distributions/gradle-8.0-bin.zip" 2>/dev/null; then
                    
                    echo -e "${RED}Nie udało się pobrać gradle-wrapper.jar z żadnego źródła.${NC}"
                    echo "Sprawdź połączenie internetowe lub zapory sieciowe."
                    echo ""
                    echo -e "${BLUE}Alternatywnie możesz otworzyć projekt w Android Studio:${NC}"
                    echo "  1. Otwórz Android Studio"
                    echo "  2. Wybierz 'Open an Existing Project'"
                    echo "  3. Wskaż katalog: $ANDROID_DIR"
                    echo ""
                    echo -n "Naciśnij Enter, aby kontynuować..."
                    read
                    return
                fi
            fi
            
            # Weryfikacja czy plik został pobrany i nie jest pusty
            if [ ! -s gradle/wrapper/gradle-wrapper.jar ]; then
                echo -e "${RED}Pobrano pusty lub niewłaściwy plik gradle-wrapper.jar.${NC}"
                echo "Sprawdź połączenie internetowe."
                echo ""
                echo -n "Naciśnij Enter, aby kontynuować..."
                read
                return
            fi
            
            echo -e "${GREEN}Pomyślnie pobrano gradle-wrapper.jar${NC}"
            
            # Utwórz gradlew script
            cat > gradlew << 'GRADLEW_SCRIPT'
#!/bin/bash
##############################################################################
## Gradle start up script for UN*X
##############################################################################

APP_NAME="Gradle"
APP_BASE_NAME=`basename "$0"`
DIRNAME=`dirname "$0"`
APP_HOME=`cd "$DIRNAME" > /dev/null; pwd`

DEFAULT_JVM_OPTS='"-Xmx64m" "-Xms64m"'
JAVA_OPTS=""

CLASSPATH=$APP_HOME/gradle/wrapper/gradle-wrapper.jar

exec java $JAVA_OPTS $DEFAULT_JVM_OPTS -classpath "$CLASSPATH" org.gradle.wrapper.GradleWrapperMain "$@"
GRADLEW_SCRIPT
            chmod +x gradlew
            
            # Utwórz gradle-wrapper.properties
            cat > gradle/wrapper/gradle-wrapper.properties << 'PROPS'
distributionBase=GRADLE_USER_HOME
distributionPath=wrapper/dists
distributionUrl=https\://services.gradle.org/distributions/gradle-8.0-bin.zip
zipStoreBase=GRADLE_USER_HOME
zipStorePath=wrapper/dists
PROPS
        fi
    fi
    
    echo ""
    echo -e "${BLUE}Rozpoczynanie kompilacji aplikacji Android...${NC}"
    echo ""
    
    # Akceptuj licencje Android SDK (automatycznie)
    if [ -n "$ANDROID_HOME" ] && [ -d "$ANDROID_HOME/licenses" ]; then
        echo -e "${CYAN}Akceptowanie licencji Android SDK...${NC}"
        yes | sdkmanager --licenses > /dev/null 2>&1 || true
    fi
    
    # Sprawdź czy potrzebne komponenty SDK są zainstalowane
    if command -v sdkmanager &> /dev/null; then
        echo -e "${CYAN}Sprawdzanie wymaganych komponentów SDK...${NC}"
        sdkmanager --list 2>/dev/null | grep -E "build-tools;34.0.0|platforms;android-34" || true
    fi
    
    # Uruchom kompilację
    echo -e "${CYAN}Uruchamianie Gradle build...${NC}"
    echo ""
    
    if ./gradlew assembleDebug --stacktrace; then
        echo ""
        echo -e "${GREEN}========================================${NC}"
        echo -e "${GREEN}Pomyślnie zbudowano aplikację Android!${NC}"
        echo -e "${GREEN}========================================${NC}"
        echo ""
        
        # Znajdź wygenerowany plik APK
        APK_FILE=$(find "$ANDROID_DIR" -name "*.apk" -path "*/build/outputs/apk/*" -type f 2>/dev/null | head -n 1)
        
        if [ -n "$APK_FILE" ]; then
            echo -e "${CYAN}Plik APK znajduje się w:${NC}"
            echo "  $APK_FILE"
            echo ""
            echo -e "${BLUE}Aby zainstalować na urządzeniu:${NC}"
            echo "  adb install $APK_FILE"
            echo ""
            echo -e "${BLUE}Lub skopiuj plik na urządzenie i zainstaluj ręcznie.${NC}"
        else
            echo -e "${YELLOW}Nie znaleziono pliku APK w oczekiwanym miejscu.${NC}"
            echo "Sprawdź katalog build/outputs/apk/ w projekcie."
        fi
    else
        echo ""
        echo -e "${RED}========================================${NC}"
        echo -e "${RED}Błąd podczas kompilacji aplikacji Android!${NC}"
        echo -e "${RED}========================================${NC}"
        echo ""
        echo -e "${YELLOW}Możliwe przyczyny:${NC}"
        echo "  - Brak Android SDK lub niepoprawnie skonfigurowane"
        echo "  - Brak wymaganych komponentów (build-tools, platform)"
        echo "  - Błędy w kodzie źródłowym"
        echo "  - Problemy z siecią podczas pobierania zależności"
        echo ""
        echo -e "${BLUE}Rozwiązania:${NC}"
        echo "  1. Upewnij się, że ANDROID_HOME jest poprawnie ustawione"
        echo "  2. Zainstaluj brakujące komponenty przez sdkmanager:"
        echo "     sdkmanager \"platform-tools\" \"platforms;android-34\" \"build-tools;34.0.0\""
        echo "  3. Spróbuj otworzyć projekt w Android Studio"
        echo ""
        echo -e "${CYAN}Możesz też ręcznie uruchomić kompilację:${NC}"
        echo "  cd $ANDROID_DIR"
        echo "  ./gradlew assembleDebug"
        echo ""
        echo -n "Naciśnij Enter, aby kontynuować..."
        read
        return 1
    fi
    
    echo ""
    echo -e "${CYAN}Pliki źródłowe pochodzą z:${NC}"
    echo "  $GITHUB_REPO"
    echo ""
    echo -e "${GREEN}Zakończono instalację i kompilację AndroidApp.${NC}"
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Funkcja do weryfikacji instalacji
verify_installation() {
    clear_screen
    show_header
    echo -e "${YELLOW}Weryfikacja instalacji...${NC}"
    echo ""
    
    local errors=0
    
    # Sprawdzenie podstawowych narzędzi
    echo "Sprawdzanie podstawowych narzędzi:"
    
    if command -v git &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} git jest zainstalowany"
    else
        echo -e "  ${RED}✗${NC} git nie jest zainstalowany"
        ((errors++))
    fi
    
    if command -v curl &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} curl jest zainstalowany"
    else
        echo -e "  ${RED}✗${NC} curl nie jest zainstalowany"
        ((errors++))
    fi
    
    if command -v wget &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} wget jest zainstalowany"
    else
        echo -e "  ${RED}✗${NC} wget nie jest zainstalowany"
        ((errors++))
    fi
    
    if command -v node &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} node.js jest zainstalowany ($(node --version))"
    else
        echo -e "  ${RED}✗${NC} node.js nie jest zainstalowany"
        ((errors++))
    fi
    
    if command -v npm &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} npm jest zainstalowany ($(npm --version))"
    else
        echo -e "  ${RED}✗${NC} npm nie jest zainstalowany"
        ((errors++))
    fi
    
    if command -v python3 &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} python3 jest zainstalowany ($(python3 --version))"
    else
        echo -e "  ${RED}✗${NC} python3 nie jest zainstalowany"
        ((errors++))
    fi
    
    if command -v pip3 &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} pip3 jest zainstalowany"
    else
        echo -e "  ${RED}✗${NC} pip3 nie jest zainstalowany"
        ((errors++))
    fi
    
    if command -v php &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} php jest zainstalowany ($(php --version | head -n 1))"
    else
        echo -e "  ${YELLOW}!${NC} php nie jest zainstalowany (wymagany dla WebUI)"
    fi
    
    if command -v apache2ctl &> /dev/null || command -v httpd &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} Apache jest zainstalowany"
    else
        echo -e "  ${YELLOW}!${NC} Apache nie jest zainstalowany (wymagany dla WebUI)"
    fi
    
    if command -v java &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} java jest zainstalowana ($(java -version 2>&1 | head -n 1))"
    else
        echo -e "  ${YELLOW}!${NC} java nie jest zainstalowana (wymagana dla Android)"
    fi
    
    if command -v gcc &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} gcc jest zainstalowany"
    else
        echo -e "  ${RED}✗${NC} gcc nie jest zainstalowany"
        ((errors++))
    fi
    
    if command -v cmake &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} cmake jest zainstalowany ($(cmake --version | head -n 1))"
    else
        echo -e "  ${YELLOW}!${NC} cmake nie jest zainstalowany (wymagany dla GUI)"
    fi
    
    # Sprawdzenie czy projekt został pobrany
    echo ""
    echo "Sprawdzanie projektu Bioresonance:"
    if [ -d "$INSTALL_DIR" ]; then
        echo -e "  ${GREEN}✓${NC} Projekt pobrany do: $INSTALL_DIR"
        
        # Sprawdź podkatalogi
        [ -d "$INSTALL_DIR/gui" ] && echo -e "  ${GREEN}✓${NC} gui/ - dostępny" || echo -e "  ${YELLOW}!${NC} gui/ - brak"
        [ -d "$INSTALL_DIR/webui" ] && echo -e "  ${GREEN}✓${NC} webui/ - dostępny" || echo -e "  ${YELLOW}!${NC} webui/ - brak"
        [ -d "$INSTALL_DIR/android_app" ] && echo -e "  ${GREEN}✓${NC} android_app/ - dostępny" || echo -e "  ${YELLOW}!${NC} android_app/ - brak"
        [ -d "$INSTALL_DIR/bash_tui" ] && echo -e "  ${GREEN}✓${NC} bash_tui/ - dostępny" || echo -e "  ${YELLOW}!${NC} bash_tui/ - brak"
        [ -d "$INSTALL_DIR/tui" ] && echo -e "  ${GREEN}✓${NC} tui/ - dostępny" || echo -e "  ${YELLOW}!${NC} tui/ - brak"
        [ -d "$INSTALL_DIR/ResoNet_Nano" ] && echo -e "  ${GREEN}✓${NC} ResoNet_Nano/ - dostępny" || echo -e "  ${YELLOW}!${NC} ResoNet_Nano/ - brak"
    else
        echo -e "  ${RED}✗${NC} Projekt nie został pobrany - użyj opcji 2"
    fi
    
    echo ""
    if [ $errors -eq 0 ]; then
        echo -e "${GREEN}Wszystkie podstawowe zależności są zainstalowane!${NC}"
    else
        echo -e "${RED}Znaleziono $errors brakujących zależności.${NC}"
        echo "Uruchom opcję 1, aby zainstalować zależności."
    fi
    
    echo ""
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Główna pętla programu
main() {
    while true; do
        show_menu
        read -r choice
        
        case $choice in
            1)
                install_dependencies
                ;;
            2)
                download_from_github
                ;;
            3)
                instruction_bashtui
                ;;
            4)
                instruction_cpptui
                ;;
            5)
                install_gui
                ;;
            6)
                install_webui
                ;;
            7)
                install_androidapp
                ;;
            8)
                verify_installation
                ;;
            9)
                clear_screen
                show_header
                echo -e "${GREEN}Dziękujemy za korzystanie ze skryptu instalacyjnego!${NC}"
                echo ""
                exit 0
                ;;
            *)
                echo -e "${RED}Nieprawidłowy wybór. Spróbuj ponownie.${NC}"
                echo -n "Naciśnij Enter, aby kontynuować..."
                read
                ;;
        esac
    done
}

# Uruchomienie głównej funkcji
main
