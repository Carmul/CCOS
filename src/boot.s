MBOOT_PAGE_ALIGN EQU 1 << 0
MBOOT_MEM_INFO EQU 1 << 1
MBOOT_USE_GFX EQU 0

MBOOT_MAGIC EQU 0x1BADB002
MBOOT_FLAGS EQU MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_USE_GFX
MBOOT_CHECKSUM EQU -(MBOOT_MAGIC + MBOOT_FLAGS)

section .multiboot
ALIGN 4
    DD MBOOT_MAGIC
    DD MBOOT_FLAGS
    DD MBOOT_CHECKSUM
    DD 0, 0, 0, 0, 0

    DD 0
    DD 800
    DD 600
    DD 32


section .bss

ALIGN 16
stack_bottom:
    RESB 16384 * 8
stack_top:



section .text:

global _start
extern kmain
_start:
    cli
    mov esp, stack_top
    push ebx           ; push multiboot information
    push eax           ; push magic value
    call kmain
    hlt                ; Halt if kmain returns (shouldn’t)



