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
    echo "[ $(date '+%H:%M:%S') ] $1"
}

success() {
    echo "[✓] $1"
}

error() {
    echo
    echo "[✗] $1" >&2
    exit 1
}

trap 'status=$?; echo; echo "[✗] Command failed on line $LINENO (exit code $status)." >&2; exit "$status"' ERR

# ==============================
# Welcome
# ==============================

clear 2>/dev/null || true

echo "========================================"
echo "          KernOS Auto Launcher"
echo "========================================"
echo "          Made by Pahal Desai"
echo "========================================"

# ==============================
# Detect Linux distribution
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
    log "Administrator privileges will be requested when needed."
fi

# ==============================
# Install dependencies
# ==============================

log "Installing required dependencies..."

case "$ID" in

    ubuntu|debian|linuxmint|pop)
        log "Using APT..."
        $SUDO apt-get update
        $SUDO apt-get install -y qemu-system-x86 curl python3
        ;;

    arch|endeavouros|manjaro)
        log "Using Pacman..."
        $SUDO pacman -Syu --noconfirm --needed qemu-system-i386 curl python
        ;;

    fedora)
        log "Using DNF..."
        $SUDO dnf install -y qemu-system-x86 curl python3
        ;;

    *)
        error "Unsupported Linux distribution: $ID"
        ;;

esac

success "Dependencies installed."

# ==============================
# Verify QEMU
# ==============================

log "Checking QEMU installation..."

command -v qemu-system-i386 >/dev/null 2>&1 ||
    error "QEMU installation failed."

success "QEMU is ready."

# ==============================
# Fetch latest release
# ==============================

log "Connecting to GitHub..."

RELEASE_URL="https://api.github.com/repos/$REPO/releases/latest"

ISO_URL=$(curl -fsSL "$RELEASE_URL" | python3 -c '
import json
import sys

release = json.load(sys.stdin)

for asset in release.get("assets", []):
    if asset["name"].lower() == "kernos.iso":
        print(asset["browser_download_url"])
        break
')

[ -n "$ISO_URL" ] || error "Could not find kernos.iso in the latest release."

success "Latest KernOS release found."

# ==============================
# Download ISO
# ==============================

log "Downloading KernOS ISO..."

curl -fL --progress-bar "$ISO_URL" -o "$ISO"

[ -s "$ISO" ] || error "Downloaded ISO is empty."

success "KernOS downloaded successfully."

# ==============================
# Launch KernOS
# ==============================

log "Launching QEMU..."

echo
echo "========================================"
echo "              KernOS Boot"
echo "========================================"
echo

qemu-system-i386 \
    -cdrom "$ISO" \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04

echo
success "KernOS has exited."
echo "Thanks for trying KernOS!"
```
