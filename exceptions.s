[BITS 32]

extern exc_handlers, irq_handlers, syscall_handler

%macro EXCEPTION_ERRCODE 1
global asm_exc%1
asm_exc%1:
    xchg eax, [esp]            ; eax = error code
    pushad                     ; save all registers
    push eax                   ; arg2 = errcode
    push %1                    ; arg1 = exception #
    push exception_end         ; unified return address
    jmp [exc_handlers + %1*4] ; exception_handlers[excnum](exc#, errcode, ...)
%endmacro

%macro EXCEPTION 1
global asm_exc%1
asm_exc%1:
    push eax                   ; for consistency with above
    pushad                     ; save all registers
    push eax                   ; arg2 = fake errcode for consistency
    push %1                    ; arg1 = exception #
    push exception_end         ; unified return address
    jmp [exc_handlers + %1*4] ; exception_handlers[excnum](exc#, 0, ...)
%endmacro

global exception_end
exception_end:
    pop eax
    pop eax
    popad
    pop eax
    iret

%macro IRQ 1
global asm_irq%1
asm_irq%1:
    pushad                     ; save all registers
    push %1                    ; arg1 = irq #
%if %1 < 8
    push master_irq_end
%else
    push slave_irq_end
%endif
    jmp [irq_handlers + %1*4] ; irq_handlers[irq#](irq#, ...)
%endmacro

global slave_irq_end, master_irq_end
slave_irq_end:
    mov al, 0x20
    out 0xA0, al               ; EOI to slave PIC
    ; fall-through

master_irq_end:
    mov al, 0x20
    out 0x20, al               ; EOI to master PIC
    pop eax
    popad
    iret

global asm_syscall
asm_syscall:
    mov eax, esp
    add eax, 12                ; don't make eip, cs, eflags args
    ; XXX: need to save other registers, or mark them as clobbered
    push eax                   ; instead just pass u32* parms
    call syscall_handler       ; returns value in eax
    add esp, 4                 ; drop u32* parms
    iret

EXCEPTION 0
EXCEPTION 1
EXCEPTION 2
EXCEPTION 3
EXCEPTION 4
EXCEPTION 5
EXCEPTION 6
EXCEPTION 7
EXCEPTION_ERRCODE 8
EXCEPTION 9
EXCEPTION_ERRCODE 10
EXCEPTION_ERRCODE 11
EXCEPTION_ERRCODE 12
EXCEPTION_ERRCODE 13
EXCEPTION_ERRCODE 14
;;EXCEPTION 15
;EXCEPTION 16
;EXCEPTION_ERRCODE 17
;EXCEPTION 18
;EXCEPTION 19

IRQ 0
IRQ 1
IRQ 2
IRQ 3
IRQ 4
IRQ 5
IRQ 6
IRQ 7
IRQ 8
IRQ 9
IRQ 10
IRQ 11
IRQ 12
IRQ 13
IRQ 14
IRQ 15

