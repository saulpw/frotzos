#include "assert.h"
#include "kernel.h"
#include "dev/ata.h"

void
setup_hdd()
{
    int ndevs = init_ata();
    int i;
    for (i=0; i < ndevs; ++i)
    {
        ata_disk *d = &disks[i];

        IDENTIFY_DEVICE_DATA id;

        if (ata_identify_device(d, &id)) {
            DPRINT(0, "ata %x:%d (%s) error with IDENTIFY_DEVICE", 
                    d->base_port, d->devnum, ata_types[d->type]);
            continue;
        }
    }
}
