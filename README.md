# Barebones Frotz platform

a return to the golden era of bootable games

This builds a floppy image that can be booted in a VM to run a .z5 program

## To build:

1. get a .z5 file and name it zcode.z5 (I used random.z5 from the frotz tests)
2. make
3. Create a virtualbox VM with fzos-floppy.bin in the floppy drive.
4. launch the VM

## TODO/bugs:

* first scroll discoloration
* extended characters (etude.z5/#7)
* reverse and bold text (etude.z5/4)
* real allocator for malloc/realloc/free
* add headers to files (simplest 'filesystem')
   * allow save games and transcripts, extractable with reverse engineering
* timeout on os_read_key
* beep
* load executable image above 0x100000
