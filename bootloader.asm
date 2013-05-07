; compile with nasm, use as floppy image to Virtualbox

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
    mov cx, 1215        ; # sectors in 0x08000-0xA0000 (640k - 32k)

nextsector:
    push ax
    push cx

    call LBAtoCHS       ; DX:CX = drive/head/track/sector
    xor bx, bx          ; ES:BX = dest address for load
    mov ax, 0x0201      ; function 13h/02, read 01 sectors
    int 0x13
    jnc success
    call error
    pop cx
    pop ax
    jmp nextsector      ; TODO: only try 3 times before halting

success:
;    mov al, '.'
;    call putc

    mov ax, es
    add ax, 0x20
    mov es, ax

    pop cx
    pop ax
    inc ax
    loop nextsector

    mov al, 'O'
    call putc

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

error:
    mov al, '!'
    call putc
;    mov al, ah
;    call hex8

; fall-through
reset:
    mov dl, 0           ; TODO: don't destroy dl?
    xor ax, ax
    int 0x13            ; 13h/00 = reset drive
    jc error
    ret

GDT   dw 0xffff         ; limit
      dd GDT            ; offset
      dw 0
       ; 0xBBBBLLLL, 0xBBFLAABB    ; F = GS00b, AA = 1001XDW0
gdtCS dd 0x0000ffff, 0x00CF9A00    ; 0x08h
gdtDS dd 0x0000ffff, 0x00CF9200    ; 0x10h

protmain:
[bits 32]
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x6000      ; data stack grows down

    jmp 0x8000

    times (512 - $ + entry - 2) db 0 ; pad boot sector with zeroes
    db 0x55, 0xAA ; 2 byte boot signature

    mov edi, 0xb8002
    mov dword [edi], 'K > '
    hlt
