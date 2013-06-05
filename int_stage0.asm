[BITS 32]

extern exc_handlers, irq_handlers, syscall_handler

global irq_stage0_start, irq_stage0_fixup, irq_stage0_end
global exc_stage0_start, exc_stage0_fixup, exc_stage0_end
global excerr_stage0_start, excerr_stage0_fixup, excerr_stage0_end
global syscall_stage0_start, syscall_stage0_fixup, syscall_stage0_end

excerr_stage0_start:
    xchg eax, [esp]            ; eax = error code
    pushad                     ; save all registers
    push eax                   ; arg2 = errcode
excerr_stage0_fixup:    
    mov eax, 0
    push eax                   ; arg1 = exception #
    push exception_end         ; unified return address
    jmp [exc_handlers + eax*4]  ; exception_handlers[excnum](exc#, errcode, ...)
excerr_stage0_end equ $

exc_stage0_start:
    push eax                   ; for consistency with above
    pushad                     ; save all registers
    push eax                   ; arg2 = fake errcode for consistency
exc_stage0_fixup:              ; "mov eax" is only 1 byte, literal follows
    mov eax, 0
    push eax                   ; arg1 = exception #
    push exception_end         ; unified return address
    jmp [exc_handlers + eax*4]  ; exception_handlers[excnum](exc#, errcode, ...)
exc_stage0_end equ $

global exception_end
exception_end:
    pop eax
    pop eax
    popad
    pop eax
    iret

irq_stage0_start:
    pushad                     ; save all registers
irq_stage0_fixup:
    mov eax, 0
    push eax                   ; arg1 = irq #
    push irq_end
    jmp [irq_handlers + eax*4] ; irq_handlers[irq#](irq#, ...)
irq_stage0_end equ $

global irq_end
irq_end:
    pop eax
    cmp al, 0x28
    jb master_irq_end

    mov al, 0x20
    out 0xA0, al               ; EOI to slave PIC

master_irq_end:
    mov al, 0x20
    out 0x20, al               ; EOI to master PIC
    popad
    iret

global syscall_stage0_start
syscall_stage0_start:
    mov eax, esp
    add eax, 12                ; don't make eip, cs, eflags args
    ; XXX: need to save other registers, or mark them as clobbered
    push eax                   ; instead just pass u32* parms
syscall_stage0_fixup:
    mov eax, 0
    push eax
    call syscall_handler       ; returns value in eax
    add esp, 4                 ; drop u32* parms
    iret
syscall_stage0_end equ $
