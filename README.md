# Barebones Frotz platform

a return to the golden era of bootable games

This builds a .izo image that can be booted in a VM to run a .z5 program.

## Features

* standalone image--only an x86 hypervisor required to play
* instantly boots directly into game
* minimal extraneous code
* no closed-source software except for game itself
* original z-code file easily identifiable and extractable via .zip

## To use the resulting .izo

1. (optional) truncate --size=1MB savedisk.img
2. qemu-system-i386 -cdrom LostPig.izo [-hda savedisk.img]

## TODO/bugs:

* lost pig color glitch on first scroll
* os_read_line positioning bug after timeout/continuation
* extended characters (etude.z5/7)

