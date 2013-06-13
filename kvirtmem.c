
#include <string.h>
#include "io.h"
#include "kernel.h"
#include "dev/ata.h"

static inline u32
get_cr2()
{
    u32 __force_order;
    u32 val;
    asm volatile("mov %%cr2,%0\n\t" : "=r" (val), "=m" (__force_order));
    return val;
}

// high-water mark for allocated physical pages
static u32 nextpage = FIRST_PHYS_PAGE;

u32 get_phys_page()
{
    nextpage += 0x1000;

    return nextpage - 0x1000;
}

void
page_fault(u32 errcode)
{
    u32 faultaddr = get_cr2();

    DEBUG("page fault at 0x%x\r\n", faultaddr);

    if (faultaddr < 0x1000) {
        kprintf("NULL page fault at 0x%x\r\n", faultaddr);
        halt();
    }

    // check for valid entry in PAGE_DIR
    if ((PAGE_DIR[faultaddr >> 22] & 0x1) == 0) // not present
    {
        PAGE_DIR[faultaddr >> 22] = get_phys_page() | 0x3;
    }

    // fill in page table entry with free page
    PAGE_TABLES[faultaddr >> 12] = get_phys_page() | 0x3; // RW + PRESENT

    void *pageaddr = (void *) (faultaddr & 0xfffff000);

    if (faultaddr < DISK0_MAP_ADDR || faultaddr >= DISK1_MAP_ADDR_MAX)
    {
        memset(pageaddr, 0, 4096);
    }
    else
    {
        int devnum = 0;
        int diskoffset = 0;
        int lba = 0;

        if (faultaddr >= DISK0_MAP_ADDR && faultaddr < DISK0_MAP_ADDR_MAX)
        {
            int diskoffset = faultaddr - DISK0_MAP_ADDR;
            int pagenum = diskoffset >> 12;

            lba = pagenum * 8;
        }
        else
        {
            devnum = 1;
            int diskoffset = faultaddr - DISK1_MAP_ADDR;
            int pagenum = diskoffset >> 12;
            lba = pagenum * 8;
        }

        int rc = dma_pci_lba28(devnum   // device
                             , CMD_READ_DMA
                             , 0        // feature
                             , 8        // sectorCount
                             , lba      // LBA
                             , (void *) 0x10000  // pageaddr // bufAddr
                             , 8        // numSect
                             );
        if (rc != 0) {
            kprintf("unable to read page from disk");
            halt();
        }
    }
}

int
write_sector(const void *ptr)
{
    if (ptr < (void *) DISK1_MAP_ADDR || ptr > (void *) DISK1_MAP_ADDR_MAX) {
        kprintf("ptr to write not on disk1");
        return -1;
    }

    uint32_t diskoffset = (uint32_t) ptr - DISK1_MAP_ADDR;
    int lba = diskoffset >> 9;

    void *src = (void *) (((uint32_t) ptr) & 0xfffffe00);

    DEBUG("writing sector %u from 0x%x\r\n", lba, src);

    int rc = reg_pio_data_out_lba28(1    // writable device
                                  , CMD_WRITE_SECTORS
                                  , 0    // feature
                                  , 1    // sectorCount
                                  , lba  // LBA
                                  , src  // bufAddr
                                  , 1    // numSect
                                  , 0    // multiCnt
                                  );
    if (rc != 0) {
        kprintf("unable to write page to disk");
        return -1;
    }
    return 0;
}
