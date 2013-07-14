PUBL=PUBLISHER_ID
PREP=PREPARER_ID
SYSI=SYSTEM_ID
VOLI=VOLUME_ID
VOLS=VOLUMESET_ID
ABST=ABSTRACT_FILE
APPI=APPLICATION_ID
COPY=COPYRIGHT_FILE
BIBL=BIBLIOGRAPHIC_FILE

FROTZDIR=frotz
FROTZLIB=frotz_common.a

BINS=bootloader.bin frotz.bin kernel.bin isoboot.bin $(FROTZLIB) tools/iso2zip

ARCHFLAGS= -ffreestanding -m32 -nostdlib -nostdinc -nostartfiles -nodefaultlibs -fno-strict-aliasing
INCLUDES= -I$(FROTZDIR)/src/common -I.
WARNFLAGS=-Wall -Wextra -Werror -Wno-pointer-sign -Wno-unused
CFLAGS += -ggdb $(ARCHFLAGS) $(INCLUDES) $(WARNFLAGS)

ifdef DEBUG
	CFLAGS += -DDEBUG=$(DEBUG)
	ASMFLAGS += -DDEBUG=$(DEBUG)
else
	CFLAGS += -O2
endif

MALLOC_CFLAGS= -O3 -DLACKS_UNISTD_H -DLACKS_FCNTL_H -DLACKS_SYS_PARAM_H  \
-DLACKS_SYS_MMAN_H -DLACKS_STRINGS_H -DLACKS_ERRNO_H -DLACKS_SYS_TYPES_H \
-DLACKS_SCHED_H -DLACKS_TIME_H -Dmalloc_getpagesize=4096 -DHAVE_MMAP=0   \
-DMALLOC_FAILURE_ACTION='abort()' -DENOMEM=12 -DEINVAL=22

# hardware layer for frotz to port to the single purpose frotz os
FROTZ_OBJS := $(addprefix frotz/src/frotzos/, \
		fzos_init.o       \
		fzos_input.o      \
		fzos_display.o    \
		fzos_file.o     ) \
		\
		dev/vgatext.o     \
		stdio.o           \
		malloc.o          \
		string.o          \
		strdup.o          \
		syscalls.o        \
		syscall_impl.o     \
		\
		sbrk.o            \
		stdlib.o          \
		iso9660.o         \
		kprintf.o         \
		dev/serial.o      \


KERNEL_OBJS := \
		dev/ata.o         \
		dev/serial.o      \
		dev/time.o        \
		hdd.o             \
		int_stage0.o      \
		interrupts.o      \
		kernel.o          \
		kprintf.o         \
		ksyscalls.o       \
		syscall_impl.o    \
		kvirtmem.o        \
		string.o          \
		\
		iso9660.o         \
		stdlib.o          \
		dev/kb.o          \

# if you have LostPig.z8, 'make LostPig.iso'

ZCODE_FILES = $(wildcard zcode/*.z*)

IMAGE_FILES += $(patsubst %.z5,%.iso,$(ZCODE_FILES))
IMAGE_FILES += $(patsubst %.z8,%.iso,$(ZCODE_FILES))

all: frotz.bin frotz.elf kernel.bin LostPig.iso.zip
# $(IMAGE_FILES)

$(FROTZLIB):
	CFLAGS="-nostdinc -I.. -ggdb -march=i386 -m32" make -C $(FROTZDIR) src/$(FROTZLIB)
	cp $(FROTZDIR)/src/$(FROTZLIB) .

frotz.elf: $(FROTZLIB) appmain.o $(FROTZ_OBJS) linker.ld
	ld -m elf_i386 -T linker.ld -o $@ $(FROTZ_OBJS) $(FROTZLIB)

kernel.elf: $(KERNEL_OBJS) kmain.o kernel.ld
	ld -m elf_i386 -T kernel.ld -o $@ $(KERNEL_OBJS)

malloc.o: malloc.c
	gcc -c $(CFLAGS) $(MALLOC_CFLAGS) -o $@ $<


%.bin: %.asm bootcommon.asm
	nasm $(ASMFLAGS) -f bin -l $@.lst -o $@ $<

bootkernel.bin: bootloader.bin kernel.bin
	cat bootloader.bin kernel.bin > bootkernel.bin

%.o: %.asm
	nasm $(ASMFLAGS) -f elf -o $@ $<

%.zcode: %.z8
	cp -f $< $@

%.zcode: %.z5
	cp -f $< $@

LostPig.iso: LostPig.z8 bootkernel.bin isoboot.bin frotz.bin
	mkisofs \
		-r \
		-iso-level 1 \
		-no-pad \
		-biblio    "$(BIBL)" \
		-copyright "$(COPY)" \
		-A         "$(APPI)" \
		-abstract  "$(ABST)" \
		-p         "$(PREP)" \
		-publisher "$(PUBL)" \
		-sysid     "$(SYSI)" \
		-V         "$(VOLI)" \
		-volset    "$(VOLS)" \
		-G bootkernel.bin \
		-b isoboot.bin \
		-c boot.cat \
		-no-emul-boot \
		-boot-load-seg=0x7c0 \
		-boot-load-size=1 \
		-boot-info-table \
		-sort fzos.sort \
		-input-charset=iso8859-1 \
		-o $@ isoboot.bin frotz.bin $<

%.lst: %.elf
	objdump -d $< > $@

%.bin: %.elf
	objcopy -O binary $< $@

.c.o:
	gcc -c $(CFLAGS) -o $@ $<

tools/iso2zip: tools/iso2zip.c
	gcc -o $@ $<

%.izo: %.iso tools/iso2zip
	./tools/iso2zip $< -o $@


clean:
	make -C $(FROTZDIR) clean
	rm -f $(BINS) $(KERNEL_OBJS) $(FROTZ_OBJS) kmain.o appmain.o *.map kernel.elf frotz.elf *.lst
