#include "wai/instruction.h"

const char *wai_opcode_name(uint8_t opcode) {
    switch ((wai_opcode)opcode) {
        case WAI_OP_MOV_IMM: return "mov.imm";
        case WAI_OP_MOV_REG: return "mov.reg";
        case WAI_OP_ADD_IMM: return "add.imm";
        case WAI_OP_ADD_REG: return "add.reg";
        case WAI_OP_SUB_IMM: return "sub.imm";
        case WAI_OP_SUB_REG: return "sub.reg";
        case WAI_OP_MUL_IMM: return "mul.imm";
        case WAI_OP_MUL_REG: return "mul.reg";
        case WAI_OP_DIV_IMM: return "div.imm";
        case WAI_OP_DIV_REG: return "div.reg";
        case WAI_OP_JMP: return "jmp";
        case WAI_OP_JZ: return "jz";
        case WAI_OP_JNZ: return "jnz";
        case WAI_OP_PRINT: return "print";
        case WAI_OP_HALT: return "halt";
        case WAI_OP_LOAD_ABS: return "load.abs";
        case WAI_OP_LOAD_REG: return "load.reg";
        case WAI_OP_STORE_ABS: return "store.abs";
        case WAI_OP_STORE_REG: return "store.reg";
        case WAI_OP_PUSH: return "push";
        case WAI_OP_POP: return "pop";
        case WAI_OP_CALL: return "call";
        case WAI_OP_RET: return "ret";
        case WAI_OP_CMP_IMM: return "cmp.imm";
        case WAI_OP_CMP_REG: return "cmp.reg";
        case WAI_OP_JE: return "je";
        case WAI_OP_JNE: return "jne";
        case WAI_OP_INVALID:
        default: return "invalid";
    }
}

int wai_register_is_valid(uint8_t reg) {
    return reg < 8u;
}
