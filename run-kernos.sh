```bash
#!/usr/bin/env bash

set -Ee

REPO="pahal-desai/kernel"
ISO="kernos.iso"

# ==============================
# Logging
# ==============================

log() {
    echo
    echo "[$(date '+%H:%M:%S')] $1"
}

success() {
    echo "[✓] $1"
}

error() {
    echo "[✗] $1" >&2
    exit 1
}

trap 'status=$?; echo "[✗] Failed at line $LINENO (exit code $status)." >&2; exit "$status"' ERR

# ==============================
# Welcome
# ==============================

echo "========================================"
echo "          KernOS Auto Launcher"
echo "========================================"
echo "             Made by Pahal"
echo "========================================"

# ==============================
# Detect OS
# ==============================

log "Detecting Linux distribution..."

[ -f /etc/os-release ] || error "Cannot detect Linux distribution."

source /etc/os-release

success "Detected: $PRETTY_NAME"

# ==============================
# Privileges
# ==============================

if [ "$EUID" -eq 0 ]; then
    SUDO=""
    success "Running as root."
else
    command -v sudo >/dev/null 2>&1 || error "sudo is required."
    SUDO="sudo"
fi

# ==============================
# Install dependencies
# ==============================

log "Installing dependencies..."

case "$ID" in

    ubuntu|debian|linuxmint|pop)
        log "Using APT..."
        $SUDO apt-get update
        $SUDO apt-get install -y qemu-system-x86 curl python3
        ;;

    arch|endeavouros|manjaro)
        log "Using Pacman..."
        $SUDO pacman -Sy --needed --noconfirm qemu-system-x86 curl python
        ;;

    fedora)
        log "Using DNF..."
        $SUDO dnf install -y qemu-system-x86 curl python3
        ;;

    *)
        error "Unsupported distribution: $ID"
        ;;

esac

success "Dependencies installed."

# ==============================
# Verify QEMU
# ==============================

log "Verifying QEMU..."

command -v qemu-system-i386 >/dev/null 2>&1 ||
    error "QEMU executable qemu-system-i386 not found."

success "QEMU is ready."

# ==============================
# Fetch latest release
# ==============================

log "Fetching latest KernOS release..."

ISO_URL=$(curl -fsSL \
    "https://api.github.com/repos/$REPO/releases/latest" |
    python3 -c '
import json, sys

release = json.load(sys.stdin)

for asset in release.get("assets", []):
    if asset["name"].lower() == "kernos.iso":
        print(asset["browser_download_url"])
        break
')

[ -n "$ISO_URL" ] || error "KernOS ISO not found in GitHub releases."

success "Release found."

# ==============================
# Download ISO
# ==============================

log "Downloading KernOS..."

curl -fL --progress-bar "$ISO_URL" -o "$ISO"

[ -s "$ISO" ] || error "Downloaded ISO is empty."

success "Download complete."

# ==============================
# Launch
# ==============================

log "Launching KernOS..."

echo
echo "========================================"
echo "              KernOS Boot"
echo "========================================"
echo

qemu-system-i386 \
    -cdrom "$ISO" \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04

success "KernOS has exited."
echo "Thanks for trying KernOS!"
```
