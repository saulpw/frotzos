[BITS 16]
[ORG 0x7c00]

boot_drive equ $        ; kernel can get the boot_drive from 0x7c00

entry:
    cli
    jmp 0x0000:start

    times (8 - $ + entry) db 0   ; pad until boot-info-table

; don't bother with making el torito load the kernel image
iso_boot_info:
bi_pvd  dd 16           ; LBA of primary volume descriptor
bi_file dd 0            ; LBA of boot file
bi_len  dd 0            ; len of boot file
bi_csum dd 0
bi_reserved times 10 dd 0

banner db 10, "SP/OS (2013) Saul Pwanson", 13, 10, 0
errstr db "error loading kernel", 0

; Disk Address Packet
dap db 16, 0            ; [2] sizeof(dap)
    dw 16               ; [2] transfer 16 sectors (before PVB)
    dw 0x8000, 0x0      ; [4] to 0:8000
    dd 0, 0             ; [8] from LBA 0

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00      ; setup stack just before the code

    mov [boot_drive], dl

    ; display banner
    mov si, banner
    call writestr

    ; read sectors from disk
    mov si, dap
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jnc readok

readerr:
    mov si, errstr
    call writestr
    hlt

readok:

%include "bootcommon.asm"    

    times (512 - $ + entry - 2) db 0 ; pad boot sector with zeroes
    
    db 0x55, 0xAA ; 2 byte boot signature
