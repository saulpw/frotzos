#include "kernel.h"
#include "dev/time.h"
#include "dev/ata.h"

#include "iso9660.h"
#include "DiskFile.h"

void
do_kernel()
{
    init_pagetable();
    DPRINT(0, "starting frotzos");
    setup_timer();

    setup_interrupts(IDT_BASE);

    init_syscalls();

    setup_hdd();

    const DiskFile * files = iso9660_enumfiles();

    const char *argv[3] = { files[3].filename, files[5].filename, 0 };

    // flat binary loader, just copies the file to known location
    __builtin_memcpy((void *) 0x1000000, files[3].data, files[3].length);

    void (*appmain)();
    appmain = (void (*)()) 0x1000000;
    appmain(2, argv);
}

