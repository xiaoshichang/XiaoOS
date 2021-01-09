[org 0x7c00] ; bootloader offset
KERNEL_OFFSET equ 0x1000    ; this is the meory offset to which we will load our kernel

    mov [BOOT_DRIVE], dl

    mov bp, 0x9000 ; set the stack
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call print ; This will be written after the BIOS messages

    call load_kernel    ; Load our kernel
    call switch_to_pm
    jmp $ ; this will actually never be executed

%include "src/boot/boot_print.asm"
%include "src/boot/gdt32.asm"
%include "src/boot/print_pm.asm"
%include "src/boot/switch_pm.asm"
%include "src/boot/disk_load.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print

    mov bx, KERNEL_OFFSET
    mov dh, 15
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

[bits 32]
BEGIN_PM: ; after the switch we will get here
    mov ebx, MSG_PROT_MODE
    call print_string_pm ; Note that this will be written at the top left corner

    call KERNEL_OFFSET
    jmp $

MSG_REAL_MODE db "Started in 16-bit real mode", 0
MSG_PROT_MODE db "Loaded 32-bit protected mode", 0

BOOT_DRIVE db 0
MSG_LOAD_KERNEL db "Loading kernel into memory", 0


; bootsector
times 510-($-$$) db 0
dw 0xaa55