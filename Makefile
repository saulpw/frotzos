
FROTZDIR = frotz-2.43d
FROTZLIB=frotz_common.a

BINS=boot.img bootloader.bin kernel.bin bootloader.bin.list $(FROTZLIB)

CFLAGS += -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs \
		  -Wno-pointer-sign -Wno-unused \
		  -I$(FROTZDIR)/src/common -I.


OBJS := $(patsubst %.c,%.o,$(wildcard fzos_*.c)) random.o

all: boot.img

$(FROTZLIB):
	make -C $(FROTZDIR) src/$(FROTZLIB)
	cp $(FROTZDIR)/src/$(FROTZLIB) .

kernel.bin: $(FROTZLIB) kmain.o $(OBJS) linker.ld
	ld -Map=$@.map -m elf_i386 -T linker.ld -o $@ $(OBJS) $(FROTZLIB)

%.o: %.z5
	objcopy -B i386 --input-target=binary --output-target=elf32-i386 $< $@

.c.o:
	gcc -ffreestanding -m32 -O2 $(CFLAGS) -c -o $@ $<

bootloader.bin: bootloader.asm
	nasm -f bin -l $@.list -o $@ $<

boot.img: bootloader.bin kernel.bin
	cat $^ > $@
	truncate $@ --size=%1K

clean:
	rm -f $(BINS) $(OBJS) kmain.o
