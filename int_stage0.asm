[BITS 32]

global irq_stage0_start, irq_stage0_fixup, irq_stage0_end
global exc_stage0_start, exc_stage0_fixup, exc_stage0_end
global excerr_stage0_start, excerr_stage0_fixup, excerr_stage0_end
global syscall_stage0_start, syscall_stage0_fixup, syscall_stage0_end
global asm_halt

extern exception_handler, irq_handler, syscall_handler

exc_handler_ptr dd exception_handler
irq_handler_ptr dd irq_handler
;sys_handler_ptr dd syscall_handler

asm_halt:
    hlt
    jmp asm_halt

excerr_stage0_start:
    xchg eax, [esp]            ; eax = error code
    pushad                     ; save all registers
    push eax                   ; arg2 = errcode
excerr_stage0_fixup:    
    mov eax, 0
    push eax                   ; arg1 = exception #
    push exception_end         ; unified return address
    jmp [exc_handler_ptr]      ; exception_handler(exc#, errcode, ...)
excerr_stage0_end equ $

exc_stage0_start:
    push eax                   ; for consistency with above
    pushad                     ; save all registers
    push eax                   ; arg2 = fake errcode for consistency
exc_stage0_fixup:              ; "mov eax" is only 1 byte, literal follows
    mov eax, 0
    push eax                   ; arg1 = exception #
    push exception_end         ; unified return address
    jmp [exc_handler_ptr]      ; exception_handler(exc#, errcode, ...)
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
    push eax                   ; arg1 = IRQ #
    push irq_end
    jmp [irq_handler_ptr]      ; irq_handler(irq#, ...)
irq_stage0_end equ $

global irq_end
irq_end:
    pop eax
    cmp al, 8                  ; ACK IRQ0-7 on master only
    jb master_irq_end

    mov al, 0x20
    out 0xA0, al               ; EOI to slave PIC

master_irq_end:
    mov al, 0x20
    out 0x20, al               ; EOI to master PIC
    popad
    iret

%if 0
global syscall_stage0_start
syscall_stage0_start:
    mov eax, esp
    add eax, 12                ; don't make eip, cs, eflags args
    ; XXX: need to save other registers, or mark them as clobbered
    push eax                   ; instead just pass u32* parms
syscall_stage0_fixup:
    mov eax, 0
    push eax
    call [sys_handler_ptr]     ; returns value in eax
    add esp, 4                 ; drop u32* parms
    iret
syscall_stage0_end equ $
%endif
