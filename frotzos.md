
# Bootloader overview

Loads rest of disk (up to 600k) to 0x8000, jumps to 32-bit protected mode,
enables A20 (>1MB memory).  provides small stack growing down from 0x6000
(~20k), and jumps to the first file in the elifs.

* enables A20 line for >1MB

* reads one sector at a time via int 13h/02
  * (DEBUG) prints . for each sector
* prints ! on sector read error
* prints < if more than 10 errors
* (DEBUG) prints > just before jumping to protected mode

* loads GDT, jumps to protected mode

* jumps to first file (at 0x8020 if filename length == 16 bytes)

# memory layout

## physical 

(page 0) 0x 0000 - 0x 07ff: real mode IDT and bios data
(page 1) 0x 1000 - 0x 11ff: IDT (64 entries)
         0x 1200 - 0x 1aff: ISR stage0 stubs
(page 2)                  :
(page 3)                  : PDE
(page 4)                  : PT0
(page 5)                  : application/kernel stack (grows down)
(page 6)
(page 7) 0x07000 - 0x07bff: TLS (gs register)
         0x07c00 - 0x07dff: boot sector
         0x07e00 - 0x07e67: TSS
(page 8+) 0x08000 - 0x90000: disk sectors 1-? kernel loaded by bootloader
         0x100000-0xf00000: 14MB free pages to allocate

## virtual

0x0000: not present (to catch null references)
0x10001MB:  1MB identity mapped (kernel, stack, video memory)
0x  100000-0x  f00000 : heap
0x10000000-0xf0000000 : ide0 mmap
0xffc00000: all page tables
0xfffff000: page dir itself

