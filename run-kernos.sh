```bash
#!/usr/bin/env bash

set -Ee

REPO="pahal-desai/kernel"
ISO="kernos.iso"

# Logging
log() {
    echo -e "\n[ $(date '+%H:%M:%S') ] $1"
}

success() {
    echo "[✓] $1"
}

error() {
    echo "[✗] $1" >&2
    exit 1
}

trap 'error "Something went wrong on line $LINENO. Check the output above."' ERR

echo "========================================"
echo "          KernOS Auto Launcher"
echo "========================================"

# Detect OS
log "Detecting Linux distribution..."

[ -f /etc/os-release ] || error "Cannot detect Linux distribution."

source /etc/os-release

success "Detected: $PRETTY_NAME"

# Sudo setup
if [ "$EUID" -eq 0 ]; then
    SUDO=""
else
    command -v sudo >/dev/null 2>&1 || error "sudo is required."
    SUDO="sudo"
fi

# Install dependencies
log "Installing required dependencies..."

case "$ID" in
    ubuntu|debian|linuxmint|pop)
        log "Using APT package manager..."
        $SUDO apt update
        $SUDO apt install -y qemu-system-x86 curl python3
        ;;

    arch|endeavouros|manjaro)
        log "Using Pacman package manager..."
        $SUDO pacman -Sy --needed qemu-system-x86 curl python
        ;;

    fedora)
        log "Using DNF package manager..."
        $SUDO dnf install -y qemu-system-x86 curl python3
        ;;

    *)
        error "Unsupported distribution: $ID"
        ;;
esac

success "Dependencies installed."

# Verify QEMU
log "Checking QEMU installation..."

command -v qemu-system-i386 >/dev/null 2>&1 ||
    error "QEMU installation failed."

success "QEMU is ready."

# Fetch release
log "Fetching the latest KernOS release from GitHub..."

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

[ -n "$ISO_URL" ] || error "Could not find kernos.iso in the latest release."

success "Found KernOS ISO."

# Download
log "Downloading KernOS..."

curl -fL --progress-bar "$ISO_URL" -o "$ISO"

success "KernOS downloaded successfully."

# Launch
log "Starting KernOS in QEMU..."

echo "========================================"
echo "             Booting KernOS"
echo "========================================"

qemu-system-i386 \
    -cdrom "$ISO" \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04

log "KernOS has exited."
success "Thanks for trying KernOS!"
```
