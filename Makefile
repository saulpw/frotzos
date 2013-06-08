
FROTZDIR=frotz
FROTZLIB=frotz_common.a

BINS=mkelifs bootloader.bin frotz.bin $(FROTZLIB)

ARCHFLAGS= -ffreestanding -m32 -nostdlib -nostdinc -nostartfiles -nodefaultlibs
INCLUDES= -I$(FROTZDIR)/src/common -I.
WARNFLAGS=-Wall -Wextra -Werror -Wno-pointer-sign -Wno-unused
CFLAGS += -ggdb $(ARCHFLAGS) $(INCLUDES) $(WARNFLAGS)

ifdef DEBUG
	CFLAGS += -DDEBUG=kprintf
	ASMFLAGS += -DDEBUG=1
else
	CFLAGS += -O2
endif

MALLOC_CFLAGS= -O3 -DLACKS_UNISTD_H -DLACKS_FCNTL_H -DLACKS_SYS_PARAM_H  \
-DLACKS_SYS_MMAN_H -DLACKS_STRINGS_H -DLACKS_ERRNO_H -DLACKS_SYS_TYPES_H \
-DLACKS_SCHED_H -DLACKS_TIME_H -Dmalloc_getpagesize=4096 -DHAVE_MMAP=0   \
-DMALLOC_FAILURE_ACTION='abort()' -DENOMEM=12 -DEINVAL=22

FROTZ_OBJS := $(addprefix frotz/src/frotzos/, \
		fzos_init.o       \
		fzos_input.o      \
		fzos_display.o    \
		fzos_file.o       \
)

FZ_OBJS := \
		dev/ata.o         \
		dev/kb.o          \
		dev/serial.o      \
		dev/time.o        \
		dev/vgatext.o     \
		int_stage0.o      \
		interrupts.o      \
		kernel.o          \
		kprintf.o         \
		kvirtmem.o        \
		elifs.o           \
		stdio.o           \
		malloc.o          \
		string.o          \
		$(FROTZ_OBJS)     \

# if you have LostPig.z8, 'make LostPig.img'

ZCODE_FILES = $(wildcard zcode/*.z*)

IMAGE_FILES += $(patsubst %.z5,%.img,$(ZCODE_FILES))
IMAGE_FILES += $(patsubst %.z8,%.img,$(ZCODE_FILES))

all: frotz.bin frotz.elf $(IMAGE_FILES)

$(FROTZLIB):
	CFLAGS="-nostdinc -I.. -ggdb -march=i386 -m32" make -C $(FROTZDIR) src/$(FROTZLIB)
	cp $(FROTZDIR)/src/$(FROTZLIB) .

frotz.bin: $(FROTZLIB) kmain.o $(FZ_OBJS) linker.ld
	ld -Map=$@.map -m elf_i386 --oformat binary -T linker.ld -o $@ $(FZ_OBJS) $(FROTZLIB)

frotz.elf: $(FROTZLIB) kmain.o $(FZ_OBJS) linker.ld
	ld -m elf_i386 -T linker.ld -o $@ $(FZ_OBJS) $(FROTZLIB)

mkelifs: mkelifs.c
	gcc -ggdb -o $@ $<

xelifs: xelifs.c
	gcc -ggdb -o $@ $<

.c.o:
	gcc -c $(CFLAGS) -o $@ $<

malloc.o: malloc.c
	gcc -c $(CFLAGS) $(MALLOC_CFLAGS) -o $@ $<

bootloader.bin: bootloader.asm
	nasm $(ASMFLAGS) -f bin -l $@.list -o $@ $<

%.o: %.asm
	nasm $(ASMFLAGS) -f elf -o $@ $<

%.simplefs:	%.z5 mkelifs frotz.bin
	./mkelifs -o $@ frotz.bin $<

%.simplefs:	%.z8 mkelifs frotz.bin
	./mkelifs -o $@ frotz.bin $<

%.img: %.simplefs bootloader.bin frotz.elf
	cat bootloader.bin $< > $@

clean:
	make -C $(FROTZDIR) clean
	rm -f $(BINS) $(FZ_OBJS) kmain.o bootloader.bin.list *.map frotz.elf
