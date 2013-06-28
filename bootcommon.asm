[BITS 16]

leap:
    call enable_A20

    lgdt [GDT]                      ; ge
    mov eax, cr0                    ; ro
    or al, 1                        ; ni
    mov cr0, eax                    ; mo
    jmp 0x08:protmain               ; !!

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

putc:
    push ax
    mov ah, 0x0e
    int 0x10
    pop ax
    ret

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

; --- protected mode ---
[BITS 32]

GDT    dw 0x28                      ; limit of 5 entries
       dd GDT                       ; linear address of GDT
       dw 0
        ; 0xBBBBLLLL, 0xBBFLAABB    ; F = GS00b, AA = 1001XDW0
gdtCS  dd 0x0000ffff, 0x00CF9A00    ; 0x08
gdtDS  dd 0x0000ffff, 0x00CF9200    ; 0x10
gdtGS  dd 0x70000bff, 0x00CF9200    ; 0x18, TLS at 0x07000-0x07bff
gdtTSS dd 0x7e000067, 0x00008900    ; 0x20, TSS at 0x07E00-0x07E67

protmain:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax

    mov ax, 0x18          ; for the TLS segment (-mno-tls-direct-seg-refs)
    mov gs, ax
;    mov fs, ax           ; would be needed instead for 64-bit TLS

    mov ax, 0x20
    ltr ax               ; TSS descriptor
    xor eax, eax
    lldt ax              ; no LDT

    mov esp, 0x6000      ; data stack grows down

; set up page tables
    mov eax, 0x3000
    mov cr3, eax

    mov edi, eax      ; bzero page dir and PT0
    mov ecx, 0x2000/4
    xor eax, eax
    rep stosd

    mov dword [0x3000], 0x4003  ; PT0
    mov dword [0x3ffc], 0x3003  ; entire pagetable itself

    mov edi, 0x4004      ; skip first page (null ptr)
    mov ecx, 0x1ff       ; 0-2MB only (0x400/4 - 1)
    mov eax, 0x1003      ; identity map; 3 = RW | PRESENT
nextpage:
    stosd
    add eax, 0x1000
    loop nextpage

    ; turn on paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov eax, 0x8200
    call eax              ; kernel starts immediately

_halt:
    hlt
    jmp _halt
