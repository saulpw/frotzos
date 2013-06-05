
#include <string.h> // memset
#include "kernel.h"
#include "elifs.h"

extern void main(int argc, char **argv);
extern char START_BSS[], END_BSS[];

void kmain()
{
    static char *argv[3] = { "frotz", 0 };

    memset(START_BSS, 0, END_BSS - START_BSS);

    init_kernel();

    argv[1] = elifs_enumfiles()[1]->name;
    main(1, argv);
}

