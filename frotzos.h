#ifndef FROTZOS_H_
#define FROTZOS_H_

#include <string.h>
#include "frotz.h"

extern int os_random_seed(void);

extern volatile char _TEXTMODE_BUFFER[];

#ifdef DEBUG

#define T(X) do {  \
    _TEXTMODE_BUFFER[158] = #X[0]; \
    _TEXTMODE_BUFFER[159]++; \
} while (0);

#else
#define T(X)

#endif

extern void os_fatal(const char *, ...); // noreturn
extern void yield();    // hlt, block until interrupt
extern void halt();     // hlt forever

extern unsigned long long rdtsc(void);

extern void trace(const char *funcname, const char *filename, int lineno, const char *note);


#define NOTIMPL TRACE(NOTIMPL)
#define TRACE(X) trace(__FUNCTION__, __FILE__, __LINE__, #X)

#endif
