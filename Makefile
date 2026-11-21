CC = gcc
LD = ld
NASM = nasm

CFLAGS = -m32 -mpreferred-stack-boundary=2 -mno-sse -mno-sse2 -mno-mmx -mno-3dnow -ffreestanding -fno-pie -fno-stack-protector -fno-builtin -nostdlib -Wall -Wextra -Iinclude -O2
LDFLAGS = -m elf_i386

BUILD_DIR = build
TOOLS_DIR = tools
CFS_DIR = cfs_root

all: $(BUILD_DIR)/insaneos.img

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 1. Host tool: mkcfs
$(BUILD_DIR)/mkcfs: $(TOOLS_DIR)/mkcfs.c include/cfs.h | $(BUILD_DIR)
	$(CC) -Wall -Wextra $(TOOLS_DIR)/mkcfs.c -o $@

# 2. InBoot Stage 1 & Stage 2
$(BUILD_DIR)/inboot_stage1.bin: boot/inboot_stage1.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(BUILD_DIR)/inboot_stage2.bin: boot/inboot_stage2.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@
	# Pad to exactly 31 sectors (31 * 512 = 15872 bytes)
	truncate -s 15872 $@

# 3. Drivers
$(BUILD_DIR)/vga_drv.o: drivers/vga_drv.c include/driver.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(CFS_DIR)/drivers/vga.drv: $(BUILD_DIR)/vga_drv.o drivers/driver_linker.ld
	$(LD) $(LDFLAGS) -T drivers/driver_linker.ld $< -o $@

$(BUILD_DIR)/timer_drv.o: drivers/timer_drv.c include/driver.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(CFS_DIR)/drivers/timer.drv: $(BUILD_DIR)/timer_drv.o drivers/driver_linker.ld
	$(LD) $(LDFLAGS) -T drivers/driver_linker.ld $< -o $@

$(BUILD_DIR)/kbd_drv.o: drivers/kbd_drv.c include/driver.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(CFS_DIR)/drivers/kbd.drv: $(BUILD_DIR)/kbd_drv.o drivers/driver_linker.ld
	$(LD) $(LDFLAGS) -T drivers/driver_linker.ld $< -o $@

$(BUILD_DIR)/serial_drv.o: drivers/serial_drv.c include/driver.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(CFS_DIR)/drivers/serial.drv: $(BUILD_DIR)/serial_drv.o drivers/driver_linker.ld
	$(LD) $(LDFLAGS) -T drivers/driver_linker.ld $< -o $@

DRIVERS = $(CFS_DIR)/drivers/vga.drv \
          $(CFS_DIR)/drivers/timer.drv \
          $(CFS_DIR)/drivers/kbd.drv \
          $(CFS_DIR)/drivers/serial.drv

# 4. iSH Shell
$(BUILD_DIR)/ish_entry.o: ish/ish_entry.asm | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

$(BUILD_DIR)/ish_main.o: ish/ish_main.c include/insane_api.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(CFS_DIR)/bin/ish: $(BUILD_DIR)/ish_entry.o $(BUILD_DIR)/ish_main.o ish/linker.ld
	$(LD) $(LDFLAGS) -T ish/linker.ld $(BUILD_DIR)/ish_entry.o $(BUILD_DIR)/ish_main.o -o $@

# 5. Core File System (CFS) Image
$(BUILD_DIR)/cfs.img: $(BUILD_DIR)/mkcfs $(CFS_DIR)/bin/ish $(DRIVERS) \
                      $(CFS_DIR)/etc/os-release $(CFS_DIR)/etc/insane.conf \
                      $(CFS_DIR)/etc/hostname $(CFS_DIR)/etc/motd $(CFS_DIR)/lib/libcfs.lib
	$(BUILD_DIR)/mkcfs $@ \
		$(CFS_DIR)/bin/ish:2 \
		$(CFS_DIR)/drivers/vga.drv:3 \
		$(CFS_DIR)/drivers/timer.drv:3 \
		$(CFS_DIR)/drivers/kbd.drv:3 \
		$(CFS_DIR)/drivers/serial.drv:3 \
		$(CFS_DIR)/etc/os-release:4 \
		$(CFS_DIR)/etc/insane.conf:4 \
		$(CFS_DIR)/etc/hostname:4 \
		$(CFS_DIR)/etc/motd:4 \
		$(CFS_DIR)/lib/libcfs.lib:5
	# Ensure CFS image is 65536 bytes (128 sectors)
	truncate -s 65536 $@

# 6. Kernel
KERNEL_OBJS = $(BUILD_DIR)/kernel_entry.o \
              $(BUILD_DIR)/kmain.o \
              $(BUILD_DIR)/console.o \
              $(BUILD_DIR)/memory.o \
              $(BUILD_DIR)/cfs.o \
              $(BUILD_DIR)/driver.o \
              $(BUILD_DIR)/kbd.o

$(BUILD_DIR)/kernel_entry.o: kernel/entry.asm | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

$(BUILD_DIR)/%.o: kernel/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJS) kernel/linker.ld
	$(LD) $(LDFLAGS) -T kernel/linker.ld $(KERNEL_OBJS) -o $@
	# Pad kernel to 128 sectors (65536 bytes)
	truncate -s 65536 $@

# 7. Final Disk Image (insaneos.img)
# LBA 0 (offset 0): Stage 1 (1 sector = 512 bytes)
# LBA 1 (offset 512): Stage 2 (31 sectors = 15872 bytes)
# LBA 32 (offset 16384): Kernel (128 sectors = 65536 bytes)
# LBA 256 (offset 131072): CFS Image (128 sectors = 65536 bytes)
# Total size: 384 sectors = 196608 bytes, pad to 10 MiB for VM compatibility
$(BUILD_DIR)/insaneos.img: $(BUILD_DIR)/inboot_stage1.bin $(BUILD_DIR)/inboot_stage2.bin $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/cfs.img
	dd if=/dev/zero of=$@ bs=512 count=20480 status=none
	dd if=$(BUILD_DIR)/inboot_stage1.bin of=$@ bs=512 count=1 seek=0 conv=notrunc status=none
	dd if=$(BUILD_DIR)/inboot_stage2.bin of=$@ bs=512 count=31 seek=1 conv=notrunc status=none
	dd if=$(BUILD_DIR)/kernel.bin of=$@ bs=512 count=128 seek=32 conv=notrunc status=none
	dd if=$(BUILD_DIR)/cfs.img of=$@ bs=512 count=128 seek=256 conv=notrunc status=none
	@echo "=== InsaneOS disk image created successfully ==="

qemu: $(BUILD_DIR)/insaneos.img
	qemu-system-x86_64 -drive format=raw,file=$< -m 128M -serial stdio

qemu-headless: $(BUILD_DIR)/insaneos.img
	qemu-system-x86_64 -drive format=raw,file=$< -m 128M -display none -serial stdio

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(CFS_DIR)/bin/ish $(CFS_DIR)/drivers/*.drv

.PHONY: all clean qemu qemu-headless
