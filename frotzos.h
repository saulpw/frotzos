#ifndef FROTZOS_H_
#define FROTZOS_H_

#include <string.h>
#include "frotz.h"

extern volatile char _TEXTMODE_BUFFER[];
extern int cursor_x, cursor_y;

void movecur(int dx, int dy);
void setch(char ch);

#define NOTIMPL TRACE(NOTIMPL)
#define TRACE(X) trace(__FUNCTION__, __FILE__, __LINE__, #X)
extern void trace(const char *func, const char *fn, int line, const char *note);

extern void yield();    // hlt, block until interrupt
extern void halt();     // hlt forever
extern void enable_interrupts();
extern void isr_keyboard();
extern void isr_timer();

extern unsigned long long rdtsc(void);

static inline void outb( unsigned short port, unsigned char val ) {
    asm volatile( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline unsigned char inportb(unsigned int port) {
   unsigned char ret;
   asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
   return ret;
}

#endif
