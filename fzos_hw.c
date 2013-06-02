
#include "frotzos.h"
#include "io.h"
#include "vgatext.h"

#define TIMER_HZ 100
#define IDT_BASE ((void *) 0x1000)

volatile double seconds = 0.0; // seconds since boot

void setup_interrupts(void *idtaddr);
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

void *exc_handlers[32] = { 
    unhandled, unhandled, unhandled, unhandled,
    unhandled, unhandled, unhandled, unhandled,
    unhandled, unhandled, unhandled, unhandled,
    unhandled, unhandled, page_fault, unhandled,
    unhandled, unhandled, unhandled, unhandled,
    unhandled, unhandled, unhandled, unhandled,
    unhandled, unhandled, unhandled, unhandled,
    unhandled, unhandled, unhandled, unhandled,
};

void *irq_handlers[16] = {
    isr_timer, isr_keyboard, unhandled_irq, unhandled_irq,
    unhandled_irq, unhandled_irq, unhandled_irq, unhandled_irq,
    unhandled_irq, unhandled_irq, unhandled_irq, unhandled_irq,
    unhandled_irq, unhandled_irq, unhandled_irq, unhandled_irq,
};

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
init_kernel()
{
    timer_phase(TIMER_HZ);

    setup_interrupts(IDT_BASE);
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

