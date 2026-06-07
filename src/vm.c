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

static wai_error_code set_error(wai_vm *vm, wai_error_code error) {
    vm->error = error;
    return error;
}

static int valid_reg(uint8_t reg) {
    return reg < WAI_REGISTER_COUNT;
}

static wai_error_code check_jump(wai_vm *vm, wai_value target) {
    if (target < 0 || (uint64_t)target >= vm->code_count) {
        return set_error(vm, WAI_ERR_BAD_JUMP);
    }
    return WAI_OK;
}

static void update_zf(wai_vm *vm, wai_value value) {
    vm->zf = value == 0 ? 1u : 0u;
}

wai_error_code wai_vm_step(wai_vm *vm) {
    if (vm == NULL || vm->code == NULL) {
        return WAI_ERR_PARSE;
    }
    if (vm->halted != 0u) {
        return WAI_OK;
    }
    if (vm->ip >= vm->code_count) {
        return set_error(vm, WAI_ERR_IP_OUT_OF_BOUNDS);
    }

    const wai_instruction *ins = &vm->code[vm->ip];
    vm->ip += 1u;

    uint8_t a = ins->a;
    uint8_t b = ins->b;
    wai_value value = 0;

    switch ((wai_opcode)ins->opcode) {
        case WAI_OP_MOV_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] = ins->imm;
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_MOV_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] = vm->regs[b];
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_ADD_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] += ins->imm;
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_ADD_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] += vm->regs[b];
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_SUB_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] -= ins->imm;
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_SUB_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] -= vm->regs[b];
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_MUL_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] *= ins->imm;
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_MUL_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] *= vm->regs[b];
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_DIV_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (ins->imm == 0) { return set_error(vm, WAI_ERR_DIV_ZERO); }
            vm->regs[a] /= ins->imm;
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_DIV_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            value = vm->regs[b];
            if (value == 0) { return set_error(vm, WAI_ERR_DIV_ZERO); }
            vm->regs[a] /= value;
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_JMP:
            if (check_jump(vm, ins->imm) != WAI_OK) { return vm->error; }
            vm->ip = (uint64_t)ins->imm;
            return WAI_OK;
        case WAI_OP_JZ:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (vm->regs[a] == 0) {
                if (check_jump(vm, ins->imm) != WAI_OK) { return vm->error; }
                vm->ip = (uint64_t)ins->imm;
            }
            return WAI_OK;
        case WAI_OP_JNZ:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (vm->regs[a] != 0) {
                if (check_jump(vm, ins->imm) != WAI_OK) { return vm->error; }
                vm->ip = (uint64_t)ins->imm;
            }
            return WAI_OK;
        case WAI_OP_PRINT:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            wai_vm_emit_print(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_HALT:
            vm->halted = 1u;
            return WAI_OK;
        case WAI_OP_INVALID:
        default:
            return set_error(vm, WAI_ERR_BAD_OPCODE);
    }
}

wai_error_code wai_vm_execute(wai_vm *vm) {
    int rc = wai_vm_exec_asm(vm);
    if (rc != 0) {
        return vm->error;
    }
    return WAI_OK;
}
