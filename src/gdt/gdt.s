global gdt_load
section .text

gdt_load:
    mov eax, [esp + 4]   ; pointer to gdt_ptr struct
    lgdt [eax]           ; load GDT

    ; Reload segment registers
    mov ax, 0x10         ; Data segment selector (index 2, ring 0)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:flush       ; Far jump to reload CS (index 1)
flush:
    ret
