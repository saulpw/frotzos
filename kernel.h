#ifndef KERNEL_H_
#define KERNEL_H_

#include <sys/types.h>
#include "x86.h"

void init_kernel();
void kprintf(const char *fmt, ...);

#ifndef DEBUG
#define DEBUG(args...)
#endif

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

#define PAGE_TABLES ((u32 *) 0xffc00000)
#define PAGE_DIR    ((u32 *) 0xfffff000)

void page_fault(u32 errcode);

// flush dirty pages to disk1
int ksync();

// interrupts
#define NUM_INTERRUPTS 64
#define IDT_BASE ((void *) 0x1000)

void setup_interrupts(void *idtaddr);

#endif
