; compile with nasm, use as disk image to qemu-system-i386

%define BANNER 'SP/OS (2013) Saul Pwanson'
%strlen BANNER_LEN BANNER

; (boot sector - elif header - filename)
BS_FILE_LEN equ 512 - 16 - BANNER_LEN 

LOWMEM_SECTOR equ 0x8000

[BITS 16]
[ORG 0x7c00]

entry:                           ; eliF header
                                 ; 4 bytes application specific
 	    jmp endhdr               ;     near jump (2 bytes)
lba     dw 1                     ;     starting sector LBA=1 (skip boot sector)

        db 'eliF'                ; 4 byte magic
        dd BS_FILE_LEN           ; 4 byte file length
        dw 0                     ; 2 bytes reserved
        db 1                     ; 1 byte status (STATUS_EXISTING)
        db BANNER_LEN            ; 1 byte name length

banner  db BANNER

retries db 10   ; max 10 retries until fail

endhdr:
    jmp 0x0000:start    ; set CS:IP to known values

start:
    cli                 ; disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00      ; just before the code

%if 0
; display banner
    push dx
    mov bx, 0x000f
    mov ax, 0x1301
    mov cx, BANNER_LEN
    mov dx, 0x1701
    mov bp, banner
    int 0x10
    pop dx 
%endif

    call enable_A20

    call resetdisk

    mov si, 0x7000      ; drive parameters buffer

%define BOOT_DRIVE       byte [si]
%define NUM_CYLINDERS    word [si+0x04] ; dword [0x7004]
%define NUM_HEADS        word [si+0x08] ; dword [0x7008]
%define SECTOR_PER_TRACK word [si+0x0c] ; dword [0x700C]
; %define TOTAL_SECTORS    word [si+0x10] ; qword [0x7010]
; %define PARA_PER_SECTOR  word [si+0x18] ;  word [0x7018] (in bytes at first)

    mov BOOT_DRIVE, dl   ; save off boot drive before 13/08 trashes it
    mov ah, 0x08
    int 0x13
    jnc parmsok

    mov al, '&'
    call putc
    hlt

parmsok:
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

    mov edi, 0x100000

    sub cx, [lba] ; # sectors to read = total sectors - 1 boot sector

nextsector:
    mov ax, [lba]
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
    mov bx, LOWMEM_SECTOR ; ES:BX = dest address for load
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
%ifdef DEBUG
    mov al, '.'
    call putc
%endif
    call copy_sector

    pop cx
    inc word [lba]
    loop nextsector

%ifdef DEBUG
    mov al, '>'
    call putc
%endif

leap:
    lgdt [GDT]                      ; ge
    mov eax, cr0                    ; ro
    or al, 1                        ; ni
    mov cr0, eax                    ; mo
    jmp 0x08:protmain               ; !!

[bits 16]
copy_sector:
    push ds
    push es
    push si

    lgdt [GDT]                      ; ge
    mov eax, cr0                    ; ro
    or al, 1                        ; ni
    mov cr0, eax                    ; mo
    jmp 0x08:pmode_copy_sector

[bits 32]
pmode_copy_sector:
    mov ax, 0x10
    mov es, ax
    mov ds, ax

    mov esi, LOWMEM_SECTOR
;    mov edi,  ; edi should already be fine
    mov ecx, 512/4
    rep movsd

    mov eax, cr0
    and al, 0xfe
    mov cr0, eax
    jmp 0000:end_copy_loop

[bits 16]
end_copy_loop:
    pop si
    pop es
    pop ds
    ret

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

; --- protected mode ---
[bits 32]

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

    mov eax, 0x100010      ; after 16-byte FILE header
    add al, [0x10000f]     ; + filename size

    call eax              ; kernel starts immediately

_halt:
    hlt
    jmp _halt

    times (512 - $ + entry - 2) db 0 ; pad boot sector with zeroes

       db 0x55, 0xAA ; 2 byte boot signature

