; compile with nasm, use as disk image to qemu-system-i386

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
resetdisk:
    xor ax, ax
    int 0x13            ; 13h/00 = reset drive
    jc resetdisk
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

parmserr:
    mov al, '&'
    call putc
    hlt

retries     db 10   ; max 10 retries until fail
current_lba dw 1    ; starting sector (skip boot sector)

start:
    cli                 ; disable interrupts
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7c00      ; just before the code

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

    call resetdisk

    mov si, 0x7000      ; drive parameters buffer

%define BOOT_DRIVE       byte [si]
%define NUM_CYLINDERS    word [si+0x04] ; dword [0x7004]
%define NUM_HEADS        word [si+0x08] ; dword [0x7008]
%define SECTOR_PER_TRACK word [si+0x0c] ; dword [0x700C]
%define TOTAL_SECTORS    word [si+0x10] ; qword [0x7010]
%define PARA_PER_SECTOR  word [si+0x18] ;  word [0x7018] (in bytes at first)

    mov BOOT_DRIVE, dl   ; save off boot drive before 13/08 trashes it
    mov ah, 0x08
    int 0x13
    jc parmserr

    mov word PARA_PER_SECTOR, 0x20 ; 512 bytes/sector is 32 paragraphs

; set up parameters for LBAtoCHS

;;; unnecessary
;    push cx
;    shr cx, 6
;    mov NUM_CYLINDERS, cx   ; max value of cylinder
;    pop cx

    and cx, 0x3f        ; mask off lower two bits of cylinder
    mov SECTOR_PER_TRACK, cx ; max value of sector

    shr dx, 8           ; dx = dh (# heads)
    inc dx
    mov NUM_HEADS, dx

    mov cx, 1165        ; read 600k at most (up to 2k below 640k)

    mov ax, 0x0800
    mov es, ax

    sub cx, [current_lba] ; # sectors to read = total sectors - 1 boot sector

nextsector:
    mov ax, [current_lba]
    push cx

    xor dx, dx               ; DX:AX = LBA
    mov bx, SECTOR_PER_TRACK ; BX = SectorsPerTrack
    mov cx, NUM_HEADS        ; CX = NumHeads

; in: DX:AX=LBA Sector, BX=SectorsPerTrack, CX = NumHeads
; out: DH, CX compatible with int 13h/02
LBAtoCHS:
    div bx           ; ax = LBA/SectorsPerTrack = track# * head#

    inc dx           ; dx = remainder+1 = sector#
    push dx
    xor dx, dx       ; dx:ax = track# * head#
    div cx           ; ax = track#, dx = head#

    mov dh, dl       ; dh = head#
    pop cx           ; cl[5:0] = sector#

    mov ch, al       ; ch = low 8 bits of track#
    shl ah, 6
    or cl, ah        ; cl[7:6] = high bits of track#
     
    mov dl, BOOT_DRIVE
    xor bx, bx          ; ES:BX = dest address for load
    mov ax, 0x0201      ; function 13h/02, read 01 sectors
    int 0x13
    jnc success
    mov al, '!'
    call putc
    ; DEBUG: print ah for error code
    call resetdisk
    pop cx

    dec byte [retries]
    jnz nextsector

    mov al, '<'
    call putc

    ; might be an odd-shaped disk, let's jump anyway
    jmp leap

success:
%if DEBUG
    mov al, '.'
    call putc
%endif

    mov ax, es
    add ax, PARA_PER_SECTOR  ; shift the destination segment by bytes/sector
    mov es, ax

    pop cx
    inc word [current_lba]
    loop nextsector

%if DEBUG
    mov al, '>'
    call putc
%endif

leap:
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
    add al, [0x800f]     ; + filename size
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

exception_halt: ; instead of iret, but restore state for easier debugging
%if DEBUG
    pop eax     ; remove ret address from call [isr2]
    pop eax     ; remove exception number parameter
    popad
    pop eax     ; original eax
    xchg esp, [isrESP]
%endif
_halt:
    hlt
    jmp _halt


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

isr2 dd exception_halt       ; ISRstage2(intnum): [0x7DF8] = stage func ptr
     dw 0x00                 ; 'reserved'

     db 0x55, 0xAA ; 2 byte boot signature

