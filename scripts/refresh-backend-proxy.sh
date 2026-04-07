#!/usr/bin/env bash

set -euo pipefail

POWERSHELL_BIN="$(command -v powershell.exe || true)"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WINDOWS_SCRIPT_PATH="$(wslpath -w "$SCRIPT_DIR/refresh-backend-proxy.ps1")"

if [ -z "$POWERSHELL_BIN" ]; then
    echo "powershell.exe not found in WSL PATH."
    exit 1
fi

echo
echo "========================================"
echo "ESP32 backend proxy refresh"
echo "========================================"
echo "A Windows admin prompt will open to refresh portproxy for backend port 8000."
echo

"$POWERSHELL_BIN" -NoProfile -ExecutionPolicy Bypass -Command \
    "Start-Process PowerShell -Verb RunAs -Wait -ArgumentList @('-NoProfile','-ExecutionPolicy','Bypass','-NoExit','-File','$WINDOWS_SCRIPT_PATH')"
