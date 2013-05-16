# Barebones Frotz platform

a return to the golden era of bootable games

This builds a floppy image that can be booted in a VM to run a .z5 program.

## Features

* standalone image--only an x86 hypervisor required to play
* instant boot
* no closed-source software except for game itself
* original .z5 file easily identifiable and extractable

## To build:

1. get a .z5 file and name it zcode.z5 (I used random.z5 from the frotz tests)
2. make
3. Create a virtualbox VM with fzos-floppy.bin in the floppy drive.
4. launch the VM

## TODO/bugs:

* os_read_line positioning bug after timeout/continuation
* extended characters (etude.z5/7)
* allow save games and transcripts, extractable with reverse engineering
* load executable image above 0x100000
* beep/sound

## Gotchas

* when disk size exactly 360k, disk geometry changes; truncate --size=361k to fix

