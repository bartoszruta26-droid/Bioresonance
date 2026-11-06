#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FQBN="${RESONET_FQBN:-arduino:avr:nano:cpu=atmega328old}"

# Nano is an inbound TCP diagnostic server; outbound TCP is unused.
if grep -Eq '^#define[[:space:]]+RESONET_ENABLE_ETHERNET[[:space:]]+1' "$SKETCH_DIR/network_config.h"; then
  ARDUINO_USER_DIR="${RESONET_ARDUINO_USER_DIR:-$(arduino-cli config dump 2>/dev/null | awk '$1 == "user:" {print $2; exit}')}"
  ARDUINO_USER_DIR="${ARDUINO_USER_DIR:-$HOME/Arduino}"
  ETHERNETENC_DIR="$ARDUINO_USER_DIR/libraries/EthernetENC"
  UIP_OPTIONS="$ETHERNETENC_DIR/src/utility/uipopt.h"

  if [[ ! -f "$UIP_OPTIONS" || ! -f "$ETHERNETENC_DIR/library.properties" ]] ||
     ! grep -Eq '^version=2\.0\.5$' "$ETHERNETENC_DIR/library.properties"; then
    echo "Expected EthernetENC 2.0.5 not found under $ARDUINO_USER_DIR/libraries." >&2
    exit 1
  fi

  UIP_BACKUP="$(mktemp)"
  cp "$UIP_OPTIONS" "$UIP_BACKUP"
  restore_uip_options() {
    cp "$UIP_BACKUP" "$UIP_OPTIONS"
    rm -f "$UIP_BACKUP"
  }
  trap restore_uip_options EXIT

  if grep -Fq '#define UIP_ACTIVE_OPEN 1' "$UIP_OPTIONS"; then
    sed -i 's/^#define UIP_ACTIVE_OPEN 1$/#define UIP_ACTIVE_OPEN 0/' "$UIP_OPTIONS"
  fi
  if ! grep -Fq '#define UIP_ACTIVE_OPEN 0' "$UIP_OPTIONS"; then
    echo "Could not apply server-only UIP_ACTIVE_OPEN=0 patch safely." >&2
    exit 1
  fi
  echo "EthernetENC 2.0.5: UIP_ACTIVE_OPEN=0 (inbound TCP only)"
fi

MEMORY_FLAGS="-mcall-prologues -maccumulate-args -DUIP_CONF_UDP=0 -DUIP_CONF_MAX_CONNECTIONS=1 -DUIP_CONF_MAX_LISTENPORTS=1 -DUIP_CONF_BUFFER_SIZE=128 -DUIP_CONF_ARPTAB_SIZE=2 -DUIP_SOCKET_NUMPACKETS=1 -DSERIAL_RX_BUFFER_SIZE=32 -DSERIAL_TX_BUFFER_SIZE=32"
CPP_FLAGS="$MEMORY_FLAGS -fno-rtti -fno-exceptions"

LINK_FLAGS="-Wl,--relax"
if [[ -n "${RESONET_MAP_FILE:-}" ]]; then
  LINK_FLAGS="$LINK_FLAGS -Wl,-Map=${RESONET_MAP_FILE}"
fi
BUILD_ARGS=()
if [[ -n "${RESONET_BUILD_DIR:-}" ]]; then
  mkdir -p "$RESONET_BUILD_DIR"
  BUILD_ARGS+=(--build-path "$RESONET_BUILD_DIR")
fi

arduino-cli compile \
  --fqbn "$FQBN" \
  "${BUILD_ARGS[@]}" \
  --build-property "compiler.cpp.extra_flags=$CPP_FLAGS" \
  --build-property "compiler.c.extra_flags=$MEMORY_FLAGS" \
  --build-property "compiler.c.elf.extra_flags=$LINK_FLAGS" \
  "$SKETCH_DIR"
