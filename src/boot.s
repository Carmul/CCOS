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


section .boot

global _start
_start:
    cli
    mov ecx, (initial_page_dir - 0xC0000000)
    mov cr3, ecx

    mov ecx, cr4
    or ecx, 0x10
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    jmp higher_half


section .text

higher_half:
    mov esp, stack_top
    push ebx      ; push multiboot information
    push eax      ; push magic value
    xor ebp, ebp
    extern kmain
    call kmain
    hlt           ; Halt if kmain returns (shouldn’t)
                  

section .data

align 4096
global initial_page_dir
initial_page_dir:
    DD 10000011b
    TIMES 768-1 DD 0

    DD (0 << 22) | 10000011b
    DD (1 << 22) | 10000011b
    DD (2 << 22) | 10000011b
    DD (3 << 22) | 10000011b
    TIMES 256-4 DD 0
