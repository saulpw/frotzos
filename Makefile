
FROTZDIR=frotz
FROTZLIB=frotz_common.a

BINS=fzos-floppy.img bootloader.bin kernel.bin $(FROTZLIB)

ARCHFLAGS= -ffreestanding -m32 -nostdlib -nostdinc -nostartfiles -nodefaultlibs
INCLUDES= -I$(FROTZDIR)/src/common -I.
WARNFLAGS=-Wall -Wextra -Werror -Wno-pointer-sign -Wno-unused
CFLAGS += -ggdb -O2 $(ARCHFLAGS) $(INCLUDES) $(WARNFLAGS)

# zcode.z5 is the z-code file to be interpreted
OBJS := $(patsubst %.c,%.o,$(wildcard fzos_*.c)) zcode.o

all: fzos-floppy.img kernel.elf

$(FROTZLIB):
	CFLAGS="-nostdinc -I.. -ggdb -march=i386 -m32" make -C $(FROTZDIR) src/$(FROTZLIB)
	cp $(FROTZDIR)/src/$(FROTZLIB) .

kernel.bin: $(FROTZLIB) kmain.o $(OBJS) linker.ld
	ld -Map=$@.map -m elf_i386 --oformat binary -T linker.ld -o $@ $(OBJS) $(FROTZLIB)

kernel.elf: $(FROTZLIB) kmain.o $(OBJS) linker.ld
	ld -m elf_i386 -T linker.ld -o $@ $(OBJS) $(FROTZLIB)

%.o: %.z5
	objcopy -B i386 --input-target=binary --output-target=elf32-i386 $< $@

.c.o:
	gcc $(CFLAGS) -c -o $@ $<

bootloader.bin: bootloader.asm
	nasm -f bin -l $@.list -o $@ $<

fzos-floppy.img: bootloader.bin kernel.bin
	cat $^ > $@
	truncate $@ --size=%1K

clean:
	make -C $(FROTZDIR) clean
	rm -f $(BINS) $(OBJS) kmain.o bootloader.bin.list *.map kernel.elf
