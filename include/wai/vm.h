#ifndef WAI_VM_H
#define WAI_VM_H

#include <stdint.h>
#include <stdio.h>
#include "wai/instruction.h"
#include "wai/error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WAI_REGISTER_COUNT 8u

typedef struct wai_vm {
    wai_value regs[WAI_REGISTER_COUNT];
    uint64_t ip;
    uint8_t zf;
    uint8_t halted;
    uint8_t reserved[6];
    const wai_instruction *code;
    uint64_t code_count;
    wai_error_code error;
    FILE *print_stream;
    wai_value last_print;
    uint64_t print_count;
} wai_vm;

void wai_vm_init(wai_vm *vm, const wai_instruction *code, uint64_t code_count);
void wai_vm_set_print_stream(wai_vm *vm, FILE *stream);
wai_error_code wai_vm_execute(wai_vm *vm);
wai_error_code wai_vm_step(wai_vm *vm);

/* Implemented in asm/vm_linux_x86_64.asm. */
int wai_vm_exec_asm(wai_vm *vm);

/* Called from the assembly runtime and the C stepping runtime. */
void wai_vm_emit_print(wai_vm *vm, wai_value value);

#ifdef __cplusplus
}
#endif

#endif
