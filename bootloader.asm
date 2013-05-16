; compile with nasm, use as floppy image to Virtualbox

%define DEBUG 0

[BITS 16]
[ORG 0x7c00]

entry:
	jmp 0x0000:start    ; set CS:IP to known values

start:
    cli                 ; disable interrupts
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7b00

    call reset

    mov ax, 0x0800
    mov es, ax

    mov ax, 1           ; starting sector (skip boot sector)
    mov cx, 1211        ; # sectors in 0x08000-0xA0000 (640k - 32k)
    mov dx, 10          ; max 10 errors

nextsector:
    push ax
    push cx

    call LBAtoCHS       ; DX:CX = drive/head/track/sector
    xor bx, bx          ; ES:BX = dest address for load
    mov ax, 0x0201      ; function 13h/02, read 01 sectors
    int 0x13
    jnc success
    mov al, '!'
    call putc
    ; DEBUG: print ah for error code
    call reset
    pop cx
    pop ax
    dec dx
    jnz nextsector

    mov al, ':'
    call putc
    mov al, '('
    call putc
    hlt

success:
%if DEBUG
    mov al, '.'
    call putc
%endif

    mov ax, es
    add ax, 0x20         ; shift the destination segment by 512bytes
    mov es, ax

    pop cx
    pop ax
    inc ax
    loop nextsector

%if DEBUG
    mov al, '>'
    call putc
%endif

    lgdt [cs:GDT]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:protmain

;[in AX=LBA Sector]
;[out DX,CX]
LBAtoCHS:
    xor     dx,dx
    mov     cx, 18 ; word [BOOT_SECTOR.sectorspertrack]
    div     cx

    inc     dx                    
    mov     cl, dl   ;sectors
    xor     dx,dx
    mov     dh, al
    and     dh, 0x1  ; head
    shr     ax, 1    ; div [num_heads]
    mov     ch, al   ; track
    xor     dl, dl   ; drive
    ret

putc:
    push ax
    mov ah, 0x0e
    int 0x10
    pop ax
    ret

reset:
    mov dl, 0
    xor ax, ax
    int 0x13            ; 13h/00 = reset drive
    jc reset
    ret

; --- protected mode ---
[bits 32]

NUM_INTS equ 64
IDT_ADDR equ 0x800
HANDLER_ADDR equ IDT_ADDR + (NUM_INTS * 8)

IDTR   dw NUM_INTS*8-1              ; limit
idtptr dd IDT_ADDR                  ; linear address of IDT

GDT    dw 0x20                      ; limit of 4 entries
       dd GDT                       ; linear address of GDT
       dw 0
        ; 0xBBBBLLLL, 0xBBFLAABB    ; F = GS00b, AA = 1001XDW0
gdtCS  dd 0x0000ffff, 0x00CF9A00    ; 0x08h
gdtDS  dd 0x0000ffff, 0x00CF9200    ; 0x10h
gdtTSS dd 0x10000067, 0x00008800    ; 0x18h

isrESP dd 0x00001000

; <= 16 bytes for the stub
stage0isr:
    xchg esp, [isrESP]
    push 0
    jmp [isr1] ; weird indirect jmp because i can't figure out how to force
               ;   nasm to generate an absolute jmp to a label

stage1isr:
    xchg eax, [esp]

    pushad

%if DEBUG
    ; DEBUG: display interrupt# in upper right
    push eax
    mov edi, 0xb8000 + 160 + 156
    call hex8
    pop eax
%endif

    push eax              ; interrupt # is argument
    call [isr2]
    pop eax

    popad

    pop eax
    xchg esp, [isrESP]
    iret

%macro outbyte 2
    mov al, %2
    out %1, al
%endmacro

_halt:
    hlt

%if DEBUG
hex8:
    mov bl, al
    shr al, 4
    call hex4
    mov al, bl
;    call hex4
;    ret

hex4:
    and al, 0x0f
    add al, '0'
    cmp al, '9'
    jle printit
    add al, 'A' - '0' - 10
printit:
    mov ah, 0x0F
    stosw
    ret
%endif

protmain:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

;    mov ax, 0x18
;    ltr ax               ; TSS descriptor
;    xor eax, eax
;    lldt ax              ; no LDT

    mov esp, 0x6000      ; data stack grows down

; create IDT entries, incrementing the stub address
    mov edi, [idtptr]   ; IDT from 0x800 - 0x9FF
    mov ecx, NUM_INTS   ; interrupts available (0-0x3f)
    mov eax, 0x00080000 + HANDLER_ADDR ; CS=0x08; IP[15:0] = 0x0200 (+ 16*interrupt#)
    mov edx, 0x00008E00 ; IP[31:16] = 0; type=0x8E (32-bit int gate); reserved=0
nextidtentry:
    stosd
    xchg eax, edx
    stosd
    xchg eax, edx
    add eax, 0x0010     ; exception stub <=16 bytes
    loop nextidtentry

; copy stage0isr stub, increasing the K in its 'push K'
;    mov edi, HANDLER_ADDR      ; interrupt handlers are at 0xA00 - 0xDFF
    mov ecx, NUM_INTS
    mov al, 0
nextinthandler:
    mov esi, stage0isr
    movsd
    movsd
    movsd
    movsd
    inc al
    mov [esi-16 + 7], al
    loop nextinthandler

    lidt [IDTR]

    call enable_A20

    jmp 0x8020           ; kernel starts after 32-byte header

enable_A20: ; from wiki.osdev.org
    cli

    call    a20wait
    mov     al,0xAD
    out     0x64,al

    call    a20wait
    mov     al,0xD0
    out     0x64,al

    call    a20wait2
    in      al,0x60
    push    eax

    call    a20wait
    mov     al,0xD1
    out     0x64,al

    call    a20wait
    pop     eax
    or      al,2
    out     0x60,al

    call    a20wait
    mov     al,0xAE
    out     0x64,al

    call    a20wait
    sti
    ret

a20wait:
        in      al,0x64
        test    al,2
        jnz     a20wait
        ret

a20wait2:
        in      al,0x64
        test    al,1
        jz      a20wait2
        ret

%if DEBUG
    mov edi, 0xb8000

    mov esi, str_welcome
    call dispstring

    call 0x8000

    mov word [0xb8000], '. '
    hlt
str_welcome db 'welcome!', 0

; move string at esi, to vidmem at edi
dispstring:
    push eax
    mov ah, 0x07

nextchar:
    lodsb
    stosw
    or al, al
    jnz nextchar

    pop eax
    ret
%endif

    times (512 - $ + entry - 12) db 0 ; pad boot sector with zeroes

isr1 dd stage1isr            ; ISR stage 1 -- keep asm
isr2 dd _halt                ; ISRstage2(intnum): [0x7DF8] = stage func ptr
     dw 0x00                 ; 'reserved'

     db 0x55, 0xAA ; 2 byte boot signature

