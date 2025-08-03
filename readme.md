# CCOS

Simple hobby operating system  project for the x86 architecture.  
The goal is to learn low-level system programming and how operating systems work.

---

## 🧠 Features (Work In Progress)

- Bootable via GRUB (Multiboot-compliant) ✅
- Text-mode VGA driver ✅
- Global Descriptor Table (GDT) ✅
- interrupt handling (IDT, IRQs) ✅
- Paging with Higher Half Kernel
- Memory allocator
- file system
- Simple kernel shell

---

## 🛠️ Build Requirements

- GCC cross-compiler for i686: `i686-elf-gcc`
- Binutils: `i686-elf-ld`
- NASM (for assembly files)
- QEMU (for emulation)
- Make
- GRUB tools: `grub-mkrescue` (for creating bootable ISO images)

---

## 🏗️ Build and Run

```bash
# Build the project
make all

# Emulate with QEMU
qemu-system-i386 CCOS.iso
