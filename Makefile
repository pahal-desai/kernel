ASM = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra

SRC_DIR = src
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/isodir
ISO = $(BUILD_DIR)/pahalos.iso

.PHONY: all run clean

all: $(ISO)

$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f elf32 $< -o $@

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o linker.ld
	$(LD) -m elf_i386 --no-warn-rwx-segments -T linker.ld -o $@ $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o

$(ISO): $(BUILD_DIR)/kernel.bin grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/kernel.bin
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISO_DIR)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -rf $(BUILD_DIR)
