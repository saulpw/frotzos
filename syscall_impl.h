#ifndef SYSCALL_IMPL_H_
#define SYSCALL_IMPL_H_

#include <stdint.h>

// see frotzos.md for memory layout
#define SYSCALL_PSP            0x2000
#define SYSCALL_STACK_BOTTOM   0x2800
#define SYSCALL_TABLE          0x2800

typedef void (*syscall_func_t)();
extern syscall_func_t *syscalls;
extern uint32_t **g_psp;

// index into syscalls[]
enum {
    SC_HALT=0,
    SC_TIME=1,
    SC_KEY,
    SC_WRITE,
    SC_ABORT,
    SC_MMAP_R,
    SC_OPEN_W,
};

void push_double(double d);
double pop_double();
void push_single(uint32_t);
uint32_t pop_single();

#endif

