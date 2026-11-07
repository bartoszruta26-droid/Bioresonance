#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FQBN="${RESONET_FQBN:-arduino:avr:nano:cpu=atmega328old}"

# EthernetENC 2.0.5's EthernetClient.cpp defines connect() and references
# uip_connect() unconditionally. Disabling UIP_ACTIVE_OPEN removes that symbol
# from uip.c and causes a linker error, even though this firmware only accepts
# inbound connections. Keep the library default enabled; the application does
# not call EthernetClient::connect(), and the linker can discard unused code.

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
