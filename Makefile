GCC = gcc
LD = ld
NASM = nasm
GRUB_MKRESCUE = grub-mkrescue
QEMU = qemu-system-i386

ISO_FILE = YOS.iso
KERNEL_ENTRY = src/kernel/entry.asm
GRUB_CFG = iso/boot/grub/grub.cfg
LINKER_SCRIPT = kernel.ld

KERNEL_OBJS = kernel.o io.o entry.o idt.o timer.o scheduler.o debug.o trampoline.o mouse.o heap.o pmm.o paging.o

all: $(ISO_FILE)

$(GRUB_CFG):
	@mkdir -p $(@D)
	@echo "set timeout=0" > $@
	@echo "set default=0" >> $@
	@echo "" >> $@
	@echo "menuentry \"My OS\" {" >> $@
	@echo "    multiboot /boot/kernel.bin" >> $@
	@echo "    boot" >> $@
	@echo "}" >> $@

kernel.bin: $(KERNEL_OBJS) $(LINKER_SCRIPT)
	$(LD) -m elf_i386 -T $(LINKER_SCRIPT) --nmagic -o $@ $(KERNEL_OBJS)

kernel.o: src/kernel/kernel.c src/kernel/io.h src/kernel/idt.h src/kernel/timer.h src/kernel/scheduler.h
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector

io.o: src/kernel/io.c src/kernel/io.h
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector

entry.o: $(KERNEL_ENTRY)
	$(NASM) -f elf -o $@ $<

idt.o: src/kernel/idt.c src/kernel/idt.h
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector

timer.o: src/kernel/timer.c src/kernel/timer.h src/kernel/idt.h
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector

scheduler.o: src/kernel/scheduler.c src/kernel/scheduler.h src/kernel/idt.h
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector

debug.o: src/kernel/debug.c src/kernel/io.h src/kernel/idt.h
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector

trampoline.o: src/kernel/trampoline.c src/kernel/io.h
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector
mouse.o: src/kernel/mouse.c src/kernel/io.h
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector
heap.o: src/kernel/heap.c
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector
pmm.o: src/kernel/pmm.c
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector
paging.o: src/kernel/paging.c src/kernel/pmm.h src/kernel/io.h
	$(GCC) -m32 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -c $< -o $@ -fno-stack-protector



$(ISO_FILE): kernel.bin $(GRUB_CFG)
	cp $< iso/boot/kernel.bin
	$(GRUB_MKRESCUE) -o $@ iso

run: $(ISO_FILE)
	$(QEMU) -cdrom $< -display gtk

clean:
	rm -f *.o *.bin $(ISO_FILE)
	rm -rf iso/boot/
