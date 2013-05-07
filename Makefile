
all: boot.img

bootsector: bootloader.asm
	nasm $< -o $@ -l $@.list

# frotz-2.43d/frotz.bin
#	cat boot frotz-2.43d/frotz.bin > $@
boot.img: bootsector
	cat bootsector > $@
	truncate --size=%1K $@
	rm bootsector

clean:
	rm -f boot.img bootsector bootsector.list
