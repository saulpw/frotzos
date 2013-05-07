dumpbytes:
    mov cx, 128          ; 
    mov ax, 0x0800

dispsector:
    mov ds, ax
    mov si, 0
    lodsb
    call putc
    mov ax, ds
    add ax, 0x100        ; every page (8 sectors)
    loop dispsector
    hlt

hex4:
    push ax
    and al, 0x0f
    add al, '0'
    cmp al, '9'
    jle printit
    add al, 'A' - '0' - 10
printit:
    call putc
    pop ax
    ret

hex8:
    push ax
    shr al, 4
    call hex4
    pop ax
    call hex4
    ret

hex16:
    push ax
    mov al, ah
    call hex8
    pop ax
    call hex8
    ret
