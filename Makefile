# === Paths ===
SRC_DIR     = src
BUILD_DIR   = build
ISO_DIR     = CCOS

# === Tools ===
CC          = /home/carmel/opt/cross/bin/i686-elf-gcc
LD          = /home/carmel/opt/cross/bin/i686-elf-ld
AS          = /usr/bin/nasm

# === Flags ===
CFLAGS      = -ffreestanding -Wall -Wextra -g -O2
LDFLAGS     = -m elf_i386 -T linker.ld
ASFLAGS     = -f elf32

# === Files ===
KERNEL_BIN  = $(ISO_DIR)/boot/kernel.bin
ISO         = CCOS.iso

# === Object files ===
C_OBJS = \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/gdt.o \
	$(BUILD_DIR)/idt.o \
	$(BUILD_DIR)/vga.o \
	$(BUILD_DIR)/memutil.o \
	$(BUILD_DIR)/string.o \
	$(BUILD_DIR)/timer.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/stdio.o \
	$(BUILD_DIR)/memory.o

ASM_OBJS = \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/gdt.s.o \
	$(BUILD_DIR)/idt.s.o

OBJS = $(C_OBJS) $(ASM_OBJS)

.PHONY: all clean run r

# === Targets ===
all: $(ISO)

$(ISO): $(KERNEL_BIN)
	grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL_BIN): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# === Compile rules ===
$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/gdt.o: $(SRC_DIR)/gdt/gdt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/gdt.s.o: $(SRC_DIR)/gdt/gdt.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/idt.o: $(SRC_DIR)/idt/idt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/idt.s.o: $(SRC_DIR)/idt/idt.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/vga.o: $(SRC_DIR)/vga/vga.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memutil.o: $(SRC_DIR)/stdlib/memutil/memutil.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/string.o: $(SRC_DIR)/stdlib/string/string.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/timer.o: $(SRC_DIR)/timer/timer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: $(SRC_DIR)/keyboard/keyboard.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stdio.o: $(SRC_DIR)/stdlib/stdio/stdio.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o: $(SRC_DIR)/memory/memory.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(ISO)
	qemu-system-i386 -cdrom $< -monitor stdio

clean:
	rm -rf $(BUILD_DIR) $(ISO) $(KERNEL_BIN)

r: clean all run