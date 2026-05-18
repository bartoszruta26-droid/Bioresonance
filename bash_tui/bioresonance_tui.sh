#!/bin/bash
# ============================================================================
# @file bioresonance_tui.sh
# @brief Terminalowy Interfejs Użytkownika dla Systemu Biorezonansu ResoNet-Nano
# @version 1.0
# 
# @description
# TUI sterujące Arduino Nano z Ethernet HAT poprzez połączenie sieciowe TCP.
# Obsługuje liczne końcówki biorezonansowe w trybie pojedynczym i wielokanałowym.
# Implementacja w czystym bashu bez zależności od C++/Python.
# 
# Uruchomienie: ./bioresonance_tui.sh [OPTIONS] <ip_address> [port]
# Options:
#   -h, --help      Show help message
#   -v, --verbose   Enable verbose output (debug mode)
#   -d, --debug     Enable debug mode (same as verbose)
# ============================================================================

set -u

# ============================================================================
# KONFIGURACJA SYSTEMU
# ============================================================================

DEFAULT_PORT=5001
TIMEOUT_SEC=2
REFRESH_RATE=0.1

# Verbose/Debug mode
VERBOSE=false
DEBUG=false

# Kolory ANSI
COLOR_RESET="\033[0m"
COLOR_BOLD="\033[1m"
COLOR_GREEN="\033[32m"
COLOR_RED="\033[31m"
COLOR_YELLOW="\033[33m"
COLOR_BLUE="\033[34m"
COLOR_MAGENTA="\033[35m"
COLOR_CYAN="\033[36m"
COLOR_WHITE="\033[37m"
COLOR_BLACK="\033[30m"
COLOR_BG_BLUE="\033[44m"
COLOR_BG_WHITE="\033[47m"

# ============================================================================
# ZMIENNE GLOBALNE
# ============================================================================

DEVICE_IP=""
DEVICE_PORT=""
SOCKET_FD=""
CONNECTED=false
RUNNING=true

# Menu navigation
CURRENT_MENU=0
SELECTED_PROBE=1

# Probe configurations (channel:name:type:freq:duty:intensity:modulation:enabled)
declare -A PROBES
PROBES[1]="Cewka Płaska Główna:FLAT_COIL:727.00:50:2048:NONE:false"
PROBES[2]="Cewka Ferrytowa Punktowa:FERRITE_ROD:10000.00:50:2048:NONE:false"
PROBES[3]="Płyta Kapacytacyjna:CAPACITIVE_PLATE:5000.00:50:2048:NONE:false"
PROBES[4]="Aplikator Punktowy:PEN_APPLICATOR:25000.00:50:2048:NONE:false"
PROBES[5]="Mata EMF Całe Ciało:MAT_APPLICATOR:78.30:50:2048:NONE:false"
PROBES[6]="Podkładka Lokalna:LOCAL_PAD:1000.00:50:2048:NONE:false"
PROBES[7]="Pierścień Kończyna:RING_APPLICATOR:500.00:50:2048:NONE:false"
PROBES[8]="Konfiguracja Niestandardowa:CUSTOM:10.00:50:2048:NONE:false"

# System status
STATUS_TEMP="0.0"
STATUS_MEMORY="0"
STATUS_UPTIME="0"
STATUS_PWM="false"
STATUS_FREQ="0"
STATUS_SAFETY="UNKNOWN"

# Log messages
declare -a LOG_MESSAGES
LOG_COUNT=0

# Probe mode
PROBE_MODE="SINGLE"

# ============================================================================
# FUNKCJE POMOCNICZE
# ============================================================================

# Debug logging function - only outputs when DEBUG or VERBOSE is true
debug_log() {
    if [ "$DEBUG" = true ] || [ "$VERBOSE" = true ]; then
        local timestamp=$(date +"%H:%M:%S")
        echo -e "${COLOR_CYAN}[DEBUG $timestamp]${COLOR_RESET} $1" >&2
    fi
}

# Error logging function - always outputs to stderr
error_log() {
    local timestamp=$(date +"%H:%M:%S")
    echo -e "${COLOR_RED}[ERROR $timestamp]${COLOR_RESET} $1" >&2
    log_message "ERROR: $1"
}

# Info logging function - outputs when VERBOSE is true
info_log() {
    if [ "$VERBOSE" = true ]; then
        local timestamp=$(date +"%H:%M:%S")
        echo -e "${COLOR_GREEN}[INFO $timestamp]${COLOR_RESET} $1" >&2
    fi
}

log_message() {
    local timestamp=$(date +"%H:%M:%S")
    LOG_MESSAGES[$LOG_COUNT]="[$timestamp] $1"
    ((LOG_COUNT++))
    if [ $LOG_COUNT -gt 50 ]; then
        # Przesuń logi
        for ((i=0; i<LOG_COUNT-1; i++)); do
            LOG_MESSAGES[$i]="${LOG_MESSAGES[$((i+1))]}"
        done
        ((LOG_COUNT--))
    fi
    debug_log "Logged: $1"
}

get_probe_field() {
    local channel=$1
    local field=$2
    local data="${PROBES[$channel]}"
    
    case $field in
        name) echo "$data" | cut -d: -f1 ;;
        type) echo "$data" | cut -d: -f2 ;;
        freq) echo "$data" | cut -d: -f3 ;;
        duty) echo "$data" | cut -d: -f4 ;;
        intensity) echo "$data" | cut -d: -f5 ;;
        modulation) echo "$data" | cut -d: -f6 ;;
        enabled) echo "$data" | cut -d: -f7 ;;
    esac
}

set_probe_field() {
    local channel=$1
    local field=$2
    local value=$3
    
    # Validate channel exists
    if [ -z "${PROBES[$channel]+x}" ]; then
        error_log "Invalid channel: $channel"
        return 1
    fi
    
    local name=$(get_probe_field $channel name)
    local type=$(get_probe_field $channel type)
    local freq=$(get_probe_field $channel freq)
    local duty=$(get_probe_field $channel duty)
    local intensity=$(get_probe_field $channel intensity)
    local modulation=$(get_probe_field $channel modulation)
    local enabled=$(get_probe_field $channel enabled)
    
    case $field in
        name) name="$value" ;;
        type) type="$value" ;;
        freq) 
            # Validate frequency is numeric
            if [[ ! "$value" =~ ^[0-9]+\.?[0-9]*$ ]]; then
                error_log "Invalid frequency format: $value"
                return 1
            fi
            freq="$value" 
            ;;
        duty) 
            # Validate duty cycle 0-100
            if [ "$value" -lt 0 ] || [ "$value" -gt 100 ] 2>/dev/null; then
                error_log "Duty cycle out of range (0-100): $value"
                return 1
            fi
            duty="$value" 
            ;;
        intensity) 
            # Validate intensity 0-4095
            if [ "$value" -lt 0 ] || [ "$value" -gt 4095 ] 2>/dev/null; then
                error_log "Intensity out of range (0-4095): $value"
                return 1
            fi
            intensity="$value" 
            ;;
        modulation) modulation="$value" ;;
        enabled) enabled="$value" ;;
        *)
            error_log "Unknown field: $field"
            return 1
            ;;
    esac
    
    debug_log "Setting probe $channel.$field = $value"
    PROBES[$channel]="$name:$type:$freq:$duty:$intensity:$modulation:$enabled"
}

get_type_name() {
    local type=$1
    case $type in
        FLAT_COIL) echo "Cewka Płaska" ;;
        FERRITE_ROD) echo "Cewka Ferrytowa" ;;
        CAPACITIVE_PLATE) echo "Płyta Kapacytacyjna" ;;
        PEN_APPLICATOR) echo "Aplikator Punktowy" ;;
        MAT_APPLICATOR) echo "Mata EMF" ;;
        LOCAL_PAD) echo "Podkładka Lokalna" ;;
        RING_APPLICATOR) echo "Pierścień" ;;
        CUSTOM) echo "Niestandardowa" ;;
        *) echo "Nieznana" ;;
    esac
}

get_modulation_name() {
    local mod=$1
    case $mod in
        NONE) echo "Brak" ;;
        AM) echo "AM" ;;
        FM) echo "FM" ;;
        BURST) echo "Burst" ;;
        SWEEP) echo "Sweep" ;;
        *) echo "Nieznana" ;;
    esac
}

# ============================================================================
# FUNKCJE SIECIOWE
# ============================================================================

connect_to_device() {
    log_message "Łączenie z urządzeniem $DEVICE_IP:$DEVICE_PORT..."
    debug_log "Attempting connection to $DEVICE_IP:$DEVICE_PORT"
    
    # Sprawdź czy ip/netcat jest dostępny
    if ! command -v nc &> /dev/null; then
        error_log "nc (netcat) nie jest zainstalowany!"
        CONNECTED=false
        return 1
    fi
    
    # Spróbuj połączyć się z urządzeniem
    SOCKET_FD="/tmp/bioresonance_socket_$$"
    
    # Test połączenia
    if timeout $TIMEOUT_SEC bash -c "echo '' | nc -w 1 $DEVICE_IP $DEVICE_PORT" 2>/dev/null; then
        CONNECTED=true
        log_message "Połączono z $DEVICE_IP:$DEVICE_PORT"
        info_log "Successfully connected to device"
        return 0
    else
        CONNECTED=false
        error_log "Błąd połączenia z urządzeniem $DEVICE_IP:$DEVICE_PORT"
        return 1
    fi
}

disconnect_from_device() {
    debug_log "Disconnecting from device"
    CONNECTED=false
    if [ -n "$SOCKET_FD" ] && [ -e "$SOCKET_FD" ]; then
        rm -f "$SOCKET_FD" 2>/dev/null
    fi
    log_message "Rozłączono z urządzeniem"
}

send_command() {
    local cmd=$1
    if [ "$CONNECTED" = false ]; then
        debug_log "Cannot send command - not connected: $cmd"
        return 1
    fi
    
    debug_log "Sending command: $cmd"
    echo "$cmd" | nc -w 1 $DEVICE_IP $DEVICE_PORT 2>/dev/null
    local ret=$?
    if [ $ret -ne 0 ]; then
        error_log "Failed to send command: $cmd"
    fi
    return $ret
}

send_probe_config() {
    local channel=$1
    if [ "$CONNECTED" = false ]; then
        return 1
    fi
    
    local freq=$(get_probe_field $channel freq)
    local duty=$(get_probe_field $channel duty)
    local intensity=$(get_probe_field $channel intensity)
    local modulation=$(get_probe_field $channel modulation)
    
    # Konwertuj częstotliwość do formatu Hz * 100
    local freq_x100=$(echo "$freq * 100" | bc | cut -d. -f1)
    
    # Format pakietu: FREQ,DUTY,INTENSITY,MODULATION,CHANNEL
    local cmd="CONFIG:$channel,$freq_x100,$duty,$intensity,$modulation"
    send_command "$cmd"
    
    log_message "Wysłano konfigurację kanału $channel"
}

request_status() {
    if [ "$CONNECTED" = false ]; then
        return 1
    fi
    send_command "s"
}

parse_status_response() {
    local response=$1
    
    # Parsuj odpowiedź statusu
    if echo "$response" | grep -q "Temperature:"; then
        STATUS_TEMP=$(echo "$response" | grep "Temperature:" | sed 's/.*Temperature: *\([0-9.]*\).*/\1/')
    fi
    
    if echo "$response" | grep -q "Free Memory:"; then
        STATUS_MEMORY=$(echo "$response" | grep "Free Memory:" | sed 's/.*Free Memory: *\([0-9]*\).*/\1/')
    fi
    
    if echo "$response" | grep -q "Uptime:"; then
        STATUS_UPTIME=$(echo "$response" | grep "Uptime:" | sed 's/.*Uptime: *\([0-9]*\).*/\1/')
    fi
    
    if echo "$response" | grep -q "PWM Running:"; then
        if echo "$response" | grep -q "YES"; then
            STATUS_PWM="true"
        else
            STATUS_PWM="false"
        fi
    fi
    
    if echo "$response" | grep -q "Frequency:"; then
        STATUS_FREQ=$(echo "$response" | grep "Frequency:" | sed 's/.*Frequency: *\([0-9]*\).*/\1/')
    fi
    
    if echo "$response" | grep -q "Safety State:"; then
        STATUS_SAFETY=$(echo "$response" | grep "Safety State:" | sed 's/.*Safety State: *\([A-Z_]*\).*/\1/')
    fi
}

# ============================================================================
# FUNKCJE UI - RYSOWANIE
# ============================================================================

clear_screen() {
    printf "\033[2J\033[H"
}

move_cursor() {
    printf "\033[%d;%dH" $1 $2
}

hide_cursor() {
    printf "\033[?25l"
}

show_cursor() {
    printf "\033[?25h"
}

draw_header() {
    local width=$(tput cols)
    
    move_cursor 1 1
    printf "${COLOR_BG_BLUE}${COLOR_WHITE}${COLOR_BOLD}"
    printf "%*s" $width ""
    move_cursor 1 1
    printf " RESONET-NANO TUI v1.0 (BASH) "
    move_cursor 1 $((width - 20))
    if [ "$CONNECTED" = true ]; then
        printf "${COLOR_GREEN}[POŁĄCZONO]${COLOR_RESET}"
    else
        printf "${COLOR_RED}[ROZŁĄCZONO]${COLOR_RESET}"
    fi
    printf "${COLOR_RESET}"
    
    move_cursor 2 1
    printf "${COLOR_CYAN}Device: %s:%d${COLOR_RESET}" "$DEVICE_IP" "$DEVICE_PORT"
    printf "  |  Tryb: ${COLOR_YELLOW}%s${COLOR_RESET}" "$PROBE_MODE"
}

draw_menu() {
    local menu_items=("Konfiguracja" "Tryb Pracy" "Start Terapii" "Stop Terapii" "Status" "Logi" "Pomoc" "Wyjście")
    local start_row=4
    local col=2
    
    move_cursor $start_row $col
    printf "${COLOR_BOLD}--- MENU GŁÓWNE ---${COLOR_RESET}\n"
    
    for i in "${!menu_items[@]}"; do
        local row=$((start_row + 2 + i))
        move_cursor $row $col
        
        if [ $i -eq $CURRENT_MENU ]; then
            printf "${COLOR_BG_WHITE}${COLOR_BLACK}${COLOR_BOLD} > %-20s < ${COLOR_RESET}" "${menu_items[$i]}"
        else
            printf "   %-20s   " "${menu_items[$i]}"
        fi
    done
}

draw_probes() {
    local start_row=4
    local col=30
    
    move_cursor $start_row $col
    printf "${COLOR_BOLD}--- KONFIGURACJA KOŃCÓWEK ---${COLOR_RESET}\n"
    printf "${COLOR_CYAN}(Strzałki L/R: wybór kanału, E: włącz/wyłącz)${COLOR_RESET}\n"
    
    local row=$((start_row + 3))
    for channel in 1 2 3 4 5 6 7 8; do
        move_cursor $row $col
        
        local name=$(get_probe_field $channel name)
        local type=$(get_probe_field $channel type)
        local freq=$(get_probe_field $channel freq)
        local duty=$(get_probe_field $channel duty)
        local intensity=$(get_probe_field $channel intensity)
        local modulation=$(get_probe_field $channel modulation)
        local enabled=$(get_probe_field $channel enabled)
        
        local type_name=$(get_type_name "$type")
        local mod_name=$(get_modulation_name "$modulation")
        
        if [ "$enabled" = "true" ]; then
            local status_str="${COLOR_GREEN}[AKTYWNA]${COLOR_RESET}"
        else
            local status_str="${COLOR_YELLOW}[NIEAKTYWNA]${COLOR_RESET}"
        fi
        
        if [ $channel -eq $SELECTED_PROBE ]; then
            printf "${COLOR_BOLD}${COLOR_BG_BLUE}>"
        else
            printf " "
        fi
        
        printf "Kanał %d: %-28s %s" $channel "$name" "$status_str"
        
        if [ $channel -eq $SELECTED_PROBE ]; then
            printf "<${COLOR_RESET}"
        fi
        
        ((row++))
        move_cursor $row $((col + 2))
        printf "Freq: %8s Hz | Duty: %2s%% | Int: %4s | Mod: %s" "$freq" "$duty" "$intensity" "$mod_name"
        ((row++))
    done
}

draw_status() {
    local height=$(tput lines)
    local start_row=$((height - 12))
    local col=2
    
    move_cursor $start_row $col
    printf "${COLOR_BOLD}--- STATUS SYSTEMU ---${COLOR_RESET}\n"
    printf "Temp: ${COLOR_CYAN}%s°C${COLOR_RESET}  |  " "$STATUS_TEMP"
    printf "Mem: ${COLOR_CYAN}%s B${COLOR_RESET}  |  " "$STATUS_MEMORY"
    printf "Uptime: ${COLOR_CYAN}%s s${COLOR_RESET}\n"
    
    if [ "$STATUS_PWM" = "true" ]; then
        printf "PWM: ${COLOR_GREEN}ACTIVE${COLOR_RESET}  |  Freq: ${COLOR_CYAN}%s Hz${COLOR_RESET}\n" "$STATUS_FREQ"
    else
        printf "PWM: ${COLOR_YELLOW}STOPPED${COLOR_RESET}\n"
    fi
    
    printf "Safety: ${COLOR_MAGENTA}%s${COLOR_RESET}\n" "$STATUS_SAFETY"
}

draw_log() {
    local height=$(tput lines)
    local start_row=$((height - 6))
    local col=2
    
    move_cursor $start_row $col
    printf "${COLOR_BOLD}--- DZIENNIK ZDARZEŃ ---${COLOR_RESET}\n"
    
    local display_count=4
    if [ $LOG_COUNT -lt $display_count ]; then
        display_count=$LOG_COUNT
    fi
    
    for ((i=0; i<display_count; i++)); do
        local idx=$((LOG_COUNT - display_count + i))
        if [ $idx -ge 0 ]; then
            move_cursor $((start_row + 2 + i)) $col
            printf "%s" "${LOG_MESSAGES[$idx]}"
        fi
    done
}

draw_help() {
    local height=$(tput lines)
    local width=$(tput cols)
    local start_row=$((height / 2 - 5))
    local start_col=$((width / 2 - 20))
    
    move_cursor $start_row $start_col
    printf "${COLOR_BG_BLUE}${COLOR_WHITE}${COLOR_BOLD}"
    printf "%*s" 40 ""
    move_cursor $start_row $start_col
    printf " --- POMOC --- "
    printf "${COLOR_RESET}\n"
    
    move_cursor $((start_row + 2)) $start_col
    printf "${COLOR_YELLOW}Sterowanie:${COLOR_RESET}\n"
    printf "  ↑/↓ : Nawigacja menu\n"
    printf "  ←/→ : Wybór kanału\n"
    printf "  1-8 : Akcje szybkie\n"
    printf "  E   : Włącz/Wyłącz końcówkę\n"
    printf "  F   : Edycja częstotliwości\n"
    printf "  I   : Edycja intensywności\n"
    printf "  M   : Zmiana modulacji\n"
    printf "  S   : Odśwież status\n"
    printf "  H   : Pomoc\n"
    printf "  Q   : Wyjście\n"
    
    move_cursor $((start_row + 14)) $start_col
    printf "${COLOR_CYAN}Naciśnij dowolny klawisz...${COLOR_RESET}"
    read -n 1 -s
}

refresh_display() {
    draw_header
    draw_menu
    draw_probes
    draw_status
    draw_log
}

# ============================================================================
# AKCJE UŻYTKOWNIKA
# ============================================================================

toggle_probe() {
    local channel=$1
    local current=$(get_probe_field $channel enabled)
    
    if [ "$current" = "true" ]; then
        set_probe_field $channel enabled "false"
        log_message "Kanał $channel: NIEAKTYWNA"
    else
        set_probe_field $channel enabled "true"
        log_message "Kanał $channel: AKTYWNA"
        send_probe_config $channel
    fi
}

edit_frequency() {
    local channel=$1
    local current_freq=$(get_probe_field $channel freq)
    local enabled=$(get_probe_field $channel enabled)
    
    if [ "$enabled" != "true" ]; then
        log_message "Wybierz aktywną końcówkę!"
        return
    fi
    
    move_cursor 20 2
    printf "${COLOR_YELLOW}Nowa częstotliwość (Hz) [obecna: %s]: ${COLOR_RESET}" "$current_freq"
    read -s new_freq
    
    if [ -n "$new_freq" ]; then
        # Walidacja
        if [[ "$new_freq" =~ ^[0-9]+\.?[0-9]*$ ]]; then
            set_probe_field $channel freq "$new_freq"
            log_message "Ustawiono freq: $new_freq Hz"
            send_probe_config $channel
        else
            log_message "Błąd: Nieprawidłowa częstotliwość!"
        fi
    fi
}

edit_intensity() {
    local channel=$1
    local current_intensity=$(get_probe_field $channel intensity)
    local enabled=$(get_probe_field $channel enabled)
    
    if [ "$enabled" != "true" ]; then
        log_message "Wybierz aktywną końcówkę!"
        return
    fi
    
    move_cursor 20 2
    printf "${COLOR_YELLOW}Intensywność (0-4095) [obecna: %s]: ${COLOR_RESET}" "$current_intensity"
    read -s new_intensity
    
    if [ -n "$new_intensity" ]; then
        if [ "$new_intensity" -ge 0 ] && [ "$new_intensity" -le 4095 ] 2>/dev/null; then
            set_probe_field $channel intensity "$new_intensity"
            log_message "Ustawiono intensity: $new_intensity"
            send_probe_config $channel
        else
            log_message "Błąd: Intensywność poza zakresem!"
        fi
    fi
}

edit_modulation() {
    local channel=$1
    local current_mod=$(get_probe_field $channel modulation)
    local enabled=$(get_probe_field $channel enabled)
    
    if [ "$enabled" != "true" ]; then
        log_message "Wybierz aktywną końcówkę!"
        return
    fi
    
    # Cykliczna zmiana modulacji
    case $current_mod in
        NONE) local new_mod="AM" ;;
        AM) local new_mod="FM" ;;
        FM) local new_mod="BURST" ;;
        BURST) local new_mod="SWEEP" ;;
        SWEEP) local new_mod="NONE" ;;
        *) local new_mod="NONE" ;;
    esac
    
    set_probe_field $channel modulation "$new_mod"
    log_message "Modulacja: $(get_modulation_name $new_mod)"
    send_probe_config $channel
}

select_probe_mode() {
    local modes=("SINGLE" "DUAL_INDEPENDENT" "DUAL_SYNC" "MULTI_CHANNEL" "SEQUENTIAL")
    local current_idx=0
    
    # Znajdź obecny tryb
    for i in "${!modes[@]}"; do
        if [ "${modes[$i]}" = "$PROBE_MODE" ]; then
            current_idx=$i
            break
        fi
    done
    
    # Pokaż menu wyboru
    move_cursor 15 30
    printf "${COLOR_BG_BLUE}${COLOR_WHITE} --- WYBÓR TRYBU --- ${COLOR_RESET}\n"
    
    for i in "${!modes[@]}"; do
        local row=$((17 + i))
        move_cursor $row 30
        if [ $i -eq $current_idx ]; then
            printf "${COLOR_BOLD}> %-20s <${COLOR_RESET}" "${modes[$i]}"
        else
            printf "  %-20s  " "${modes[$i]}"
        fi
    done
    
    move_cursor $((17 + ${#modes[@]})) 30
    printf "${COLOR_CYAN}(N/P: wybór, Enter: zatwierdź)${COLOR_RESET}"
    
    while true; do
        read -s -n 1 key
        case $key in
            $'\x1b') # Escape sequence
                read -s -n 2 rest
                case $rest in
                    '[A') # Up
                        ((current_idx--))
                        [ $current_idx -lt 0 ] && current_idx=$((${#modes[@]} - 1))
                        ;;
                    '[B') # Down
                        ((current_idx++))
                        [ $current_idx -ge ${#modes[@]} ] && current_idx=0
                        ;;
                esac
                ;;
            '') # Enter
                PROBE_MODE="${modes[$current_idx]}"
                log_message "Tryb pracy: $PROBE_MODE"
                break
                ;;
        esac
        
        # Redraw menu
        for i in "${!modes[@]}"; do
            local row=$((17 + i))
            move_cursor $row 30
            if [ $i -eq $current_idx ]; then
                printf "${COLOR_BOLD}> %-20s <${COLOR_RESET}" "${modes[$i]}"
            else
                printf "  %-20s  " "${modes[$i]}"
            fi
        done
    done
}

start_therapy() {
    log_message "Rozpoczynanie terapii..."
    
    local active_count=0
    for channel in 1 2 3 4 5 6 7 8; do
        local enabled=$(get_probe_field $channel enabled)
        if [ "$enabled" = "true" ]; then
            ((active_count++))
            send_probe_config $channel
        fi
    done
    
    if [ $active_count -eq 0 ]; then
        log_message "BRAK AKTYWNYCH KOŃCÓWEK!"
        return
    fi
    
    send_command "t"
    log_message "Terapia rozpoczęta! ($active_count kanałów)"
}

stop_therapy() {
    log_message "Zatrzymywanie terapii..."
    send_command "x"
    log_message "Terapia zatrzymana"
}

# ============================================================================
# OBSŁUGA WEJŚCIA
# ============================================================================

handle_input() {
    read -s -n 1 -t $REFRESH_RATE key
    
    if [ -z "$key" ]; then
        return
    fi
    
    case $key in
        $'\x1b') # Escape sequence
            read -s -n 2 -t 0.1 rest
            case $rest in
                '[A') # Up
                    ((CURRENT_MENU--))
                    [ $CURRENT_MENU -lt 0 ] && CURRENT_MENU=7
                    ;;
                '[B') # Down
                    ((CURRENT_MENU++))
                    [ $CURRENT_MENU -gt 7 ] && CURRENT_MENU=0
                    ;;
                '[D') # Left
                    ((SELECTED_PROBE--))
                    [ $SELECTED_PROBE -lt 1 ] && SELECTED_PROBE=8
                    ;;
                '[C') # Right
                    ((SELECTED_PROBE++))
                    [ $SELECTED_PROBE -gt 8 ] && SELECTED_PROBE=1
                    ;;
            esac
            ;;
        'q'|'Q')
            RUNNING=false
            ;;
        '1')
            edit_frequency $SELECTED_PROBE
            ;;
        '2')
            select_probe_mode
            ;;
        '3')
            start_therapy
            ;;
        '4')
            stop_therapy
            ;;
        '5')
            request_status
            log_message "Żądanie statusu..."
            ;;
        'e'|'E')
            toggle_probe $SELECTED_PROBE
            ;;
        'f'|'F')
            edit_frequency $SELECTED_PROBE
            ;;
        'i'|'I')
            edit_intensity $SELECTED_PROBE
            ;;
        'm'|'M')
            edit_modulation $SELECTED_PROBE
            ;;
        's'|'S')
            request_status
            ;;
        'h'|'H')
            draw_help
            ;;
    esac
}

# ============================================================================
# CLEANUP
# ============================================================================

cleanup() {
    show_cursor
    disconnect_from_device
    clear_screen
    printf "${COLOR_RESET}"
    move_cursor 1 1
    echo "Dziękujemy za użycie ResoNet-Nano TUI!"
}

trap cleanup EXIT INT TERM

# ============================================================================
# MAIN
# ============================================================================

show_help() {
    echo "ResoNet-Nano Bioresonance TUI (BASH) v1.0"
    echo ""
    echo "Usage: $0 [OPTIONS] <ip_address> [port]"
    echo ""
    echo "Options:"
    echo "  -h, --help      Show this help message and exit"
    echo "  -v, --verbose   Enable verbose output (debug mode)"
    echo "  -d, --debug     Enable debug mode (same as verbose)"
    echo ""
    echo "Arguments:"
    echo "  ip_address      IP address of the Arduino Nano device (default: 192.168.1.100)"
    echo "  port            TCP port for communication (default: $DEFAULT_PORT)"
    echo ""
    echo "Controls:"
    echo "  UP/DOWN         Menu navigation"
    echo "  LEFT/RIGHT      Probe selection"
    echo "  1-8             Quick actions"
    echo "  E               Toggle probe on/off"
    echo "  F               Edit frequency"
    echo "  I               Edit intensity"
    echo "  M               Edit modulation"
    echo "  S               Refresh status"
    echo "  H               Help"
    echo "  Q               Quit"
    echo ""
    echo "Examples:"
    echo "  $0                          # Use default IP and port"
    echo "  $0 192.168.1.50             # Connect to specific IP"
    echo "  $0 192.168.1.50 5002        # Connect to specific IP and port"
    echo "  $0 -v 192.168.1.50          # Verbose mode with specific IP"
    echo "  $0 --debug                  # Debug mode with defaults"
    echo ""
}

main() {
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                VERBOSE=true
                DEBUG=true
                shift
                ;;
            -d|--debug)
                DEBUG=true
                VERBOSE=true
                shift
                ;;
            -*)
                error_log "Nieznana opcja: $1"
                echo "Użyj '$0 --help' aby uzyskać pomoc."
                exit 1
                ;;
            *)
                # Pozycyjne argumenty
                if [ -z "$DEVICE_IP" ]; then
                    DEVICE_IP="$1"
                elif [ -z "$DEVICE_PORT" ]; then
                    DEVICE_PORT="$1"
                else
                    error_log "Za dużo argumentów: $1"
                    exit 1
                fi
                shift
                ;;
        esac
    done
    
    # Set defaults if not provided
    if [ -z "$DEVICE_IP" ]; then
        DEVICE_IP="192.168.1.100"
        info_log "Użyto domyślnego IP: $DEVICE_IP"
    fi
    
    if [ -z "$DEVICE_PORT" ]; then
        DEVICE_PORT=$DEFAULT_PORT
        info_log "Użyto domyślnego portu: $DEVICE_PORT"
    fi
    
    debug_log "Starting TUI with DEVICE_IP=$DEVICE_IP, DEVICE_PORT=$DEVICE_PORT"
    debug_log "VERBOSE=$VERBOSE, DEBUG=$DEBUG"
    
    # Welcome message
    clear_screen
    echo "=== ResoNet-Nano Bioresonance TUI (BASH) ==="
    echo "Connecting to: $DEVICE_IP:$DEVICE_PORT"
    if [ "$VERBOSE" = true ]; then
        echo "Verbose mode: ENABLED"
    fi
    echo ""
    echo "Controls:"
    echo "  UP/DOWN   - Menu navigation"
    echo "  LEFT/RIGHT - Probe selection"
    echo "  1-8       - Quick actions"
    echo "  E         - Toggle probe"
    echo "  F         - Edit frequency"
    echo "  I         - Edit intensity"
    echo "  M         - Edit modulation"
    echo "  S         - Refresh status"
    echo "  H         - Help"
    echo "  Q         - Quit"
    echo ""
    echo "Press Enter to continue..."
    read
    
    # Initialize
    hide_cursor
    clear_screen
    log_message "Uruchomiono TUI"
    info_log "TUI initialized successfully"
    
    # Connect
    connect_to_device
    
    # Main loop
    while [ "$RUNNING" = true ]; do
        refresh_display
        handle_input
        
        # Periodic status update
        if [ "$CONNECTED" = true ]; then
            local response=$(request_status 2>/dev/null)
            if [ -n "$response" ]; then
                parse_status_response "$response"
            fi
        fi
    done
}

# Start application
main "$@"
