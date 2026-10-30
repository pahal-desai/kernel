```bash
#!/usr/bin/env bash

set -e

REPO="pahal-desai/kernel"
ISO="KernOS.iso"

echo "================================"
echo "       KernOS Launcher"
echo "================================"

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
else
    echo "Cannot detect your Linux distribution."
    exit 1
fi

# Check privileges
if [ "$EUID" -eq 0 ]; then
    SUDO=""
else
    SUDO="sudo"
fi

echo "[1/4] Detected OS: $PRETTY_NAME"

# Install dependencies
echo "[2/4] Installing dependencies..."

case "$ID" in
    ubuntu|debian|linuxmint|pop)
        $SUDO apt update
        $SUDO apt install -y qemu-system-x86 curl python3
        ;;

    arch|endeavouros|manjaro)
        $SUDO pacman -Sy --needed qemu-system-x86 curl python
        ;;

    fedora)
        $SUDO dnf install -y qemu-system-x86 curl python3
        ;;

    *)
        echo "Unsupported Linux distribution: $ID"
        exit 1
        ;;
esac

# Verify QEMU
if ! command -v qemu-system-i386 >/dev/null 2>&1; then
    echo "QEMU installation failed!"
    exit 1
fi

# Fetch latest release ISO URL
echo "[3/4] Fetching latest KernOS release..."

ISO_URL=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" |
python3 -c '
import json, sys

release = json.load(sys.stdin)

for asset in release.get("assets", []):
    if asset["name"].lower() == "kernos.iso":
        print(asset["browser_download_url"])
        break
')

if [ -z "$ISO_URL" ]; then
    echo "Could not find KernOS.iso in the latest release."
    exit 1
fi

# Download ISO
echo "Downloading KernOS..."
curl -fL "$ISO_URL" -o "$ISO"

echo "[4/4] Launching KernOS..."

qemu-system-i386 \
    -cdrom "$ISO" \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04

echo "KernOS has exited."
```
