# Barebones Frotz platform

a return to the golden era of bootable games

This builds a floppy image that can be booted in a VM to run a .z5 program

## To build:

1) untar frotz tgz from http://sourceforge.net/projects/frotz/files/frotz/2.43/
  a) edit Makefile, add "OPTS += -nostdinc -I.."
2) get a .z5 file and name it zcode.z5 (I used random.z5 from the frotz tests)
3) make
4) Create a virtualbox VM with fzos-floppy.bin in the floppy drive.
5) launch the VM

## TODO:

) add the NOTIMPLs
) create keyboard handler

