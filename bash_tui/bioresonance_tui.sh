#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
if ! command -v python3 >/dev/null 2>&1; then
  echo "Wymagany jest Python 3." >&2
  exit 127
fi
exec python3 "$SCRIPT_DIR/resonet_tui.py" "$@"
