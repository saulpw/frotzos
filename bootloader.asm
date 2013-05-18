; compile with nasm, use as floppy image to Virtualbox

%define DEBUG 0

[BITS 16]
[ORG 0x7c00]

entry:
	jmp 0x0000:start    ; set CS:IP to known values

putc:
    push ax
    mov ah, 0x0e
    int 0x10
    pop ax
    ret

; dl = boot drive
reset:
    xor ax, ax
    int 0x13            ; 13h/00 = reset drive
    jc reset
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

start:
    cli                 ; disable interrupts
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7b00

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

; A20 enabled

    ; dl still boot drive
    call reset

    mov ax, 0x0800
    mov es, ax

    mov si, 0x7000      ; drive parameters buffer
    mov word [si], 0x1e ; (maximum size expected)
    mov ah, 0x48        ; get drive parameters
    int 0x13
    jnc paramsok

    mov al, '&'
    call putc
    hlt
   
paramsok:
    mov [si], dl         ; save boot drive

; dword [0x7004]  ; cylinders
; dword [0x7008]  ; heads
; dword [0x700C]  ; sectors/track
; qword [0x7010]  ; total sectors on drive
;  word [0x7018]  ; bytes/sector

    shr word [si+0x18], 4 ; into paragraphs/sector for segment math

    mov cx, [si+0x10]   ; total # of sectors
    mov ax, 1           ; starting sector (skip boot sector)
    sub cx, ax          ; # sectors to read = total sectors - 1 boot sector
    mov dx, 10          ; max 10 errors

nextsector:
    push ax
    push cx
    push dx

    xor dx, dx            ; DX:AX
    mov bx, [si+0x0c]     ; BX = SectorsPerTrack
    mov cx, [si+0x08]     ; CX = NumHeads

; in: DX:AX=LBA Sector, BX=SectorsPerTrack, CX = NumHeads
; out: DH, CX compatible with int 13h/02
LBAtoCHS:
    div bx           ; ax = LBA/SectorsPerTrack = track# * head#

    inc dx           ; dx = remainder+1 = sector#
    push dx
    xor dx, dx       ; dx:ax = track# * head#
    div cx           ; ax = track#, dx = head#

    mov dh, dl       ; dh = head#
    pop cx           ; cx[5:0]  = sector#
    shr ax, 5
    or cx, ax        ; cx[15:6] = track#

    mov dl, [si]
    xor bx, bx          ; ES:BX = dest address for load
    mov ax, 0x0201      ; function 13h/02, read 01 sectors
    int 0x13
    jnc success
    mov al, '!'
    call putc
    ; DEBUG: print ah for error code
    call reset
    pop dx
    pop cx
    pop ax
    dec dx
    jnz nextsector

    mov al, '<'
    call putc
_halt:
    hlt
    jmp _halt

success:
%if DEBUG
    mov al, '.'
    call putc
%endif

    mov ax, es
    add ax, [si+0x18]  ; shift the destination segment by bytes/sector
    mov es, ax

    pop dx
    pop cx
    pop ax
    inc ax
    loop nextsector

%if DEBUG
    mov al, '>'
    call putc
%endif

    lgdt [GDT]                      ; ge
    mov eax, cr0                    ; ro
    or al, 1                        ; ni
    mov cr0, eax                    ; mo
    jmp 0x08:protmain               ; !!

; --- protected mode ---
[bits 32]

NUM_INTS     equ 64
IDT_ADDR     equ 0x800
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

    mov eax, 0x8010      ; after 16-byte FILE header
    add al, [0x8015]     ; + filename size
    jmp eax              ; kernel starts immediately

isrESP dd 0x00002000

; <= 16 bytes for the stub
stage0isr:
    xchg esp, [isrESP]
    push 0
    jmp [isr1] ; weird indirect jmp because i can't figure out how to force
               ;   nasm to generate an absolute jmp to a label

isr1    dd stage1isr            ; ISR stage 1 -- keep as assembly

stage1isr:
    xchg eax, [esp]

    pushad

%if DEBUG
    ; DEBUG: display interrupt#
    push eax
    mov edi, 0xb8000 + 160 + 156
    shl eax, 2
    add edi, eax
    shr eax, 2
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

    times (512 - $ + entry - 8) db 0 ; pad boot sector with zeroes

isr2 dd _halt                ; ISRstage2(intnum): [0x7DF8] = stage func ptr
     dw 0x00                 ; 'reserved'

     db 0x55, 0xAA ; 2 byte boot signature

