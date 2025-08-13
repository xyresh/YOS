; src/kernel/entry.asm

; Define the Multiboot header magic number and flags
MB_MAGIC equ 0x1BADB002
MB_FLAGS equ 0x0

; Tell the assembler that kernel_main and other functions are defined in another file
extern kernel_main
extern isr_handler
extern current_task

; GDT constants
GDT_CODE_SEGMENT equ 0x08
GDT_DATA_SEGMENT equ 0x10

; The Multiboot header must be in the first 8KB of the kernel
section .multiboot
    dd MB_MAGIC
    dd MB_FLAGS
    dd -(MB_MAGIC + MB_FLAGS)

; --- GDT Descriptors and Pointer ---
section .rodata
gdt_start:
    dd 0x0
    dd 0x0
    
    ; 32-bit Code Segment
    dw 0xffff       ; Limit (low)
    dw 0x0          ; Base (low)
    db 0x0          ; Base (middle)
    db 10011010b    ; Access byte
    db 11001111b    ; Flags + Limit (high)
    db 0x0          ; Base (high)
gdt_code:
    
    ; 32-bit Data Segment
    dw 0xffff       ; Limit (low)
    dw 0x0          ; Base (low)
    db 0x0          ; Base (middle)
    db 10010010b    ; Access byte
    db 11001111b    ; Flags + Limit (high)
    db 0x0          ; Base (high)
gdt_data:
    
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
    
; The actual entry point of the kernel
global _start
_start:
    ; --- GDT Setup ---
    lgdt [gdt_descriptor]
    
    ; Far jump to reload CS with the new GDT entry
    jmp GDT_CODE_SEGMENT:flush_cs
flush_cs:
    ; Reload all segment registers
    mov ax, GDT_DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; --- End GDT Setup ---

    ; Set up the stack at a valid address
    extern _stack_end
    mov esp, _stack_end

    ; Call the C code
    call kernel_main

    ; If kernel_main returns, we've finished, so we just halt.
    cli
    hlt

; --- The core context switching function for subsequent switches ---
; This function is called from C.
; It saves the current ESP to old_task_esp and loads new_task_esp.
global context_switch_asm
context_switch_asm:
    pushad              ; Save all general-purpose registers
    
    mov ebx, [esp + 36] ; Get the 'old_task_esp' argument (at esp+36)
    mov [ebx], esp      ; Save the current stack pointer to the TCB's ESP field
    
    mov ebx, [esp + 40] ; Get the 'new_task_esp' argument (at esp+40)
    mov esp, [ebx]      ; Load the new stack pointer from the TCB's ESP field

    popad               ; Restore the new task's general-purpose registers
    
    ret                 ; Return to the new task's entry point

; --- The initial jump function for the very first task ---
; This is a special function that jumps into the first task without
; needing to save any old state. It will never return.
global jump_to_first_task
jump_to_first_task:
    ; Load the ESP for the first task from the current_task struct.
    mov eax, [current_task]
    mov esp, [eax]

    ; The stack frame was prepared to handle this popad/ret sequence.
    popad
    ret

; --- Generic ISR Macro (for all interrupts) ---
%macro isr_stub_generic 1
global isr%1
isr%1:
    %if %1 == 8 || %1 == 10 || %1 == 11 || %1 == 12 || %1 == 13 || %1 == 14 || %1 == 17
    %else
    push dword 0
    %endif
    
    push dword %1
    
    pushad
    
    push ds
    push es
    push fs
    push gs

    mov ax, GDT_DATA_SEGMENT
    mov ds, ax
    mov es, ax
    
    mov eax, esp
    push eax
    call isr_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popad
    
    add esp, 8
    
    iretd
%endmacro

; --- Define stubs for all 256 interrupts ---
%assign i 0
%rep 256
    isr_stub_generic i
%assign i i+1
%endrep

; --- Define the global isr_stubs array ---
section .data
    global isr_stubs
    isr_stubs:
    %assign i 0
    %rep 256
        dd isr%+i
    %assign i i+1
    %endrep