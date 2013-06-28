[BITS 16]
[ORG 0x7c00]

boot_drive equ $        ; kernel can get the boot_drive from 0x7c00

entry:
    cli
    jmp 0x0000:start

%if 1
    times (8 - $ + entry) db 0   ; pad until boot-info-table

iso_boot_info:
bi_pvd  dd 16           ; LBA of primary volume descriptor
bi_file dd 0            ; LBA of boot file
bi_len  dd 0            ; len of boot file
bi_csum dd 0
bi_reserved times 10 dd 0
%endif

banner db 10, "SP/OS (2013) Saul Pwanson", 13, 10, 0
errstr db "error loading kernel", 0

; Disk Address Packet
dap db 0x10, 0          ; sizeof(dap)
    dw 16               ; transfer 16 sectors (before PVB)
    dw 0x8000, 0x0      ; to 0:8000
    dd 0, 0             ; from LBA 0

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
    call enable_A20

    hlt
    jmp 0x8200

writestr:
    lodsb
    test al, al
    jz end
    mov ah, 0x0e
    mov bx, 0x000f
    int 0x10
    jmp writestr
end:
    ret

enable_A20: ; from wiki.osdev.org
    call a20wait
    mov al,0xAD
    out 0x64,al

    call a20wait
    mov al,0xD0
    out 0x64,al

    call a20wait2
    in al,0x60
    push eax

    call a20wait
    mov al,0xD1
    out 0x64,al

    call a20wait
    pop eax
    or al,2
    out 0x60,al

    call a20wait
    mov al,0xAE
    out 0x64,al

    call a20wait
    ret

a20wait:
    in al,0x64
    test al,2
    jnz a20wait
    ret

a20wait2:
    in al,0x64
    test al,1
    jz a20wait2
    ret

    times (512 - $ + entry - 2) db 0 ; pad boot sector with zeroes
    
    db 0x55, 0xAA ; 2 byte boot signature
