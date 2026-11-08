#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FQBN="${RESONET_FQBN:-arduino:avr:nano:cpu=atmega328old}"

# The firmware is a TCP server only. Remove EthernetENC's unused outbound
# EthernetClient::connect() definitions before compiling, then disable uIP
# active-open support. This avoids retaining outbound TCP connection code.
ETHERNET_ENABLED=$(grep -E '^#define[[:space:]]+RESONET_ENABLE_ETHERNET[[:space:]]+1' "$SKETCH_DIR/network_config.h" || true)
LIBRARY_CLIENT=""
LIBRARY_BACKUP=""
restore_ethernet_library() {
  if [[ -n "$LIBRARY_CLIENT" && -n "$LIBRARY_BACKUP" && -f "$LIBRARY_BACKUP" ]]; then
    cp "$LIBRARY_BACKUP" "$LIBRARY_CLIENT"
    rm -f "$LIBRARY_BACKUP"
  fi
}
if [[ -n "$ETHERNET_ENABLED" ]]; then
  LIBRARY_CLIENT="$(find "$HOME/Arduino/libraries" -path '*/EthernetENC/src/EthernetClient.cpp' -print -quit 2>/dev/null || true)"
  if [[ -z "$LIBRARY_CLIENT" ]]; then
    echo "EthernetENC EthernetClient.cpp not found; install EthernetENC 2.0.5" >&2
    exit 1
  fi
  LIBRARY_BACKUP="$(mktemp)"
  cp "$LIBRARY_CLIENT" "$LIBRARY_BACKUP"
  trap restore_ethernet_library EXIT
  python3 - "$LIBRARY_CLIENT" <<'PY'
import pathlib, sys
path = pathlib.Path(sys.argv[1])
source = path.read_text()
signature = source.find("UIPClient::connect(IPAddress ip, uint16_t port)")
if signature < 0:
    raise SystemExit("EthernetENC outbound connect signature not found; refusing unsafe patch")
start = source.rfind("int", 0, signature)
stop_signature = source.find("UIPClient::stop()", signature)
if start < 0 or stop_signature < 0:
    raise SystemExit("EthernetENC connect/stop boundaries changed; refusing unsafe patch")
end = source.rfind("void", signature, stop_signature)
if end < signature or end <= start:
    raise SystemExit("EthernetENC stop boundary not found; refusing unsafe patch")
replacement = """int
UIPClient::connect(IPAddress ip, uint16_t port)
{
  (void)ip;
  (void)port;
  return 0;
}

int
UIPClient::connect(const char *host, uint16_t port)
{
  (void)host;
  (void)port;
  return 0;
}

"""
# The server uses buffer I/O only. Keep virtual symbols for the Client vtable,
# but make unused byte I/O, peek and client comparison fail closed.
unused_methods = [
    ("""bool
UIPClient::operator==(const UIPClient& rhs)
{
  return data && rhs.data && (data == rhs.data);
}""", """bool
UIPClient::operator==(const UIPClient& rhs)
{
  (void)rhs;
  return false;
}"""),
    ("""size_t
UIPClient::write(uint8_t c)
{
  return write(&c, 1);
}""", """size_t
UIPClient::write(uint8_t c)
{
  (void)c;
  return 0;
}"""),
    ("""int
UIPClient::read()
{
  uint8_t c;
  if (read(&c,1) <= 0)
    return -1;
  return c;
}""", """int
UIPClient::read()
{
  return -1;
}"""),
    ("""int
UIPClient::peek()
{
  if (*this)
    {
      if (data->packets_in[0] != NOBLOCK)
        {
          uint8_t c;
          Enc28J60Network::readPacket(data->packets_in[0],0,&c,1);
          return c;
        }
    }
  return -1;
}""", """int
UIPClient::peek()
{
  return -1;
}""")
]
for old_method, new_method in unused_methods:
    if old_method not in source:
        raise SystemExit("EthernetENC unused virtual method changed; refusing unsafe patch")
    source = source.replace(old_method, new_method, 1)
path.write_text(source[:start] + replacement + source[end:])

PY
fi

MEMORY_FLAGS="-mcall-prologues -maccumulate-args -DUIP_CONF_ACTIVE_OPEN=0 -DUIP_CONF_UDP=0 -DUIP_CONF_MAX_CONNECTIONS=1 -DUIP_CONF_MAX_LISTENPORTS=1 -DUIP_CONF_BUFFER_SIZE=128 -DUIP_CONF_ARPTAB_SIZE=2 -DUIP_SOCKET_NUMPACKETS=1 -DSERIAL_RX_BUFFER_SIZE=32 -DSERIAL_TX_BUFFER_SIZE=32"
# Keep warning/error diagnostics in the compact production image. Set RESONET_LOG_LEVEL=0 for full logs.
LOG_LEVEL="${RESONET_LOG_LEVEL:-3}"
if [[ ! "$LOG_LEVEL" =~ ^[0-5]$ ]]; then
  echo "RESONET_LOG_LEVEL must be an integer from 0 (verbose) to 5 (fatal)" >&2
  exit 2
fi
CPP_FLAGS="$MEMORY_FLAGS -fno-rtti -fno-exceptions -DRESONET_LOG_LEVEL=$LOG_LEVEL"

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
