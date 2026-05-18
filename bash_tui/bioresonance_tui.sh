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

# Frequencies database loaded from frequencies.md
declare -A FREQUENCIES_BY_DISEASE
declare -a DISEASE_LIST
declare -a FREQUENCY_DATA

# Therapeutic presets database
declare -a PRESET_LIST
declare -A PRESET_DATA
declare -a PRESET_FREQUENCIES

# Therapy session management
SESSION_ACTIVE=false
SESSION_START_TIME=0
SESSION_CURRENT_STEP=0
SESSION_TOTAL_STEPS=0
SESSION_SEQUENCE=()
SESSION_STEP_START_TIME=0
SESSION_PAUSED=false

# Schedule system
declare -a SCHEDULE_ENTRIES
SCHEDULE_ENABLED=false

# Offline mode
OFFLINE_MODE=false
CACHED_STATUS=""

# ============================================================================
# FUNKCJE POMOCNICZE
# ============================================================================

# Load frequencies from frequencies.md file
load_frequencies() {
    local freq_file="/workspace/frequencies.md"
    
    # Check if running from different directory
    if [ ! -f "$freq_file" ]; then
        freq_file="$(dirname "$0")/../frequencies.md"
    fi
    if [ ! -f "$freq_file" ]; then
        freq_file="./frequencies.md"
    fi
    
    if [ ! -f "$freq_file" ]; then
        debug_log "Cannot find frequencies.md file"
        return 1
    fi
    
    debug_log "Loading frequencies from: $freq_file"
    
    # Parse frequencies.md - extract lines with format: freq_hz|category|subcategory|description|modulation|carrier_khz
    local line_num=0
    while IFS= read -r line; do
        # Skip comments, headers, and empty lines
        [[ "$line" =~ ^[[:space:]]*# ]] && continue
        [[ "$line" =~ ^[[:space:]]*$ ]] && continue
        [[ "$line" =~ ^--- ]] && continue
        [[ "$line" =~ ^=== ]] && continue
        [[ "$line" =~ ^FREQUENCY_DATA ]] && continue
        [[ "$line" =~ ^freq_hz\| ]] && continue
        
        # Match frequency data lines (starting with number followed by |)
        if [[ "$line" =~ ^([0-9]+)\|([^|]+)\|([^|]+)\|([^|]+)\|([^|]+)\|([0-9]+) ]]; then
            local freq="${BASH_REMATCH[1]}"
            local category="${BASH_REMATCH[2]}"
            local subcategory="${BASH_REMATCH[3]}"
            local description="${BASH_REMATCH[4]}"
            local modulation="${BASH_REMATCH[5]}"
            local carrier="${BASH_REMATCH[6]}"
            
            # Extract disease name from description (before the / separator)
            local disease_name=$(echo "$description" | cut -d'/' -f1 | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
            
            # Store in arrays
            DISEASE_LIST+=("$disease_name ($freq Hz)")
            FREQUENCY_DATA+=("$freq|$category|$subcategory|$description|$modulation|$carrier")
            
            ((line_num++))
        fi
    done < "$freq_file"
    
    debug_log "Loaded $line_num frequencies"
    return 0
}

# Load therapeutic presets from therapeutic_presets.md file
load_presets() {
    local preset_file="/workspace/therapeutic_presets.md"
    
    # Check if running from different directory
    if [ ! -f "$preset_file" ]; then
        preset_file="$(dirname "$0")/../therapeutic_presets.md"
    fi
    if [ ! -f "$preset_file" ]; then
        preset_file="./therapeutic_presets.md"
    fi
    
    if [ ! -f "$preset_file" ]; then
        debug_log "Cannot find therapeutic_presets.md file"
        return 1
    fi
    
    debug_log "Loading presets from: $preset_file"
    
    # Parse therapeutic_presets.md
    local preset_count=0
    local in_data=false
    while IFS= read -r line; do
        # Skip comments, headers, and empty lines
        [[ "$line" =~ ^[[:space:]]*# ]] && continue
        [[ "$line" =~ ^[[:space:]]*$ ]] && continue
        [[ "$line" =~ ^--- ]] && continue
        
        # Check for data section markers
        if [[ "$line" == "PRESET_DATA_START" ]]; then
            in_data=true
            continue
        fi
        if [[ "$line" == "PRESET_DATA_END" ]]; then
            in_data=false
            continue
        fi
        if [[ "$line" =~ ^preset_name\| ]]; then
            continue  # Skip header line
        fi
        
        # Parse preset data lines only when in data section
        if [ "$in_data" = true ] && [[ "$line" =~ ^([^|]+)\|([0-9]+)\|([^|]+)\|([^|]+)\|(.+)$ ]]; then
            local preset_name="${BASH_REMATCH[1]}"
            local duration_min="${BASH_REMATCH[2]}"
            local frequencies="${BASH_REMATCH[3]}"
            local category="${BASH_REMATCH[4]}"
            local description="${BASH_REMATCH[5]}"
            
            # Store preset info
            PRESET_LIST+=("$preset_name")
            PRESET_DATA["${preset_name}_duration"]="$duration_min"
            PRESET_DATA["${preset_name}_frequencies"]="$frequencies"
            PRESET_DATA["${preset_name}_category"]="$category"
            PRESET_DATA["${preset_name}_description"]="$description"
            
            ((preset_count++))
        fi
    done < "$preset_file"
    
    debug_log "Loaded $preset_count therapeutic presets"
    return 0
}

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
            # Validate duty cycle is numeric and in range 0-100
            if [[ ! "$value" =~ ^[0-9]+$ ]]; then
                error_log "Duty cycle must be numeric: $value"
                return 1
            fi
            if [ "$value" -lt 0 ] || [ "$value" -gt 100 ]; then
                error_log "Duty cycle out of range (0-100): $value"
                return 1
            fi
            duty="$value" 
            ;;
        intensity) 
            # Validate intensity is numeric and in range 0-4095
            if [[ ! "$value" =~ ^[0-9]+$ ]]; then
                error_log "Intensity must be numeric: $value"
                return 1
            fi
            if [ "$value" -lt 0 ] || [ "$value" -gt 4095 ]; then
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
    printf "  P   : Wybierz program terapii (choroba)\n"
    printf "  I   : Edycja intensywności\n"
    printf "  M   : Zmiana modulacji\n"
    printf "  S   : Odśwież status\n"
    printf "  H   : Pomoc\n"
    printf "  Q   : Wyjście\n"
    
    move_cursor $((start_row + 14)) $start_col
    printf "${COLOR_CYAN}Naciśnij dowolny klawisz...${COLOR_RESET}"
    read -n 1 -s
}

# Select therapy program by disease from frequencies.md
select_therapy_program() {
    local channel=$SELECTED_PROBE
    
    if [ ${#DISEASE_LIST[@]} -eq 0 ]; then
        log_message "Brak wczytanych programów terapii!"
        return
    fi
    
    # Enable the selected probe first
    set_probe_field $channel enabled "true"
    
    local current_idx=0
    local visible_count=15
    local scroll_offset=0
    
    while true; do
        clear_screen
        move_cursor 2 2
        printf "${COLOR_BOLD}${COLOR_BG_BLUE} --- WYBÓR PROGRAMU TERAPII --- ${COLOR_RESET}\n"
        move_cursor 3 2
        printf "${COLOR_CYAN}(Kanał %d) Strzałki: nawigacja, Enter: wybierz, Q: anuluj${COLOR_RESET}\n" "$channel"
        
        # Display list with scrolling
        local display_idx=0
        for ((i=scroll_offset; i<${#DISEASE_LIST[@]} && display_idx<visible_count; i++)); do
            local row=$((5 + display_idx))
            move_cursor $row 2
            
            if [ $i -eq $current_idx ]; then
                printf "${COLOR_BOLD}${COLOR_GREEN}> %-70s${COLOR_RESET}" "${DISEASE_LIST[$i]}"
            else
                printf "  %-70s" "${DISEASE_LIST[$i]}"
            fi
            ((display_idx++))
        done
        
        # Show position info
        move_cursor $((5 + visible_count + 1)) 2
        printf "${COLOR_YELLOW}Pozycja: %d / %d${COLOR_RESET}" $((current_idx + 1)) ${#DISEASE_LIST[@]}
        
        # Read input
        read -s -n 1 key
        case $key in
            $'\x1b')
                read -s -n 2 rest
                case $rest in
                    '[A') # Up
                        ((current_idx--))
                        [ $current_idx -lt 0 ] && current_idx=0
                        [ $current_idx -lt $scroll_offset ] && scroll_offset=$current_idx
                        ;;
                    '[B') # Down
                        ((current_idx++))
                        [ $current_idx -ge ${#DISEASE_LIST[@]} ] && current_idx=$((${#DISEASE_LIST[@]} - 1))
                        [ $current_idx -ge $((scroll_offset + visible_count)) ] && scroll_offset=$((current_idx - visible_count + 1))
                        ;;
                esac
                ;;
            '') # Enter
                # Apply selected frequency to current channel
                local freq_data="${FREQUENCY_DATA[$current_idx]}"
                local freq=$(echo "$freq_data" | cut -d'|' -f1)
                local modulation=$(echo "$freq_data" | cut -d'|' -f5)
                
                set_probe_field $channel freq "$freq"
                set_probe_field $channel modulation "$modulation"
                
                log_message "Program: ${DISEASE_LIST[$current_idx]}"
                send_probe_config $channel
                break
                ;;
            q|Q)
                break
                ;;
        esac
    done
}

# Select therapeutic preset (multi-frequency sequence)
select_therapeutic_preset() {
    if [ ${#PRESET_LIST[@]} -eq 0 ]; then
        log_message "Brak wczytanych presetów terapeutycznych!"
        return
    fi
    
    local current_idx=0
    local visible_count=15
    local scroll_offset=0
    
    while true; do
        clear_screen
        move_cursor 2 2
        printf "${COLOR_BOLD}${COLOR_BG_BLUE} --- PRESETY TERAPEUTYCZNE --- ${COLOR_RESET}\n"
        move_cursor 3 2
        printf "${COLOR_CYAN}Strzałki: nawigacja, Enter: wybierz, D: szczegóły, Q: anuluj${COLOR_RESET}\n"
        
        # Display list with scrolling
        local display_idx=0
        for ((i=scroll_offset; i<${#PRESET_LIST[@]} && display_idx<visible_count; i++)); do
            local row=$((5 + display_idx))
            move_cursor $row 2
            local preset_name="${PRESET_LIST[$i]}"
            local duration="${PRESET_DATA[${preset_name}_duration]}"
            
            if [ $i -eq $current_idx ]; then
                printf "${COLOR_BOLD}${COLOR_GREEN}> %-50s [%d min]${COLOR_RESET}" "$preset_name" "$duration"
            else
                printf "  %-50s [%d min]" "$preset_name" "$duration"
            fi
            ((display_idx++))
        done
        
        # Show position info
        move_cursor $((5 + visible_count + 1)) 2
        printf "${COLOR_YELLOW}Pozycja: %d / %d${COLOR_RESET}" $((current_idx + 1)) ${#PRESET_LIST[@]}"
        
        # Read input
        read -s -n 1 key
        case $key in
            $'\x1b')
                read -s -n 2 rest
                case $rest in
                    '[A') # Up
                        ((current_idx--))
                        [ $current_idx -lt 0 ] && current_idx=0
                        [ $current_idx -lt $scroll_offset ] && scroll_offset=$current_idx
                        ;;
                    '[B') # Down
                        ((current_idx++))
                        [ $current_idx -ge ${#PRESET_LIST[@]} ] && current_idx=$((${#PRESET_LIST[@]} - 1))
                        [ $current_idx -ge $((scroll_offset + visible_count)) ] && scroll_offset=$((current_idx - visible_count + 1))
                        ;;
                esac
                ;;
            '') # Enter - Start sequential session
                local preset_name="${PRESET_LIST[$current_idx]}"
                start_sequential_session "$preset_name"
                break
                ;;
            d|D) # Details
                local preset_name="${PRESET_LIST[$current_idx]}"
                show_preset_details "$preset_name"
                ;;
            q|Q)
                break
                ;;
        esac
    done
}

# Show preset details
show_preset_details() {
    local preset_name="$1"
    
    clear_screen
    move_cursor 2 2
    printf "${COLOR_BOLD}${COLOR_BG_BLUE} --- SZCZEGÓŁY PRESETU: %s --- ${COLOR_RESET}\n" "$preset_name"
    
    local duration="${PRESET_DATA[${preset_name}_duration]}"
    local frequencies="${PRESET_DATA[${preset_name}_frequencies]}"
    local category="${PRESET_DATA[${preset_name}_category]}"
    local description="${PRESET_DATA[${preset_name}_description]}"
    
    move_cursor 4 2
    printf "${COLOR_CYAN}Czas trwania: ${COLOR_WHITE}%d minut${COLOR_RESET}\n" "$duration"
    move_cursor 5 2
    printf "${COLOR_CYAN}Kategoria: ${COLOR_WHITE}%s${COLOR_RESET}\n" "$category"
    move_cursor 6 2
    printf "${COLOR_CYAN}Opis: ${COLOR_WHITE}%s${COLOR_RESET}\n" "$description"
    
    move_cursor 8 2
    printf "${COLOR_BOLD}Sekwencja częstotliwości:${COLOR_RESET}\n"
    
    local row=9
    IFS=';' read -ra FREQ_PAIRS <<< "$frequencies"
    for pair in "${FREQ_PAIRS[@]}"; do
        local freq=$(echo "$pair" | cut -d',' -f1)
        local dur=$(echo "$pair" | cut -d',' -f2)
        move_cursor $row 4
        printf "${COLOR_GREEN}• %s Hz przez %d sekund${COLOR_RESET}\n" "$freq" "$dur"
        ((row++))
    done
    
    move_cursor $((row + 2)) 2
    printf "${COLOR_YELLOW}Naciśnij dowolny klawisz...${COLOR_RESET}"
    read -n 1 -s
}

# Start sequential therapy session
start_sequential_session() {
    local preset_name="$1"
    
    if [ -z "${PRESET_DATA[${preset_name}_frequencies]+x}" ]; then
        error_log "Nieznany preset: $preset_name"
        return
    fi
    
    local frequencies="${PRESET_DATA[${preset_name}_frequencies]}"
    local duration="${PRESET_DATA[${preset_name}_duration]}"
    
    # Parse frequency sequence
    SESSION_SEQUENCE=()
    IFS=';' read -ra FREQ_PAIRS <<< "$frequencies"
    for pair in "${FREQ_PAIRS[@]}"; do
        SESSION_SEQUENCE+=("$pair")
    done
    
    SESSION_TOTAL_STEPS=${#SESSION_SEQUENCE[@]}
    SESSION_CURRENT_STEP=0
    SESSION_ACTIVE=true
    SESSION_PAUSED=false
    SESSION_START_TIME=$(date +%s)
    SESSION_STEP_START_TIME=$SESSION_START_TIME
    
    log_message "Rozpoczęto sesję sekwencyjną: $preset_name "
    
    # Configure first channel and start
    apply_session_step
    
    # Send START command to Arduino
    send_command "START"
}

# Apply current session step
apply_session_step() {
    if [ $SESSION_CURRENT_STEP -ge ${#SESSION_SEQUENCE[@]} ]; then
        end_session
        return
    fi
    
    local pair="${SESSION_SEQUENCE[$SESSION_CURRENT_STEP]}"
    local freq=$(echo "$pair" | cut -d',' -f1)
    local duration=$(echo "$pair" | cut -d',' -f2)
    
    # Configure channel 1 with current frequency
    set_probe_field 1 freq "$freq"
    set_probe_field 1 enabled "true"
    send_probe_config 1
    
    SESSION_STEP_START_TIME=$(date +%s)
    
    log_message "Krok ${SESSION_CURRENT_STEP}: ${freq} Hz"
}

# Update session state (called in main loop)
update_session() {
    if [ "$SESSION_ACTIVE" = false ] || [ "$SESSION_PAUSED" = true ]; then
        return
    fi
    
    if [ $SESSION_CURRENT_STEP -ge ${#SESSION_SEQUENCE[@]} ]; then
        end_session
        return
    fi
    
    local pair="${SESSION_SEQUENCE[$SESSION_CURRENT_STEP]}"
    local duration=$(echo "$pair" | cut -d',' -f2)
    local now=$(date +%s)
    local elapsed=$((now - SESSION_STEP_START_TIME))
    
    if [ $elapsed -ge $duration ]; then
        # Move to next step
        ((SESSION_CURRENT_STEP++))
        if [ $SESSION_CURRENT_STEP -lt ${#SESSION_SEQUENCE[@]} ]; then
            apply_session_step
        else
            end_session
        fi
    fi
}

# End current session
end_session() {
    SESSION_ACTIVE=false
    send_command "STOP"
    
    local total_time=$(($(date +%s) - SESSION_START_TIME))
    log_message "Sesja zakończona. Czas całkowity: ${total_time}s"
    
    # Disable all channels
    for ch in 1 2 3 4 5 6 7 8; do
        set_probe_field $ch enabled "false"
    done
}

# Pause/Resume session
toggle_session_pause() {
    if [ "$SESSION_ACTIVE" = false ]; then
        return
    fi
    
    if [ "$SESSION_PAUSED" = true ]; then
        SESSION_PAUSED=false
        SESSION_STEP_START_TIME=$(($(date +%s) - $(date +%s -d "$elapsed seconds ago")))
        log_message "Wznowiono sesję"
    else
        SESSION_PAUSED=true
        log_message "Sesja wstrzymana"
    fi
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
# TRYB BEZPOŚREDNI (DIRECT CONTROL MODE)
# ============================================================================

parse_control_arg() {
    local control="$1"
    local channel freq duty intensity modulation
    
    # Parse channel:freq[:duty:intensity:modulation]
    IFS=':' read -r channel freq duty intensity modulation <<< "$control"
    
    # Validate channel
    if [ -z "$channel" ] || [[ ! "$channel" =~ ^[1-8]$ ]]; then
        error_log "Invalid channel: $channel (must be 1-8)"
        return 1
    fi
    
    # Validate frequency (numeric, can be decimal)
    if [ -z "$freq" ] || [[ ! "$freq" =~ ^[0-9]+\.?[0-9]*$ ]]; then
        error_log "Invalid frequency: $freq"
        return 1
    fi
    
    # Set defaults
    duty=${duty:-50}
    intensity=${intensity:-2048}
    modulation=${modulation:-NONE}
    
    # Validate duty cycle is numeric and in range 0-100
    if [[ ! "$duty" =~ ^[0-9]+$ ]]; then
        error_log "Duty cycle must be numeric: $duty"
        return 1
    fi
    if [ "$duty" -lt 0 ] || [ "$duty" -gt 100 ]; then
        error_log "Duty cycle out of range (0-100): $duty"
        return 1
    fi
    
    # Validate intensity is numeric and in range 0-4095
    if [[ ! "$intensity" =~ ^[0-9]+$ ]]; then
        error_log "Intensity must be numeric: $intensity"
        return 1
    fi
    if [ "$intensity" -lt 0 ] || [ "$intensity" -gt 4095 ]; then
        error_log "Intensity out of range (0-4095): $intensity"
        return 1
    fi
    
    # Validate modulation type
    case "$modulation" in
        NONE|AM|FM|BURST|SWEEP) ;;
        *)
            error_log "Invalid modulation: $modulation (must be NONE, AM, FM, BURST, or SWEEP)"
            return 1
            ;;
    esac
    
    # Return parsed values via global variables
    PARSED_CHANNEL="$channel"
    PARSED_FREQ="$freq"
    PARSED_DUTY="$duty"
    PARSED_INTENSITY="$intensity"
    PARSED_MODULATION="$modulation"
    
    debug_log "Parsed control: channel=$channel, freq=$freq, duty=$duty, intensity=$intensity, modulation=$modulation"
    return 0
}

run_direct_control() {
    info_log "Running in Direct Control Mode"
    
    # First, validate all control arguments before connecting
    local validated_configs=()
    for control in "${DIRECT_CONTROLS[@]}"; do
        if ! parse_control_arg "$control"; then
            echo "  ✗ Failed to parse: $control"
            return 1
        fi
        # Store validated config
        validated_configs+=("$PARSED_CHANNEL:$PARSED_FREQ:$PARSED_DUTY:$PARSED_INTENSITY:$PARSED_MODULATION")
    done
    
    # Connect to device
    if ! connect_to_device; then
        error_log "Failed to connect to device"
        return 1
    fi
    
    local success_count=0
    local fail_count=0
    local idx=0
    
    for control in "${DIRECT_CONTROLS[@]}"; do
        echo "Processing control: $control"
        
        # Re-parse to set global variables (we already validated above)
        parse_control_arg "$control"
        
        # Update probe configuration
        set_probe_field "$PARSED_CHANNEL" freq "$PARSED_FREQ"
        set_probe_field "$PARSED_CHANNEL" duty "$PARSED_DUTY"
        set_probe_field "$PARSED_CHANNEL" intensity "$PARSED_INTENSITY"
        set_probe_field "$PARSED_CHANNEL" modulation "$PARSED_MODULATION"
        set_probe_field "$PARSED_CHANNEL" enabled "true"
        
        # Send configuration to device
        if send_probe_config "$PARSED_CHANNEL"; then
            echo "  ✓ Channel $PARSED_CHANNEL configured: ${PARSED_FREQ} Hz, ${PARSED_DUTY}% duty, intensity ${PARSED_INTENSITY}, ${PARSED_MODULATION}"
            ((success_count++))
        else
            echo "  ✗ Failed to send config for channel $PARSED_CHANNEL"
            ((fail_count++))
        fi
        ((idx++))
    done
    
    # Start therapy if any configurations succeeded
    if [ $success_count -gt 0 ]; then
        echo ""
        echo "Starting therapy with $success_count channel(s)..."
        send_command "t"
        echo "Therapy started successfully."
    fi
    
    echo ""
    echo "Summary: $success_count successful, $fail_count failed"
    
    # Disconnect
    disconnect_from_device
    
    if [ $fail_count -gt 0 ]; then
        return 1
    fi
    return 0
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
        'p'|'P')
            select_therapeutic_preset
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
    echo "Direct Control Mode (bypasses TUI):"
    echo "  Use -c or --control followed by channel:frequency[:duty:intensity:modulation]"
    echo "  Example: -c 1:727:50:2048:NONE"
    echo "  Channels: 1-8 (see probe configurations below)"
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
    echo "Probe Configurations (Channels):"
    echo "  1 - Flat Coil (Cewka Płaska)          Default: 727 Hz"
    echo "  2 - Ferrite Rod (Cewka Ferrytowa)     Default: 10000 Hz"
    echo "  3 - Capacitive Plate (Płyta Kapacyt.) Default: 5000 Hz"
    echo "  4 - Pen Applicator (Aplikator Punkt.) Default: 25000 Hz"
    echo "  5 - EMF Mat (Mata EMF)                Default: 78.3 Hz"
    echo "  6 - Local Pad (Podkładka Lokalna)     Default: 1000 Hz"
    echo "  7 - Ring Applicator (Pierścień)       Default: 500 Hz"
    echo "  8 - Custom (Niestandardowa)           Default: 10 Hz"
    echo ""
    echo "Examples:"
    echo "  TUI Mode:"
    echo "    $0                          # Use default IP and port"
    echo "    $0 192.168.1.50             # Connect to specific IP"
    echo "    $0 192.168.1.50 5002        # Connect to specific IP and port"
    echo "    $0 -v 192.168.1.50          # Verbose mode with specific IP"
    echo "    $0 --debug                  # Debug mode with defaults"
    echo ""
    echo "  Direct Control Mode (no TUI):"
    echo "    $0 -c 1:727                 # Activate channel 1 at 727 Hz"
    echo "    $0 -c 1:727:50              # Channel 1, 727 Hz, 50% duty"
    echo "    $0 -c 1:727:50:2048         # Channel 1, 727 Hz, 50% duty, intensity 2048"
    echo "    $0 -c 1:727:50:2048:AM      # Channel 1 with AM modulation"
    echo "    $0 -c 2:10000 -c 3:5000     # Multiple channels at once"
    echo "    $0 192.168.1.50 -c 1:727    # Direct control with custom IP"
    echo ""
}

# Globalne zmienne do kontrolerów bezpośrednich
declare -a DIRECT_CONTROLS

main() {
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -h|--help)
                show_help
                trap - EXIT INT TERM
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
            -c|--control)
                if [ -z "$2" ]; then
                    error_log "Opcja --control wymaga argumentu w formacie channel:freq[:duty:intensity:modulation]"
                    exit 1
                fi
                DIRECT_CONTROLS+=("$2")
                shift 2
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
    
    # Obsługa trybu bezpośredniego (Direct Control Mode)
    if [ ${#DIRECT_CONTROLS[@]} -gt 0 ]; then
        run_direct_control
        exit $?
    fi
    
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
    
    # Load frequencies database
    echo "Loading frequencies from frequencies.md..."
    load_frequencies
    echo "Loaded ${#DISEASE_LIST[@]} therapy programs"
    echo ""
    
    # Load therapeutic presets
    echo "Loading therapeutic presets..."
    load_presets
    echo "Loaded ${#PRESET_LIST[@]} therapeutic presets"
    echo ""
    
    echo "Controls:"
    echo "  UP/DOWN   - Menu navigation"
    echo "  LEFT/RIGHT - Probe selection"
    echo "  1-8       - Quick actions"
    echo "  E         - Toggle probe"
    echo "  F         - Edit frequency"
    echo "  P         - Therapeutic presets (sequential sessions)"
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
