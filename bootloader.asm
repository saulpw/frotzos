; compile with nasm, use as disk image to qemu-system-i386

[BITS 16]
[ORG 0x7c00]

BOOT_DRIVE equ $

entry:
    cli                     ; disable interrupts
    jmp 0x0000:start        ; set CS:IP to known values

lba     dw 0                ;     starting sector LBA=0 (incl. boot sector)
retries db 10               ; max 10 retries until fail

banner db 10, "SP/OS (2013) Saul Pwanson", 13, 10, 0

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00          ; just before the code

    mov [BOOT_DRIVE], dl    ; save off boot drive

    mov di, banner
    call writestr


    mov si, 0x7000          ; drive parameters buffer

%define NUM_CYLINDERS    word [si+0x04] ; dword [0x7004]
%define NUM_HEADS        word [si+0x08] ; dword [0x7008]
%define SECTOR_PER_TRACK word [si+0x0c] ; dword [0x700C]
; %define TOTAL_SECTORS    word [si+0x10] ; qword [0x7010]
; %define PARA_PER_SECTOR  word [si+0x18] ;  word [0x7018] (in bytes at first)
    
    mov dl, [BOOT_DRIVE]
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

    mov cx, 64          ; read inital 32k

    mov di, 0x8000

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
     
    mov dl, [BOOT_DRIVE]
    mov bx, di       ; ES:BX = dest address for load
    mov ax, 0x0201   ; function 13h/02, read 01 sectors
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

; dl = boot drive
resetdisk:
    xor ax, ax
    int 0x13            ; 13h/00 = reset drive
    jc resetdisk
    ret

success:
%ifdef DEBUG
    mov al, '.'
    call putc
%endif
    add di, 512

    pop cx
    inc word [lba]
    loop nextsector

%ifdef DEBUG
    mov al, '>'
    call putc
%endif

%include "bootcommon.asm"

    times (512 - $ + entry - 2) db 0 ; pad boot sector with zeroes

       db 0x55, 0xAA ; 2 byte boot signature

