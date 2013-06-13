# Barebones Frotz platform

a return to the golden era of bootable games

This builds a hard disk image that can be booted in a VM to run a .z5 program.

## Features

* standalone image--only an x86 hypervisor required to play
* instant boot
* no closed-source software except for game itself
* original .z5 file easily identifiable and extractable

## To build an image:

1. get a .z5 file (I used random.z5 from the frotz tests)
2. make random.img

## To use the resulting image

3. truncate --size=1MB savedisk.img
4. qemu-system-i386 random.img -hdb savedisk.img

## TODO/bugs:

* make second hard disk not required to play (only to save)
* os_read_line positioning bug after timeout/continuation
* extended characters (etude.z5/7)
* beep/sound

## Gotchas

* raw image doesn't work in VMWare/VirtualBox; convert to vmdk first
