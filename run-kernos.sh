#!/usr/bin/env bash

set -Ee

REPO="pahal-desai/kernel"
ISO="kernos.iso"

# =====================================
# Logging
# =====================================

log() {
    echo
    echo -e "\033[1;36m[INFO]\033[0m $(date '+%H:%M:%S') $*"
}

success() {
    echo -e "\033[1;32m[ OK ]\033[0m $*"
}

warning() {
    echo -e "\033[1;33m[WARN]\033[0m $*"
}

error() {
    echo -e "\033[1;31m[FAIL]\033[0m $*" >&2
}

run() {
    echo -e "\033[1;35m[EXEC]\033[0m $*"
    "$@"
}

# =====================================
# Error handling
# =====================================

trap 'status=$?; error "Command failed: $BASH_COMMAND"; error "Line: $LINENO | Exit code: $status"; exit "$status"' ERR

# =====================================
# Welcome
# =====================================

echo "========================================"
echo "          KernOS Auto Launcher"
echo "========================================"
echo "             Made by Pahal"
echo "========================================"

log "Starting KernOS installation and launch process."

# =====================================
# OS Detection
# =====================================

log "Checking Linux distribution..."

if [[ ! -f /etc/os-release ]]; then
    error "Cannot find /etc/os-release."
    exit 1
fi

source /etc/os-release

success "Operating system: $PRETTY_NAME"
log "Distribution ID: $ID"

# =====================================
# Privileges
# =====================================

if [[ $EUID -eq 0 ]]; then
    SUDO=""
    success "Running with root privileges."
else
    if ! command -v sudo >/dev/null 2>&1; then
        error "sudo is not installed."
        exit 1
    fi

    SUDO="sudo"
    log "Administrator privileges will be requested."
fi

# =====================================
# Dependency Installation
# =====================================

log "Checking and installing dependencies..."

case "$ID" in

    ubuntu|debian|linuxmint|pop)
        log "Package manager: APT"
        run $SUDO apt-get update
        run $SUDO apt-get install -y qemu-system-x86 curl python3
        ;;

    arch|endeavouros|manjaro)
        log "Package manager: Pacman"
        run $SUDO pacman -Sy --needed --noconfirm qemu-system-x86 curl python
        ;;

    fedora)
        log "Package manager: DNF"
        run $SUDO dnf install -y qemu-system-x86 curl python3
        ;;

    *)
        error "Unsupported distribution: $ID"
        exit 1
        ;;

esac

success "Dependency installation completed."

# =====================================
# Verify Dependencies
# =====================================

log "Verifying installed dependencies..."

for cmd in curl python3 qemu-system-i386; do
    if command -v "$cmd" >/dev/null 2>&1; then
        success "$cmd found: $(command -v "$cmd")"
    else
        error "$cmd is missing."
        exit 1
    fi
done

# =====================================
# GitHub Release Detection
# =====================================

log "Connecting to GitHub API..."

API_URL="https://api.github.com/repos/$REPO/releases/latest"

log "Fetching latest release information..."

RELEASE_JSON=$(curl -fsSL "$API_URL")

success "GitHub API response received."

log "Searching for kernos.iso..."

ISO_URL=$(printf '%s' "$RELEASE_JSON" | python3 -c '
import json, sys

release = json.load(sys.stdin)

print("Release:", release.get("tag_name", "unknown"), file=sys.stderr)

for asset in release.get("assets", []):
    print("Found asset:", asset["name"], file=sys.stderr)
    if asset["name"].lower() == "kernos.iso":
        print(asset["browser_download_url"])
        break
')

if [[ -z "$ISO_URL" ]]; then
    error "kernos.iso was not found in the latest release."
    exit 1
fi

success "ISO download URL found."

# =====================================
# Download ISO
# =====================================

log "Downloading KernOS ISO..."
log "Destination: $(pwd)/$ISO"

run curl -fL --progress-bar "$ISO_URL" -o "$ISO"

if [[ ! -s "$ISO" ]]; then
    error "Downloaded ISO is missing or empty."
    exit 1
fi

success "ISO downloaded successfully."
log "File size: $(du -h "$ISO" | cut -f1)"

# =====================================
# Launch KernOS
# =====================================

log "Preparing QEMU..."

QEMU_CMD=$(command -v qemu-system-i386)

success "QEMU executable: $QEMU_CMD"

log "Launching KernOS..."
echo
echo "========================================"
echo "              KernOS Boot"
echo "========================================"
echo

"$QEMU_CMD" \
    -cdrom "$ISO" \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04

success "KernOS has exited."
log "Thank you for trying KernOS!"
