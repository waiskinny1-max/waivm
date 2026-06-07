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
_Static_assert(offsetof(wai_vm, memory) == 128u, "wai_vm.memory ABI offset mismatch");
_Static_assert(offsetof(wai_vm, sp) == 65664u, "wai_vm.sp ABI offset mismatch");

void wai_vm_init(wai_vm *vm, const wai_instruction *code, uint64_t code_count) {
    memset(vm, 0, sizeof(*vm));
    vm->code = code;
    vm->code_count = code_count;
    vm->error = WAI_OK;
    vm->print_stream = stdout;
    vm->sp = WAI_STACK_START;
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

static wai_error_code check_address_u64(uint64_t address) {
    if (address > (uint64_t)(WAI_MEMORY_SIZE - sizeof(wai_value))) {
        return WAI_ERR_MEMORY_OOB;
    }
    return WAI_OK;
}

static wai_error_code check_address_i64(wai_value address, uint64_t *out) {
    if (address < 0) {
        return WAI_ERR_MEMORY_OOB;
    }
    uint64_t converted = (uint64_t)address;
    wai_error_code status = check_address_u64(converted);
    if (status != WAI_OK) {
        return status;
    }
    *out = converted;
    return WAI_OK;
}

wai_error_code wai_vm_memory_load_i64(const wai_vm *vm, uint64_t address, wai_value *out) {
    if (vm == NULL || out == NULL) {
        return WAI_ERR_PARSE;
    }
    wai_error_code status = check_address_u64(address);
    if (status != WAI_OK) {
        return status;
    }
    uint64_t raw = 0;
    for (size_t i = 0; i < sizeof(raw); i++) {
        raw |= ((uint64_t)vm->memory[address + i]) << (i * 8u);
    }
    *out = (wai_value)raw;
    return WAI_OK;
}

wai_error_code wai_vm_memory_store_i64(wai_vm *vm, uint64_t address, wai_value value) {
    if (vm == NULL) {
        return WAI_ERR_PARSE;
    }
    wai_error_code status = check_address_u64(address);
    if (status != WAI_OK) {
        return status;
    }
    uint64_t raw = (uint64_t)value;
    for (size_t i = 0; i < sizeof(raw); i++) {
        vm->memory[address + i] = (uint8_t)((raw >> (i * 8u)) & 0xffu);
    }
    return WAI_OK;
}

static wai_error_code push_i64(wai_vm *vm, wai_value value) {
    if (vm->sp < sizeof(wai_value)) {
        return set_error(vm, WAI_ERR_STACK_OVERFLOW);
    }
    vm->sp -= sizeof(wai_value);
    wai_error_code status = wai_vm_memory_store_i64(vm, vm->sp, value);
    if (status != WAI_OK) {
        return set_error(vm, status);
    }
    return WAI_OK;
}

static wai_error_code pop_i64(wai_vm *vm, wai_value *out) {
    if (vm->sp > (uint64_t)(WAI_MEMORY_SIZE - sizeof(wai_value))) {
        return set_error(vm, WAI_ERR_STACK_UNDERFLOW);
    }
    wai_error_code status = wai_vm_memory_load_i64(vm, vm->sp, out);
    if (status != WAI_OK) {
        return set_error(vm, status);
    }
    vm->sp += sizeof(wai_value);
    return WAI_OK;
}

static void update_zf(wai_vm *vm, wai_value value) {
    vm->zf = value == 0 ? 1u : 0u;
}

static wai_error_code check_shift(wai_vm *vm, wai_value shift, uint8_t *out) {
    if (shift < 0 || shift > 63) {
        return set_error(vm, WAI_ERR_BAD_SHIFT);
    }
    *out = (uint8_t)shift;
    return WAI_OK;
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
    uint64_t address = 0;
    uint8_t shift = 0;

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
        case WAI_OP_MOD_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (ins->imm == 0) { return set_error(vm, WAI_ERR_DIV_ZERO); }
            vm->regs[a] %= ins->imm;
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_MOD_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            value = vm->regs[b];
            if (value == 0) { return set_error(vm, WAI_ERR_DIV_ZERO); }
            vm->regs[a] %= value;
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
        case WAI_OP_LOAD_ABS:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (check_address_i64(ins->imm, &address) != WAI_OK) { return set_error(vm, WAI_ERR_MEMORY_OOB); }
            if (wai_vm_memory_load_i64(vm, address, &value) != WAI_OK) { return set_error(vm, WAI_ERR_MEMORY_OOB); }
            vm->regs[a] = value;
            update_zf(vm, value);
            return WAI_OK;
        case WAI_OP_LOAD_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (check_address_i64(vm->regs[b], &address) != WAI_OK) { return set_error(vm, WAI_ERR_MEMORY_OOB); }
            if (wai_vm_memory_load_i64(vm, address, &value) != WAI_OK) { return set_error(vm, WAI_ERR_MEMORY_OOB); }
            vm->regs[a] = value;
            update_zf(vm, value);
            return WAI_OK;
        case WAI_OP_STORE_ABS:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (check_address_i64(ins->imm, &address) != WAI_OK) { return set_error(vm, WAI_ERR_MEMORY_OOB); }
            if (wai_vm_memory_store_i64(vm, address, vm->regs[a]) != WAI_OK) { return set_error(vm, WAI_ERR_MEMORY_OOB); }
            return WAI_OK;
        case WAI_OP_STORE_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (check_address_i64(vm->regs[b], &address) != WAI_OK) { return set_error(vm, WAI_ERR_MEMORY_OOB); }
            if (wai_vm_memory_store_i64(vm, address, vm->regs[a]) != WAI_OK) { return set_error(vm, WAI_ERR_MEMORY_OOB); }
            return WAI_OK;
        case WAI_OP_PUSH:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            return push_i64(vm, vm->regs[a]);
        case WAI_OP_POP:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (pop_i64(vm, &value) != WAI_OK) { return vm->error; }
            vm->regs[a] = value;
            update_zf(vm, value);
            return WAI_OK;
        case WAI_OP_CALL:
            if (check_jump(vm, ins->imm) != WAI_OK) { return vm->error; }
            if (push_i64(vm, (wai_value)vm->ip) != WAI_OK) { return vm->error; }
            vm->ip = (uint64_t)ins->imm;
            return WAI_OK;
        case WAI_OP_RET:
            if (pop_i64(vm, &value) != WAI_OK) { return vm->error; }
            if (check_jump(vm, value) != WAI_OK) { return vm->error; }
            vm->ip = (uint64_t)value;
            return WAI_OK;
        case WAI_OP_CMP_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->zf = vm->regs[a] == ins->imm ? 1u : 0u;
            return WAI_OK;
        case WAI_OP_CMP_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->zf = vm->regs[a] == vm->regs[b] ? 1u : 0u;
            return WAI_OK;
        case WAI_OP_JE:
            if (vm->zf != 0u) {
                if (check_jump(vm, ins->imm) != WAI_OK) { return vm->error; }
                vm->ip = (uint64_t)ins->imm;
            }
            return WAI_OK;
        case WAI_OP_JNE:
            if (vm->zf == 0u) {
                if (check_jump(vm, ins->imm) != WAI_OK) { return vm->error; }
                vm->ip = (uint64_t)ins->imm;
            }
            return WAI_OK;
        case WAI_OP_NOP:
            return WAI_OK;
        case WAI_OP_AND_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] & (uint64_t)ins->imm);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_AND_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] & (uint64_t)vm->regs[b]);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_OR_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] | (uint64_t)ins->imm);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_OR_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] | (uint64_t)vm->regs[b]);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_XOR_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] ^ (uint64_t)ins->imm);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_XOR_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] ^ (uint64_t)vm->regs[b]);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_NOT:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            vm->regs[a] = (wai_value)(~(uint64_t)vm->regs[a]);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_SHL_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (check_shift(vm, ins->imm, &shift) != WAI_OK) { return vm->error; }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] << shift);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_SHL_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (check_shift(vm, vm->regs[b], &shift) != WAI_OK) { return vm->error; }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] << shift);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_SHR_IMM:
            if (!valid_reg(a)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (check_shift(vm, ins->imm, &shift) != WAI_OK) { return vm->error; }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] >> shift);
            update_zf(vm, vm->regs[a]);
            return WAI_OK;
        case WAI_OP_SHR_REG:
            if (!valid_reg(a) || !valid_reg(b)) { return set_error(vm, WAI_ERR_BAD_REGISTER); }
            if (check_shift(vm, vm->regs[b], &shift) != WAI_OK) { return vm->error; }
            vm->regs[a] = (wai_value)((uint64_t)vm->regs[a] >> shift);
            update_zf(vm, vm->regs[a]);
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
