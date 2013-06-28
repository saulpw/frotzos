#include <string.h> // memset

extern void do_kernel();

extern char START_BSS[], END_BSS[];

void kmain()
{
    memset(START_BSS, 0, END_BSS - START_BSS);

    do_kernel();
}

