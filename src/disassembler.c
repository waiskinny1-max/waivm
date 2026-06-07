#include "wai/disassembler.h"

static void print_reg(FILE *out, uint8_t reg) {
    (void)fprintf(out, "r%u", (unsigned)reg);
}

wai_error_code wai_disassemble_instruction(const wai_instruction *ins, size_t ip, FILE *out) {
    if (ins == NULL || out == NULL) {
        return WAI_ERR_PARSE;
    }

    (void)fprintf(out, "%04zu  ", ip);
    switch ((wai_opcode)ins->opcode) {
        case WAI_OP_MOV_IMM:
            (void)fprintf(out, "mov "); print_reg(out, ins->a); (void)fprintf(out, ", %lld", (long long)ins->imm); break;
        case WAI_OP_MOV_REG:
            (void)fprintf(out, "mov "); print_reg(out, ins->a); (void)fprintf(out, ", "); print_reg(out, ins->b); break;
        case WAI_OP_ADD_IMM:
            (void)fprintf(out, "add "); print_reg(out, ins->a); (void)fprintf(out, ", %lld", (long long)ins->imm); break;
        case WAI_OP_ADD_REG:
            (void)fprintf(out, "add "); print_reg(out, ins->a); (void)fprintf(out, ", "); print_reg(out, ins->b); break;
        case WAI_OP_SUB_IMM:
            (void)fprintf(out, "sub "); print_reg(out, ins->a); (void)fprintf(out, ", %lld", (long long)ins->imm); break;
        case WAI_OP_SUB_REG:
            (void)fprintf(out, "sub "); print_reg(out, ins->a); (void)fprintf(out, ", "); print_reg(out, ins->b); break;
        case WAI_OP_MUL_IMM:
            (void)fprintf(out, "mul "); print_reg(out, ins->a); (void)fprintf(out, ", %lld", (long long)ins->imm); break;
        case WAI_OP_MUL_REG:
            (void)fprintf(out, "mul "); print_reg(out, ins->a); (void)fprintf(out, ", "); print_reg(out, ins->b); break;
        case WAI_OP_DIV_IMM:
            (void)fprintf(out, "div "); print_reg(out, ins->a); (void)fprintf(out, ", %lld", (long long)ins->imm); break;
        case WAI_OP_DIV_REG:
            (void)fprintf(out, "div "); print_reg(out, ins->a); (void)fprintf(out, ", "); print_reg(out, ins->b); break;
        case WAI_OP_JMP:
            (void)fprintf(out, "jmp %lld", (long long)ins->imm); break;
        case WAI_OP_JZ:
            (void)fprintf(out, "jz "); print_reg(out, ins->a); (void)fprintf(out, ", %lld", (long long)ins->imm); break;
        case WAI_OP_JNZ:
            (void)fprintf(out, "jnz "); print_reg(out, ins->a); (void)fprintf(out, ", %lld", (long long)ins->imm); break;
        case WAI_OP_PRINT:
            (void)fprintf(out, "print "); print_reg(out, ins->a); break;
        case WAI_OP_HALT:
            (void)fprintf(out, "halt"); break;
        case WAI_OP_INVALID:
        default:
            (void)fprintf(out, ".byte 0x%02x ; invalid opcode", (unsigned)ins->opcode); break;
    }
    (void)fprintf(out, "\n");
    return WAI_OK;
}

wai_error_code wai_disassemble_program(const wai_program *program, FILE *out) {
    if (program == NULL || out == NULL) {
        return WAI_ERR_PARSE;
    }
    for (size_t i = 0; i < program->count; i++) {
        wai_error_code status = wai_disassemble_instruction(&program->code[i], i, out);
        if (status != WAI_OK) {
            return status;
        }
    }
    return WAI_OK;
}
