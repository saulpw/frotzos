#include "kernel.h"

void __assert_failure(const char *msg)
{
    kprintf("%s\r\n", msg);
    halt();
}

