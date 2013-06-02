
#include "frotzos.h"
#include "io.h"
#include "vgatext.h"

#define TIMER_HZ 100

volatile double seconds = 0.0; // seconds since boot

void enable_interrupts();
void isr_keyboard();
void isr_timer();
extern void page_fault(u32 errcode);

void unhandled_irq(u32 irq)
{
    kprintf("Unhandled IRQ%d\r\n", irq);
}

void unhandled(u32 excnum, u32 errcode,
               u32 edi, u32 esi, u32 ebp, u32 esp,
               u32 ebx, u32 edx, u32 ecx, u32 new_eax,
               u32 eax, u32 eip, u32 cs, u32 eflags)
{
#ifdef DEBUG
    static const char *exc_names[] = {
        "divide-by-zero", "debug", "NMI", "breakpoint",
        "overflow", "bounds", "invalid opcode", "device n/a",
        "double fault", "coproc", "invalid tss", "segment not present",
        "stack-segment", "GPF", "page fault", "reserved",
        "x87 fpe", "alignment", "machine check", "simd fpe",
    };

    kprintf("Unhandled exception %d (%s): eip=0x%x\r\n", excnum, exc_names[excnum], eip);
#endif

    halt();
}

void *exc_handlers[32] = { unhandled };
void *irq_handlers[16] = { unhandled_irq };


static
void timer_phase(int hz)
{
    int divisor = 1193180 / hz;
    out8(0x43, 0x36);             // Counter0, LSB then MSB, square wave
    out8(0x40, divisor & 0xFF);   // send LSB
    out8(0x40, divisor >> 8);     // send MSB
}

static inline void a20wait()
{
    while (in8(0x64) & 0x2)
        ;
}

static inline void a20wait2()
{
    while (! (in8(0x64) & 0x1))
        ;
}

void
enable_A20()
{
    a20wait();
    out8(0x64, 0xad);
    a20wait();
    out8(0x64, 0xd0);
    a20wait2();
    int r = in8(0x60);
    a20wait();
    out8(0x64, 0xd1);
    a20wait();
    out8(0x60, r | 0x2);
    a20wait();
    out8(0x64, 0xae);
    a20wait();
}

void
exception_handler(int n)
{
    if (n >= 0x20) {
        void (*h)(int);
        h = irq_handlers[n - 0x20];
        h(n - 0x20);
        if (n >= 0x28) out8(0xa0, 0x20);
        if (n >= 0x20) out8(0x20, 0x20);
    } else {
        void (*h)(int, int);
        h = exc_handlers[n];
        h(n, 0);
    }
}

void
set_exception(int excnum, void *codeptr)
{
#if 1
    u32 *idt = (u32 *) 0x1000;
    idt[excnum*2] = 0x00080000 | (((u32) codeptr) & 0x0000ffff);
    idt[excnum*2 + 1] = 0x00008E00 | (((u32) codeptr) & 0xffff0000);
#endif
}

void
enable_interrupts()
{
    // set up 8259 PIC for hardware interrupts at 0x20/0x28

    out8( 0x20, 0x11);  // expect ICW4
    out8( 0x21, 0x20);  // IRQ0 is int 0x20
    out8( 0x21, 0x04);  // slave is on IRQ2
    out8( 0x21, 0x01);  // manual EOI
    out8( 0x21, 0x0);   // unmask all ints

    out8( 0xA0, 0x11);  // expect ICW4
    out8( 0xA1, 0x28);  // IRQ8 is int 0x28
    out8( 0xA1, 0x02);  // i am attached to IRQ2
    out8( 0xA1, 0x01);  // manual EOI
    out8( 0xA1, 0x0);   // unmask all ints

    // call timer isr at 100hz
    timer_phase(TIMER_HZ);

#define EXCEPTION(N, FUNC) \
    extern void *asm_exc##N; \
    set_exception(N, &asm_exc##N); \
    exc_handlers[N] = FUNC;

    EXCEPTION(0, unhandled);
    EXCEPTION(1, unhandled);
    EXCEPTION(2, unhandled);
    EXCEPTION(3, unhandled);
    EXCEPTION(4, unhandled);
    EXCEPTION(5, unhandled);
    EXCEPTION(6, unhandled);
    EXCEPTION(7, unhandled);
    EXCEPTION(8, unhandled);
    EXCEPTION(9, unhandled);
    EXCEPTION(10, unhandled);
    EXCEPTION(11, unhandled);
    EXCEPTION(12, unhandled);
    EXCEPTION(13, unhandled);
    EXCEPTION(14, page_fault);

#define IRQ(N, FUNC) \
    extern void *asm_irq##N; \
    set_exception(0x20 + N, &asm_irq##N); \
    irq_handlers[N] = FUNC;

    IRQ(0, isr_timer);
    IRQ(1, isr_keyboard);
    IRQ(2, unhandled);
    IRQ(3, unhandled);
    IRQ(4, unhandled);
    IRQ(5, unhandled);
    IRQ(6, unhandled);
    IRQ(7, unhandled);
    IRQ(8, unhandled);
    IRQ(9, unhandled);
    IRQ(10, unhandled);
    IRQ(11, unhandled);
    IRQ(12, unhandled);
    IRQ(13, unhandled);
    IRQ(14, unhandled);
    IRQ(15, unhandled);

    *(u32 *) 0x7df8 = (u32) exception_handler;

    // enable interrupts on processor
    asm volatile ("sti");
}

void setch(int x, int y, char ch, char attr)
{
    vga_charptr(x, y)[0] = ch;
    vga_charptr(x, y)[1] = attr;
}

void isr_timer()
{
    seconds += (1.0 / TIMER_HZ);

    static const char spinny[] = "\\|/-";
    vga_charptr(80, 1)[0] = spinny[(int) (seconds) % (sizeof(spinny)-1)];
}

extern void key_released(int sc);
extern void key_pressed(int sc);

static unsigned char keyqueue[128];       // classic ring queue
static unsigned int kqfront=0, kqback=0; // (kqend - kqstart) % 16 == size

int pop_scancode()
{
    if (kqback == kqfront) { // queue is empty
        return -1;
    }

    unsigned char k = keyqueue[kqfront++];
    kqfront %= sizeof(keyqueue);
    return k;
}

void
isr_keyboard()
{
    int scancode = in8(0x60);

    if ((kqback + 1) % sizeof(keyqueue) != kqfront)
    {
        keyqueue[kqback++] = scancode;
        kqback %= sizeof(keyqueue);
    } 
    // else os_fatal("kb queue is full");
}

void yield()
{
    asm volatile ("hlt");
}

void halt()
{
    asm volatile ("1: hlt; jmp 1b");
}

unsigned long long rdtsc(void)
{
    unsigned long long int x;
    asm volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

