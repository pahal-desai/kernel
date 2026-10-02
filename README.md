# KernOS
This is a tiny x86 hobby operating system written in C and Assembly.

## Features
* It has runs basic commands like neofetch, ver, clear, help, and an easter egg command.
* It has a mini game too (hangman).

## why i made this??!!
just for fun -_-
I wanted to learn assembly and more on how operating systems work. That's why i started this project.
And i was heavily inspired by the story of temple os.

## What features i want to add in future??
* have some basic terminal keyboard shortcuts (like ctrl+c to inturrupt, up arrow key to get previous commands, etc)
* create/delete/edit files
* run a video game (doom to be precise :D)

## How to test?

Download `kernos.iso` from the [Releases](https://github.com/pahal-desai/kernel/releases) page and run it using QEMU!

### 1. Install QEMU

#### Linux (Debian / Ubuntu)
```bash
sudo apt install qemu-system-x86
```

#### Windows
Install using [winget](https://github.com/microsoft/winget-cli):
```powershell
winget install QEMU
```

#### macOS
Install using [Homebrew](https://brew.sh/):
```bash
brew install qemu
```

### 2. Run the ISO

Open your terminal in the directory where you downloaded `kernos.iso`:

* **Linux / macOS:**
  ```bash
  qemu-system-i386 -cdrom kernos.iso -device isa-debug-exit,iobase=0xf4,iosize=0x04
  ```

* **Windows (PowerShell / Command Prompt):**
  ```powershell
  qemu-system-i386 -cdrom kernos.iso -device isa-debug-exit,iobase=0xf4,iosize=0x04
  ```

---

*(Optional) To build from source on Linux:*
```bash
sudo apt install build-essential nasm grub2-common grub-pc-bin xorriso mtools
make clean && make run
```


## Screenshots:-
* neofetch

![Neofetch](screenshots/neofetch.png)

* hangman

![Hangman](screenshots/hangman.png)

* other commands

![other commands](screenshots/main.png)

* shutdown

![shutdown](/screenshots/shutdown.png)