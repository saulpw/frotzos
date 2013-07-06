#include "assert.h"
#include "kernel.h"
#include "dev/ata.h"

void
setup_hdd()
{
#ifdef ATA_DMA
    dma_pci_config();
#endif

    int ndevs = init_ata();
    int i;
    // TODO: choose game disk and save disk

    int gamedisk = 0;       // if no atapi drives, first hdd is gamedisk
    for (i=0; i < ndevs; i++)
        if (disks[i].type == ATAPI)
        { 
            // otherwise first atapi drive is gamedisk
            gamedisk = i;
            break;
        }

    map_disk(gamedisk, GAMEDISK_ADDR);

    int savedisk = -1;
//    map_disk(savedisk, SAVEDISK_ADDR);
}

// read 4096 bytes from disk at byte 4096*pagenum into buf
//    returns 0 on success
int
hdd_read_page(int disknum, unsigned long pagenum, void *buf)
{
    ata_disk *d = &disks[disknum];

    int i;

    if (d->type == ATAPI) {
        for (i=0; i < 2; ++i)
           if (atapi_read_lba(d, buf + i*2048, 2048, pagenum*2 + i)) return -1;
    } else if (d->type == ATA) {
        for (i=0; i < 8; ++i)
           if (ata_read_lba28(d, buf + i*512, 512, pagenum*8 + i)) return -1;
    } else {
        DPRINT(0, "invalid type for disk %d", disknum);
        return -1;
    }

    return 0;
}

