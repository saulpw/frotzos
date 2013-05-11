
FROTZDIR=frotz-2.43d
FROTZLIB=frotz_common.a

BINS=fzos-floppy.img bootloader.bin kernel.bin $(FROTZLIB)

CFLAGS += -Wall -Wextra -Werror -Wno-pointer-sign -Wno-unused \
		  -nostdlib -nostdinc -nostartfiles -nodefaultlibs \
		  -I$(FROTZDIR)/src/common -I.


# zcode.z5 is the z-code file to be interpreted
OBJS := $(patsubst %.c,%.o,$(wildcard fzos_*.c)) zcode.o

all: fzos-floppy.img

$(FROTZLIB):
	CFLAGS="-march=i386 -m32" make -C $(FROTZDIR) src/$(FROTZLIB)
	cp $(FROTZDIR)/src/$(FROTZLIB) .

kernel.bin: $(FROTZLIB) kmain.o $(OBJS) linker.ld
	ld -Map=$@.map -m elf_i386 -T linker.ld -o $@ $(OBJS) $(FROTZLIB)

%.o: %.z5
	objcopy -B i386 --input-target=binary --output-target=elf32-i386 $< $@

.c.o:
	gcc -ffreestanding -m32 -O2 $(CFLAGS) -c -o $@ $<

bootloader.bin: bootloader.asm
	nasm -f bin -l $@.list -o $@ $<

fzos-floppy.img: bootloader.bin kernel.bin
	cat $^ > $@
	truncate $@ --size=%1K

clean:
	make -C $(FROTZDIR) clean
	rm -f $(BINS) $(OBJS) kmain.o bootloader.bin.list kernel.bin.map
