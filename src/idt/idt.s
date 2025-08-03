global idt_flush
global isr0
global irq0

section .text

; Load IDT pointer from argument (on stack)
idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    sti
    ret

%macro isr_no_err_stub 1
    global isr%1
    isr%1:
        cli
        push dword 0   ; no error code
        push dword %1  ; interrupt number
        jmp isr_common_stub
%endmacro

%macro isr_err_stub 1
    global isr%1
    isr%1:
        cli
        push dword %1
        jmp isr_common_stub
%endmacro

%macro irq_stub 2
    global irq%1
    irq%1:
        cli
        push dword 0
        push dword %2
        jmp irq_common_stub
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7

isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

isr_no_err_stub 128
isr_no_err_stub 177


irq_stub   0,    32
irq_stub   1,    33
irq_stub   2,    34
irq_stub   3,    35
irq_stub   4,    36
irq_stub   5,    37
irq_stub   6,    38
irq_stub   7,    39
irq_stub   8,    40
irq_stub   9,    41
irq_stub  10,    42
irq_stub  11,    43
irq_stub  12,    44
irq_stub  13,    45
irq_stub  14,    46
irq_stub  15,    47

; Common handlers
extern isr_handler
extern irq_handler

isr_common_stub:
    pusha
    mov eax, [esp + 32]   ; interrupt number is at ESP + 32 (because pusha pushed 8 regs = 32 bytes)
    mov ebx, [esp + 36]   ; error code is above that
    push ebx               ; push error_code as 2nd param
    push eax               ; push int_no as 1st param
    call isr_handler
    add esp, 8             ; remove params
    popa
    add esp, 8
    sti
    iret

extern irq_handler
irq_common_stub:
    pusha
    mov eax, [esp + 32]   ; interrupt number is at ESP + 32 (because pusha pushed 8 regs = 32 bytes)
    mov ebx, [esp + 36]   ; error code is above that
    push ebx               ; push error_code as 2nd param
    push eax               ; push int_no as 1st param
    call irq_handler
    add esp, 8             ; remove params
    popa
    add esp, 8
    sti
    iret
