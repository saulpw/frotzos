#include <assert.h>
#include <string.h>
#include "x86.h"
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
page_fault(u32 errcode, const struct registers *regs)
{
    u32 faultaddr = get_cr2();

    if (faultaddr < 0x1000) {
        dump_regs(regs);
        kprintf("NULL page fault at 0x%x\r\n", faultaddr);
        halt();
    }

    if (errcode & 0x2) { // caused by page write    
        if (PAGE_DIR[faultaddr >> 22] & 0x1) {
            if ((PAGE_TABLES[faultaddr >> 12] & 0x3) == 0x1) {
                // present but not writable
                kprintf("invalid write to 0x%08X\r\n", faultaddr);
                halt();
            }
        }
    }

    DPRINT(2, "page fault at 0x%x", faultaddr);
    int disknum = -1;
    int lba = 0;

    if ((errcode & 0x1) == 0x0) { // not present 
        // check for valid entry in PAGE_DIR
        if ((PAGE_DIR[faultaddr >> 22] & 0x1) == 0) // not present
        {
            PAGE_DIR[faultaddr >> 22] = get_phys_page() | 0x3;
        }
        else
        {
            u32 entry = PAGE_TABLES[faultaddr >> 12];

            if ((entry & 0x2) == 0x2) // on disk
            {
                disknum = (entry >> 2) & 0x3;
                lba = entry >> 4;
            }

            // fill in page table entry with free page
            PAGE_TABLES[faultaddr >> 12] = get_phys_page() | 0x3; // RW + PRESENT
        }
    }

    void *pageaddr = (void *) (faultaddr & 0xfffff000);

#if 0
    if (faultaddr >= DISK0_MAP_ADDR && faultaddr < DISK0_MAP_ADDR_MAX)
    {
        unsigned int diskoffset = faultaddr - DISK0_MAP_ADDR;
        unsigned int pagenum = diskoffset >> 12;

        disknum = 0;
        lba = pagenum * 8;
    }
    else if (faultaddr >= DISK1_MAP_ADDR && faultaddr < DISK1_MAP_ADDR_MAX)
    {
        unsigned int diskoffset = faultaddr - DISK1_MAP_ADDR;
        unsigned int pagenum = diskoffset >> 12;

        lba = pagenum * 8;
        disknum = 1;
    }
#endif
    if (disknum == -1) {
        memset(pageaddr, 0, 4096);
    } else {
#ifdef ATA_DMA
        int rc = dma_pci_lba28(disknum   // device
                             , CMD_READ_DMA
                             , 0        // feature
                             , 8        // sectorCount
                             , lba      // LBA
                             , (void *) 0x10000  // pageaddr // bufAddr
                             , 8        // numSect
                             );

        if (rc != 0) {
            kprintf("unable to read 0x%08X from disk%d (LBA=%d)\r\n", pageaddr, disknum, lba);
        }
#else
//        if (*boot_drive == 0x80) {
            int i=0;
            for (i=0; i < 8; ++i) {
                int rc = reg_pio_data_in_lba28(disknum   // device
                                , CMD_READ_SECTORS
                                , 0        // feature
                                , 1        // sectorCount
                                , lba+i    // LBA
                                , pageaddr + i*512 // bufAddr
                                , 1        // numSect
                                , 0        // multicnt
                             );
                if (rc != 0) {
                    DPRINT(0, "unable to read LBA %d from disk%d (0x%08X)", lba, disknum, pageaddr);
                    if (i == 0) {
                        halt();
                    }
                }
            }
//        } else if (*boot_drive == 0xe0) {
//        }
#endif

        // after reading the page from disk, make the page read-only if
        //   it comes from disk0

        if (disknum == 0) {
//            PAGE_TABLES[faultaddr >> 12] &= ~0x2; // not writable
        }
    }
}

int
write_sector(const void *ptr)
{
    if (ptr < (void *) DISK1_MAP_ADDR || ptr > (void *) DISK1_MAP_ADDR_MAX) {
        kprintf("ptr to write (0x%08X) not on disk1", ptr);
        return -1;
    }

    uint32_t diskoffset = (uint32_t) ptr - DISK1_MAP_ADDR;
    int lba = diskoffset >> 9;

    void *src = (void *) (((uint32_t) ptr) & 0xfffffe00);

    DPRINT(1, "writing sector %u from 0x%x", lba, src);

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
        kprintf("unable to write sector to disk");
        return -1;
    }
    return 0;
}

int
write_page(u32 ptr)
{
    if (ptr < DISK1_MAP_ADDR || ptr > DISK1_MAP_ADDR_MAX) {
        kprintf("page to write (0x%08X) not on disk1", ptr);
        return -1;
    }

    void *src = (void *) (ptr & 0xfffff000);
    uint32_t diskoffset = (uint32_t) src - DISK1_MAP_ADDR;
    int lba = diskoffset >> 9;

    DPRINT(0, "writing page at lba %u from 0x%x", lba, src);

    int rc = reg_pio_data_out_lba28(1    // writable device
                                  , CMD_WRITE_SECTORS
                                  , 0    // feature
                                  , 8    // sectorCount
                                  , lba  // LBA
                                  , src  // bufAddr
                                  , 8    // numSect
                                  , 0    // multiCnt
                                  );
    if (rc != 0) {
        kprintf("unable to write page to disk");
        return -1;
    }
    return 0;
}

int
is_dirty(u32 addr)
{
    return (PAGE_DIR[addr >> 22] & 0x01)     // page table itself is Present
        && (PAGE_TABLES[addr >> 12] & 0x40); // DIRTY bit is set
}

int
ksync()
{
    int ret = 0;
    u32 ptr = DISK1_MAP_ADDR;
    for ( ; ptr < DISK1_MAP_ADDR_MAX; ptr += 4096)
    {
        if (is_dirty(ptr)) {
            DPRINT(1, "page at 0x%08X is dirty, writing to disk", ptr);
            if (write_page(ptr) != 0)
                ret = -1; // but keep writing the rest
        }
    }
    return ret;
}

// makes sure the PAGE_DIR entry exists first
void *
set_pt_entry(u32 addr, u32 val)
{
    if (PAGE_DIR[addr >> 22] == 0) {
        PAGE_DIR[addr >> 22] = get_phys_page() | 0x3;
    }
    PAGE_TABLES[addr >> 12] = val;
    return (void *) addr;
}

u32 get_pt_entry(u32 addr)
{
    return PAGE_TABLES[addr >> 12];
}


void *
map(void *dest, const void *src, size_t len)
{
    u32 d = (u32) dest;
    u32 s = (u32) src;

    size_t i=0;
    while (i < len)
    {
//        assert (get_pt_entry(d + i) == 0);

        set_pt_entry(d + i, get_pt_entry(s + i));

        i += 4096;
    }
    return dest;
}

void
init_pagetable()
{
    size_t lba = 0;
    size_t i=0;

    u32 d = 0x10000000;
    while (i < 0x400000)
    {
        u32 val = (lba << 4) | 0x2;  // not-present, on-disk, disk0
        set_pt_entry(d + i, val); 
        i += 4096;
        lba += 4096 / 512;
    }
}
