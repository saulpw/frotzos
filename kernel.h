#ifndef KERNEL_H_
#define KERNEL_H_

#include <sys/types.h>
#include "x86.h"

void init_kernel();
void kprintf(const char *fmt, ...);

#ifndef DEBUG
#define DEBUG 0
#endif

// DPRINT(0, ...) always prints
// DPRINT(1, ...) prints if DEBUG >= 1
#define DPRINT(L, FMT, args...) do { \
        if (DEBUG >= L) kprintf(FMT "\r\n", ##args); \
    } while (0)

// physical memory
#define FIRST_PHYS_PAGE        0x200000

// virtual memory
#define HEAP_ADDR              0x200000
#define HEAP_ADDR_MAX          0xf00000

// disk0 is read-only (3.5GB)
#define DISK0_MAP_ADDR       0x10000000
#define DISK0_MAP_ADDR_MAX   0xefffffff

// disk1 is writable (252MB)
#define DISK1_MAP_ADDR       0xf0000000
#define DISK1_MAP_ADDR_MAX   0xffbfffff

static u32 * const PAGE_TABLES = ((u32 *) 0xffc00000);
static u32 * const PAGE_DIR    = ((u32 *) 0xfffff000);

// does a copy from src to dest via page tables
void init_pagetable();
void *map(void *dest, const void *src, size_t length);
void page_fault(u32 errcode, const struct registers *regs);

// flush dirty pages to disk1
int ksync();

// interrupts
#define NUM_INTERRUPTS 64
#define IDT_BASE ((void *) 0x1000)

void setup_interrupts(void *idtaddr);

void setup_hdd();

void init_syscalls();

#endif
