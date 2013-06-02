
#include <string.h>
#include "io.h"
#include "kernel.h"
#include "kdev_ata.h"

static inline u32
get_cr2()
{
    u32 __force_order;
    u32 val;
    asm volatile("mov %%cr2,%0\n\t" : "=r" (val), "=m" (__force_order));
    return val;
}

// high-water mark for allocated physical pages
static u32 nextpage = 0x100000;        // 1MB

u32 get_phys_page()
{
    nextpage += 0x1000;

    return nextpage - 0x1000;
}

void
page_fault(u32 errcode)
{
    u32 faultaddr = get_cr2();

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

    if (faultaddr >= 0x10000000 && faultaddr < 0xf0000000)
    {
        // 256MB-3.8GB is mmap'ed disk
        // if page backed by disk, load from disk
        
        int diskoffset = faultaddr - 0x10000000;
        int pagenum = diskoffset >> 12;
        int sectornum = pagenum * 8;
        int lba = sectornum + 1;

        void *dest = (void *) (faultaddr & 0xfffff000);

        int rc = reg_pio_data_in_lba28(0    // device
                                     , CMD_READ_SECTORS
                                     , 0    // feature
                                     , 8    // sectorCount
                                     , lba  // LBA
                                     , dest // bufAddr
                                     , 8    // numSect
                                     , 0    // multiCnt
                                     );
        if (rc != 0) {
            kprintf("unable to read page from disk");
            halt();
        }
         
    } else {
        // otherwise bzero
        memset((void *) (faultaddr & 0xfffff000), 0, 4096);
    }
}

