#ifndef KERNEL_H_
#define KERNEL_H_

#include <sys/types.h>

extern void init_kernel();
extern void kprintf(const char *fmt, ...);

extern void yield();    // hlt, block until interrupt
extern void halt();     // hlt forever

#ifndef DEBUG
#define DEBUG(args...)
#endif

// virtual memory
#define PAGE_TABLES ((u32 *) 0xffc00000)
#define PAGE_DIR ((u32 *) 0xfffff000)

extern void page_fault(u32 errcode);

// interrupts
#define NUM_INTERRUPTS 64
#define IDT_BASE ((void *) 0x1000)

extern void setup_interrupts(void *idtaddr);

extern void *exc_handlers[];
extern void *irq_handlers[];

// timer
#define TIMER_HZ 100

extern volatile double seconds;        // seconds since start

extern void setup_timer();
extern void isr_timer();

static inline unsigned long long rdtsc(void) // read time-stamp counter
{
    unsigned long long int x;
    asm volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

// keyboard

extern int pop_scancode();
extern void isr_keyboard();

#endif
