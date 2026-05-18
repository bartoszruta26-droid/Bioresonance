#!/bin/bash

# Skrypt instalacyjny z menu
# Autor: Assistant
# Wersja: 1.0

# Kolory dla wyjścia
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Funkcja do czyszczenia ekranu
clear_screen() {
    clear
}

# Funkcja do wyświetlania nagłówka
show_header() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}       Skrypt Instalacyjny${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
}

# Funkcja do wyświetlania menu
show_menu() {
    clear_screen
    show_header
    echo -e "${YELLOW}Wybierz opcję:${NC}"
    echo ""
    echo -e "  ${GREEN}1)${NC} Instalacja zależności"
    echo -e "  ${GREEN}2)${NC} Instrukcja Bashtui"
    echo -e "  ${GREEN}3)${NC} Instrukcja Cpptui"
    echo -e "  ${GREEN}4)${NC} Instalacja GUI"
    echo -e "  ${GREEN}5)${NC} Instalacja WebUI"
    echo -e "  ${GREEN}6)${NC} Instalacja AndroidApp"
    echo -e "  ${GREEN}7)${NC} Weryfikacja instalacji"
    echo -e "  ${GREEN}8)${NC} Wyjście"
    echo ""
    echo -n -e "${BLUE}Podaj numer opcji [1-8]: ${NC}"
}

# Funkcja do instalacji zależności
install_dependencies() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instalacja zależności...${NC}"
    echo ""
    
    # Sprawdzenie menedżera pakietów
    if command -v apt &> /dev/null; then
        echo -e "${BLUE}Wykryto apt. Instalowanie zależności...${NC}"
        sudo apt update
        sudo apt install -y build-essential git curl wget nodejs npm python3 python3-pip
    elif command -v yum &> /dev/null; then
        echo -e "${BLUE}Wykryto yum. Instalowanie zależności...${NC}"
        sudo yum update -y
        sudo yum install -y gcc gcc-c++ git curl wget nodejs npm python3 python3-pip
    elif command -v dnf &> /dev/null; then
        echo -e "${BLUE}Wykryto dnf. Instalowanie zależności...${NC}"
        sudo dnf update -y
        sudo dnf install -y gcc gcc-c++ git curl wget nodejs npm python3 python3-pip
    elif command -v pacman &> /dev/null; then
        echo -e "${BLUE}Wykryto pacman. Instalowanie zależności...${NC}"
        sudo pacman -Syu --noconfirm
        sudo pacman -S --noconfirm base-devel git curl wget nodejs npm python python-pip
    else
        echo -e "${RED}Nie wykryto znanego menedżera pakietów.${NC}"
        echo "Proszę ręcznie zainstalować niezbędne zależności."
    fi
    
    echo ""
    echo -e "${GREEN}Zakończono instalację zależności.${NC}"
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

# Funkcja do instalacji GUI
install_gui() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instalacja GUI...${NC}"
    echo ""
    
    echo -e "${BLUE}Instalowanie zależności dla GUI...${NC}"
    
    if command -v apt &> /dev/null; then
        sudo apt update
        sudo apt install -y python3-tk qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
    elif command -v yum &> /dev/null; then
        sudo yum install -y python3-tkinter qt5-qtbase-devel
    elif command -v dnf &> /dev/null; then
        sudo dnf install -y python3-tkinter qt5-qtbase-devel
    elif command -v pacman &> /dev/null; then
        sudo pacman -S --noconfirm tk qt5-base
    else
        echo -e "${RED}Nie wykryto znanego menedżera pakietów.${NC}"
    fi
    
    echo ""
    echo -e "${GREEN}Zakończono instalację GUI.${NC}"
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Funkcja do instalacji WebUI
install_webui() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instalacja WebUI...${NC}"
    echo ""
    
    echo -e "${BLUE}Instalowanie zależności dla WebUI...${NC}"
    
    if command -v npm &> /dev/null; then
        echo "Instalowanie Node.js dependencies..."
        npm install -g express react-cli vue-cli
    else
        echo -e "${RED}npm nie jest zainstalowane. Najpierw zainstaluj Node.js.${NC}"
    fi
    
    if command -v pip3 &> /dev/null; then
        echo "Instalowanie Python dependencies..."
        pip3 install flask django
    else
        echo -e "${RED}pip3 nie jest zainstalowane.${NC}"
    fi
    
    echo ""
    echo -e "${GREEN}Zakończono instalację WebUI.${NC}"
    echo -n "Naciśnij Enter, aby kontynuować..."
    read
}

# Funkcja do instalacji AndroidApp
install_androidapp() {
    clear_screen
    show_header
    echo -e "${YELLOW}Instalacja AndroidApp...${NC}"
    echo ""
    
    echo -e "${BLUE}Instalowanie narzędzi dla Android...${NC}"
    
    # Sprawdzenie czy JAVA jest zainstalowana
    if ! command -v java &> /dev/null; then
        echo -e "${YELLOW}Java nie jest zainstalowana. Instalowanie...${NC}"
        if command -v apt &> /dev/null; then
            sudo apt install -y openjdk-11-jdk
        elif command -v yum &> /dev/null; then
            sudo yum install -y java-11-openjdk-devel
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y java-11-openjdk-devel
        elif command -v pacman &> /dev/null; then
            sudo pacman -S --noconfirm jdk11-openjdk
        fi
    fi
    
    echo ""
    echo "Aby skonfigurować Android SDK:"
    echo "  1. Pobierz Android Command Line Tools z: https://developer.android.com/studio"
    echo "  2. Rozpakuj do ~/Android/Sdk"
    echo "  3. Dodaj do ~/.bashrc:"
    echo "     export ANDROID_HOME=\$HOME/Android/Sdk"
    echo "     export PATH=\$PATH:\$ANDROID_HOME/tools"
    echo "     export PATH=\$PATH:\$ANDROID_HOME/platform-tools"
    echo ""
    echo "  4. Zainstaluj wymagane komponenty:"
    echo "     sdkmanager \"platform-tools\" \"platforms;android-30\""
    echo ""
    echo -e "${GREEN}Zakończono konfigurację AndroidApp.${NC}"
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
                instruction_bashtui
                ;;
            3)
                instruction_cpptui
                ;;
            4)
                install_gui
                ;;
            5)
                install_webui
                ;;
            6)
                install_androidapp
                ;;
            7)
                verify_installation
                ;;
            8)
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
