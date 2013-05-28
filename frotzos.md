
# Bootloader overview

Loads rest of disk (up to 600k) to 0x8000, jumps to 32-bit protected mode, sets
up interrupt and exception handlers, enables A20 (>1MB memory).  provides small
stack growing down from 0x6000 (~20k), and jumps to the first file in the stupidfs.

* enables A20 line for >1MB
* gets disk geometry via int 13h/48h

* reads one sector at a time via int 13h/02
  * (DEBUG) prints . for each sector
* prints ! on sector read error
* prints < if more than 10 errors
* (DEBUG) prints > just before jumping to protected mode

* loads GDT, jumps to protected mode
* creates IDT and interrupt-stubs
  * interrupts have their own stack at 0x1000

* jumps to first file (at 0x8020 if filename < 16 bytes)

# archivalfs overview

A linear stream of files, each with a minimal header with filename, length,
status (empty/writing/closed).  like a simple heap.  intentionally trivial to
reverse engineer and extract the original files.  should be able to handle
basic file operations in a running system.

header (all multi-byte fields little endian)
  * 4-byte magic sync token: FILE (little-endian, so reads "ELIF")
  * 4-byte type
  * 5-byte exact file length (up to 1TB, not including padding or header)
  * 1 byte reserved (for multi-terabyte files?)
  * 1 byte status: 0=empty, 1=existing, 2=writable for append
  * 1-byte filename length, including zero-padding for alignment
     * length <= 112: value in bytes
     * special values for name sizes >112 (240, 496, 1008, 2032, 4080 bytes)
  * utf8 filename (7-bit clean preferred), must be zero-terminated and
    padded to (at least) word boundary

only one 'appending' (status == 2) file can exist, and at the end of the
archive; it must be 'closed' (status == 1) before another file can be created.

file contents are linear and should be copied verbatim from the original source
as much as is reasonable (minus obsolete DRM for example).

can zero-pad between end of file and the next ELIF sync marker for alignment.

# memory layout

## physical 

(page 0) 0x00000 - 0x007ff: real mode IDT and bios data
         0x  800 - 0x  9ff: bootloader IDT (64 entries)
(page 1) 0x01000 - 0x013ff: ISR stage0 stubs
         0x01400 - 0x01fff: exception stack (grows down)
(page 2)                  :
(page 3)                  : PDE
(page 4)                  : PT0
(page 5)                  : 'application' stack (grows down)
(page 6) 0x06000 - 0x06fff: PDE

(page 7) 0x07000 - 0x07bff: TLS (gs register)
         0x07c00 - 0x07dff: boot sector
         0x07e00 - 0x07e67: TSS
(page 8+) 0x08000 - 0x90000: disk sectors 1-? kernel loaded by bootloader

## virtual

0x0000: not present (to catch null references)
0x1000-4MB:  identity mapped (kernel, stack, video memory)
0x10000000- : ide0 mmap

0xfffc0000: all page tables
0xfffff000: page dir itself

