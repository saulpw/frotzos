
FROTZDIR=frotz
FROTZLIB=frotz_common.a

BINS=mkfzimg bootloader.bin frotz.bin $(FROTZLIB)

ARCHFLAGS= -ffreestanding -m32 -nostdlib -nostdinc -nostartfiles -nodefaultlibs
INCLUDES= -I$(FROTZDIR)/src/common -I.
WARNFLAGS=-Wall -Wextra -Werror -Wno-pointer-sign -Wno-unused
CFLAGS += -ggdb -O2 $(ARCHFLAGS) $(INCLUDES) $(WARNFLAGS)

MALLOC_CFLAGS= -O3 -DLACKS_UNISTD_H -DLACKS_FCNTL_H -DLACKS_SYS_PARAM_H  \
-DLACKS_SYS_MMAN_H -DLACKS_STRINGS_H -DLACKS_ERRNO_H -DLACKS_SYS_TYPES_H \
-DLACKS_SCHED_H -DLACKS_TIME_H -Dmalloc_getpagesize=4096 -DHAVE_MMAP=0   \
-DMALLOC_FAILURE_ACTION='abort()' -DENOMEM=12 -DEINVAL=22

FZ_OBJS := fzos_display.o    \
		fzos_file.o       \
		fzos_hw.o         \
		fzos_init.o       \
		fzos_input.o      \
		fzos_mem.o        \
		fzos_readline.o   \
		fzos_string.o     \
		debug.o           \
		malloc.o

# if you have LostPig.z8, 'make LostPig.img'
all: frotz.bin frotz.elf

$(FROTZLIB):
	CFLAGS="-nostdinc -I.. -ggdb -march=i386 -m32" make -C $(FROTZDIR) src/$(FROTZLIB)
	cp $(FROTZDIR)/src/$(FROTZLIB) .

frotz.bin: $(FROTZLIB) kmain.o $(FZ_OBJS) linker.ld
	ld -Map=$@.map -m elf_i386 --oformat binary -T linker.ld -o $@ $(FZ_OBJS) $(FROTZLIB)

frotz.elf: $(FROTZLIB) kmain.o $(FZ_OBJS) linker.ld
	ld -m elf_i386 -T linker.ld -o $@ $(FZ_OBJS) $(FROTZLIB)

mkfzimg: mkfzimg.c
	gcc -ggdb -o $@ $<

.c.o:
	gcc -c $(CFLAGS) -o $@ $<

malloc.o: malloc.c
	gcc -c $(CFLAGS) $(MALLOC_CFLAGS) -o $@ $<

bootloader.bin: bootloader.asm
	nasm -f bin -l $@.list -o $@ $<

%.simplefs:	%.z* mkfzimg frotz.bin
	./mkfzimg -o $@ frotz.bin $<

%.img: %.simplefs bootloader.bin frotz.elf
	cat bootloader.bin $< > $@
	truncate $@ --size=%1K

clean:
	make -C $(FROTZDIR) clean
	rm -f $(BINS) $(FZ_OBJS) kmain.o bootloader.bin.list *.map frotz.elf
