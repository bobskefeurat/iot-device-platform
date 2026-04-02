#!/usr/bin/env bash

set -euo pipefail

USBIPD_BIN="$(command -v usbipd.exe || true)"

if [ -z "$USBIPD_BIN" ]; then
    echo "usbipd.exe not found in WSL PATH."
    exit 1
fi

echo
echo "========================================"
echo "ESP32 USB reconnect"
echo "========================================"
echo
echo "Available USB devices:"
"$USBIPD_BIN" list
echo

read -rp "Enter BUSID to attach to WSL: " BUSID

if [ -z "${BUSID:-}" ]; then
    echo "No BUSID entered. Aborting."
    exit 1
fi

echo
echo "Binding $BUSID..."
if ! "$USBIPD_BIN" bind --busid "$BUSID"; then
    echo "Bind failed or was not needed. Continuing to attach."
fi

echo "Attaching $BUSID to WSL..."
"$USBIPD_BIN" attach --wsl --busid "$BUSID"

echo
echo "Checking serial devices in WSL:"
if ! ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null; then
    echo "No /dev/ttyUSB* or /dev/ttyACM* device found yet."
fi
