; Reserved for later direct Linux sys_write print support.
; v0.1 prints through the C hook wai_vm_emit_print so tests can observe output.

BITS 64
section .text
global wai_print_linux_x86_64_reserved

wai_print_linux_x86_64_reserved:
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
