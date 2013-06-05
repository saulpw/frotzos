#ifndef _X86_HARDWARE_H_
#define _X86_HARDWARE_H_

static inline void yield()
{
    asm volatile ("hlt");
}

static inline void halt()
{
    while (1)
        yield();
}

static inline unsigned long long rdtsc(void) // read time-stamp counter
{
    unsigned long long int x;
    asm volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

#endif
