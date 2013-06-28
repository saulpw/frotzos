#include "dev/time.h"
#include "dev/kb.h"
#include "syscall_impl.h"

void syscall_TIME()
{
    push_double(seconds());
}

void syscall_KEY()
{
    push_single(get_key());
}


void init_syscalls()
{
    *g_psp = (uint32_t *) SYSCALL_STACK_BOTTOM;
#define SYSCALL(X) syscalls[SC_##X] = syscall_##X;
//    SYSCALL(HALT)
    SYSCALL(TIME)
    SYSCALL(KEY)
#undef SYSCALL
}
