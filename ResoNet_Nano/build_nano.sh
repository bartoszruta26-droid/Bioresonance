#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FQBN="${RESONET_FQBN:-arduino:avr:nano:cpu=atmega328old}"

# The Nano is a TCP diagnostic server only. Remove the unused outgoing TCP
# connection path from the pinned EthernetENC uIP stack to preserve flash.
if grep -Eq '^#define[[:space:]]+RESONET_ENABLE_ETHERNET[[:space:]]+1' "$SKETCH_DIR/network_config.h"; then
  ARDUINO_USER_DIR="${RESONET_ARDUINO_USER_DIR:-$(arduino-cli config dump 2>/dev/null | awk '$1 == "user:" {print $2; exit}')}"
  ARDUINO_USER_DIR="${ARDUINO_USER_DIR:-$HOME/Arduino}"
  UIP_OPTIONS="$ARDUINO_USER_DIR/libraries/EthernetENC/src/utility/uipopt.h"
  if [[ ! -f "$UIP_OPTIONS" ]]; then
    echo "EthernetENC uipopt.h not found: $UIP_OPTIONS" >&2
    echo "Set RESONET_ARDUINO_USER_DIR to the Arduino user directory." >&2
    exit 1
  fi
  if grep -Fq '#define UIP_ACTIVE_OPEN 1' "$UIP_OPTIONS"; then
    sed -i 's/^#define UIP_ACTIVE_OPEN 1$/#define UIP_ACTIVE_OPEN 0/' "$UIP_OPTIONS"
  fi
  if ! grep -Fq '#define UIP_ACTIVE_OPEN 0' "$UIP_OPTIONS"; then
    echo "Could not apply server-only UIP_ACTIVE_OPEN=0 patch safely." >&2
    exit 1
  fi
  echo "EthernetENC: UIP_ACTIVE_OPEN=0 (inbound TCP server only)"
fi

MEMORY_FLAGS="-mcall-prologues -DUIP_CONF_UDP=0 -DUIP_CONF_MAX_CONNECTIONS=1 -DUIP_SOCKET_NUMPACKETS=1 -DSERIAL_RX_BUFFER_SIZE=32 -DSERIAL_TX_BUFFER_SIZE=32"

arduino-cli compile \
  --fqbn "$FQBN" \
  --build-property "compiler.cpp.extra_flags=$MEMORY_FLAGS" \
  --build-property "compiler.c.extra_flags=$MEMORY_FLAGS" \
  "$SKETCH_DIR"
