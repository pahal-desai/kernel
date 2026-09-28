org 0x7C00

mov si, message
mov ah, 0x0E

print:
    lodsb
    cmp al, 0
    je done
    int 0x10
    jmp print

done:
    cli
    hlt

message db "hello world", 0

times 510-($-$$) db 0
dw 0xAA55