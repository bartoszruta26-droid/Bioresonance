#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FQBN="${RESONET_FQBN:-arduino:avr:nano:cpu=atmega328old}"
# Arduino AVR platform already supplies -Os, LTO, function/data sections
# and --gc-sections. Add only AVR size-oriented prologues; do not replace
# the platform's normal optimization/linker settings.
MEMORY_FLAGS="-mcall-prologues -maccumulate-args -fshort-enums -fno-rtti -fno-exceptions -DUIP_CONF_UDP=0 -DUIP_CONF_MAX_CONNECTIONS=1 -DUIP_CONF_MAX_LISTENPORTS=1 -DUIP_CONF_BUFFER_SIZE=128 -DUIP_CONF_ARPTAB_SIZE=2 -DUIP_SOCKET_NUMPACKETS=1 -DSERIAL_RX_BUFFER_SIZE=32 -DSERIAL_TX_BUFFER_SIZE=32"

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
  --build-property "compiler.cpp.extra_flags=$MEMORY_FLAGS" \
  --build-property "compiler.c.extra_flags=$MEMORY_FLAGS" \
  --build-property "compiler.c.elf.extra_flags=$LINK_FLAGS" \
  "$SKETCH_DIR"
