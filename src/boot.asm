;im learning so i'll be commenting to understand the code

bits 32

section .multiboot
align 4
dd 0x1BADB002              ; magic number (multiboot 1)
dd 0x00000000              ; flags
dd -(0x1BADB002 + 0x00000000) ; checksum

section .bss
align 16
stack_bottom:
resb 16384                 ; 16 kib stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top     ; set up C stack pointer

    call kernel_main       ; jump into C kernel

.halt:
    cli
    hlt
    jmp .halt
