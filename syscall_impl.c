#include "syscall_impl.h"

// see frotzos.md for memory layout
uint32_t **g_psp = (uint32_t **) SYSCALL_PSP;
syscall_func_t *syscalls = (syscall_func_t *) SYSCALL_TABLE;

void push_double(double d)
{
    *g_psp -= 2;
    *(double *) *g_psp = d;
}

double pop_double()
{
    double r = *(const double *) *g_psp;
    *g_psp += 2;
    return r;
}

void push_single(uint32_t n)
{
    *g_psp -= 1;
    **g_psp = n;
}

uint32_t pop_single()
{
    uint32_t r = **g_psp;
    *g_psp += 1;
    return r;
}


