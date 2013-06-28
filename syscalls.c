#include "syscalls.h"
#include "syscall_impl.h"

double seconds()
{
    syscalls[SC_TIME]();
    return pop_double();
}

unsigned int get_key()
{
    syscalls[SC_KEY]();
    return (unsigned int) pop_single();
}
