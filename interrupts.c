#include <sys/types.h>
#include <string.h>
#include "io.h"
#include "kernel.h"
#include "dev/kb.h"
#include "dev/time.h"

/*
 * IDT at 0x1000
 *    exceptions at INT 00h-1fh
 *          IRQs at INT 20h-2fh
 *      syscalls at INT 30h-3fh
 * stage 0 handlers at 0x1200 + MAX_S0_LEN*intnum
 */

void irq_handler(u32 irq)
{
    switch (irq) {
    case 0: isr_timer(); break;
    case 1: isr_keyboard(); break;
    default:
            kprintf("Unhandled IRQ%d\r\n", irq);
            break;
    };
}

void exception_handler(u32 exc, u32 errcode,
               u32 edi, u32 esi, u32 ebp, u32 esp,
               u32 ebx, u32 edx, u32 ecx, u32 new_eax,
               u32 eax, u32 eip, u32 cs, u32 eflags)
{
    switch (exc) {
        case 14: page_fault(errcode); break;

        case 0:  // divide-by-zero
        case 1:  // debug
        case 2:  // nmi
        case 3:  // breakpoint
        case 4:  // overflow
        case 5:  // bounds
        case 6:  // invalid opcode
        case 7:  // device n/a
        case 8:  // double fault
        case 9:  // coproc
        case 10: // invalid tss
        case 11: // segment not present
        case 12: // stack-segment
        case 13: // GPF
        case 16: // x87 fpe
        case 17: // alignment
        case 18: // machine check
        case 19: // simd fpe
        default:
            kprintf("Unhandled exception %d at EIP=%08X  ESP=%08X  EBP=%08X\r\n", exc, eip, esp, ebp);
            halt();
    };
}

#define MAX_S0_LEN 32

extern u32 irq_stage0_start, irq_stage0_fixup, irq_stage0_end;
extern u32 exc_stage0_start, exc_stage0_fixup, exc_stage0_end;
extern u32 excerr_stage0_start, excerr_stage0_fixup, excerr_stage0_end;
extern u32 syscall_stage0_start, syscall_stage0_fixup, syscall_stage0_end;
extern u32 asm_halt;

static void
create_handler(u8 *h, void *start, void *fixup, void *end, int intnum)
{
    memcpy(h, (const void *) start, end - start);
    h[fixup - start + 1] = intnum;
}

static inline void
set_idt_entry(u32 *idtentry, void *handler_addr)
{
    idtentry[0] = 0x00080000 + (u32) handler_addr;
    idtentry[1] = 0x00008E00;
}

void
lidt(void *base, unsigned int limit)
{
   volatile u16 idtr[3];

   idtr[0] = limit;
   idtr[1] = (u32) base;
   idtr[2] = (u32) base >> 16;

   asm volatile ("lidt (%0)": :"r" (idtr));
}

void
create_idt(u32 *idt) // and also stage0 interrupt stubs after the IDT
{
    u8 *handler_addr = (u8 *) (idt + 2*NUM_INTERRUPTS);

    int i;
    for (i=0; i < NUM_INTERRUPTS; ++i)
    {
        if (i == 8) { // double fault
            set_idt_entry(&idt[i*2], (u8 *) asm_halt);
        } else {
            set_idt_entry(&idt[i*2], handler_addr);
        }

        if (i == 8 || i == 10 || i == 11 || i == 12 || 
                      i == 13 || i == 14 || i == 17)    // with errcode
        {
            create_handler(handler_addr, &excerr_stage0_start,
                                         &excerr_stage0_fixup,
                                         &excerr_stage0_end,
                                         i);
        }
        else if (i < 0x20) // exception without errcode
        {
            create_handler(handler_addr, &exc_stage0_start,
                                         &exc_stage0_fixup,
                                         &exc_stage0_end,
                                         i);
        }
        else if (i < 0x30) // irq
        {
            create_handler(handler_addr, &irq_stage0_start,
                                         &irq_stage0_fixup,
                                         &irq_stage0_end,
                                         i - 0x20);
        }
        else // syscall
        {
            create_handler(handler_addr, &syscall_stage0_start,
                                         &syscall_stage0_fixup,
                                         &syscall_stage0_end,
                                         i - 0x30);
        }
        handler_addr += MAX_S0_LEN;
    }
}

void
setup_pic(u8 master_int, u8 slave_int)
{
    out8(0x20, 0x11);        // expect ICW4
    out8(0x21, master_int);  // IRQ0 is at INT master_int
    out8(0x21, 0x04);        // slave is on IRQ2
    out8(0x21, 0x01);        // manual EOI
    out8(0x21, 0x0);         // unmask all ints

    out8(0xA0, 0x11);        // expect ICW4
    out8(0xA1, slave_int);   // IRQ8 is at INT slave_int
    out8(0xA1, 0x02);        // i am attached to IRQ2
    out8(0xA1, 0x01);        // manual EOI
    out8(0xA1, 0x0);         // unmask all ints
}

void
setup_interrupts(void *idtaddr)
{
    // set up 8259 PIC for hardware interrupts at 0x20/0x28
    setup_pic(0x20, 0x28);

    // create IDT and handlers
    create_idt(idtaddr);

    // load IDTR
    lidt(idtaddr, 8*NUM_INTERRUPTS-1);

    // enable interrupts on processor
    asm volatile ("sti");
}

