#ifndef KERNEL_H_
#define KERNEL_H_

#include <sys/types.h>
#include "x86.h"

void init_kernel();
void kprintf(const char *fmt, ...);

#ifndef DEBUG
#define DEBUG(args...)
#endif

// virtual memory
#define PAGE_TABLES ((u32 *) 0xffc00000)
#define PAGE_DIR ((u32 *) 0xfffff000)
#define DISK_MAP_ADDR 0x10000000
#define DISK_MAP_ADDR_MAX 0xf0000000
#define HEAP_ADDR 0x100000
#define HEAP_ADDR_MAX 0xf00000

void page_fault(u32 errcode);

// interrupts
#define NUM_INTERRUPTS 64
#define IDT_BASE ((void *) 0x1000)

void setup_interrupts(void *idtaddr);

extern void *exc_handlers[];
extern void *irq_handlers[];

#endif
