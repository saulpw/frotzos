# Barebones Frotz platform

a return to the golden era of bootable games

This builds a floppy image that can be booted in a VM to run a .z5 program.

## Features

* standalone image--only an x86 hypervisor required to play
* instant boot
* no closed-source software except for game itself
* original .z5 file easily identifiable and extractable

## To build an image:

1. get a .z5 file (I used random.z5 from the frotz tests)
2. make random.vmdk

## To use the resulting image

3. qemu-system-i386 random.vmdk [-s]

## TODO/bugs:

* os_read_line positioning bug after timeout/continuation
* extended characters (etude.z5/7)
* allow save games and transcripts, extractable with reverse engineering
* load executable image above 0x100000
* beep/sound

## Gotchas

* must be used as a hard drive image (not floppy) due to int 13h/48h
* doesn't work as a hard disk on vmware/virtualbox due to bad mbr?
