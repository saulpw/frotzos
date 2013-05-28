
#include "frotzos.h"

extern void main(int argc, char **argv);
extern char START_BSS[], END_BSS[];

void kmain()
{
    static char *argv[3] = { "frotz", 0 };

    extern void enable_A20();
    enable_A20();

    memset(START_BSS, 0, END_BSS - START_BSS);

    main(1, argv);

    halt();
}

