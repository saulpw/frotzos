
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



