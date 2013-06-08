
#include <string.h> // memset
#include "kernel.h"
#include "elifs.h"

extern void main(int argc, char **argv);
extern char START_BSS[], END_BSS[];

void kmain()
{
    static char *argv[3] = { "frotz", "story.z5", 0 };

    memset(START_BSS, 0, END_BSS - START_BSS);

    init_kernel();

    struct fz_filehdr * const *files = elifs_enumfiles();
    argv[1] = files[2]->name;

    main(2, argv);
}

