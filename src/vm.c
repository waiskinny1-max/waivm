#include "wai/vm.h"
#include <stddef.h>
#include <string.h>

_Static_assert(sizeof(wai_instruction) == 12u, "wai_instruction must be exactly 12 bytes");
_Static_assert(offsetof(wai_vm, regs) == 0u, "wai_vm.regs ABI offset mismatch");
_Static_assert(offsetof(wai_vm, ip) == 64u, "wai_vm.ip ABI offset mismatch");
_Static_assert(offsetof(wai_vm, zf) == 72u, "wai_vm.zf ABI offset mismatch");
_Static_assert(offsetof(wai_vm, halted) == 73u, "wai_vm.halted ABI offset mismatch");
_Static_assert(offsetof(wai_vm, code) == 80u, "wai_vm.code ABI offset mismatch");
_Static_assert(offsetof(wai_vm, code_count) == 88u, "wai_vm.code_count ABI offset mismatch");
_Static_assert(offsetof(wai_vm, error) == 96u, "wai_vm.error ABI offset mismatch");

void wai_vm_init(wai_vm *vm, const wai_instruction *code, uint64_t code_count) {
    memset(vm, 0, sizeof(*vm));
    vm->code = code;
    vm->code_count = code_count;
    vm->error = WAI_OK;
    vm->print_stream = stdout;
}

void wai_vm_set_print_stream(wai_vm *vm, FILE *stream) {
    vm->print_stream = stream;
}

void wai_vm_emit_print(wai_vm *vm, wai_value value) {
    vm->last_print = value;
    vm->print_count += 1u;
    if (vm->print_stream != NULL) {
        (void)fprintf(vm->print_stream, "%lld\n", (long long)value);
    }
}

wai_error_code wai_vm_execute(wai_vm *vm) {
    int rc = wai_vm_exec_asm(vm);
    if (rc != 0) {
        return vm->error;
    }
    return WAI_OK;
}
