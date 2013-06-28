#include "kernel.h"
#include "x86.h"
#include "vgatext.h"
#include "dev/time.h"

static volatile double g_seconds = 0.0L; // seconds since boot

double seconds() { return g_seconds; }

void setup_timer()
{
    int divisor = 1193180 / TIMER_HZ;
    out8(0x43, 0x36);             // Counter0, LSB then MSB, square wave
    out8(0x40, divisor & 0xFF);   // send LSB
    out8(0x40, divisor >> 8);     // send MSB
}

void isr_timer()
{
    g_seconds += (1.0L / TIMER_HZ);

    static const char spinny[] = "\\|/-";
    vga_charptr(79, 0)[0] = spinny[(int) (g_seconds) % (sizeof(spinny)-1)];
}

