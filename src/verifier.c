#include "wai/verifier.h"
#include "wai/vm.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void report_set(wai_verify_report *report, wai_error_code error, size_t ip, const char *fmt, ...) {
    if (report == NULL) {
        return;
    }
    report->error = error;
    report->ip = ip;
    if (fmt == NULL) {
        report->message[0] = '\0';
        return;
    }
    va_list args;
    va_start(args, fmt);
    (void)vsnprintf(report->message, sizeof(report->message), fmt, args);
    va_end(args);
}

static int opcode_known(uint8_t opcode) {
    return opcode >= (uint8_t)WAI_OP_MOV_IMM && opcode <= (uint8_t)WAI_OPCODE_MAX;
}

static int jump_target_valid(const wai_program *program, wai_value target) {
    return target >= 0 && (uint64_t)target < (uint64_t)program->count;
}

static int address_valid(wai_value address) {
    return address >= 0 && (uint64_t)address <= (uint64_t)(WAI_MEMORY_SIZE - sizeof(wai_value));
}

static wai_error_code verify_reg(uint8_t reg, size_t ip, const char *name, wai_verify_report *report) {
    if (!wai_register_is_valid(reg)) {
        report_set(report, WAI_ERR_BAD_REGISTER, ip, "invalid %s register r%u", name, (unsigned)reg);
        return WAI_ERR_BAD_REGISTER;
    }
    return WAI_OK;
}

static wai_error_code verify_jump(const wai_program *program, const wai_instruction *ins, size_t ip, wai_verify_report *report) {
    if (!jump_target_valid(program, ins->imm)) {
        report_set(report, WAI_ERR_BAD_JUMP, ip, "jump target %lld is outside 0..%llu",
                   (long long)ins->imm,
                   program->count == 0u ? 0ull : (unsigned long long)(program->count - 1u));
        return WAI_ERR_BAD_JUMP;
    }
    return WAI_OK;
}

wai_error_code wai_verify_program(const wai_program *program, wai_verify_report *out_report) {
    report_set(out_report, WAI_OK, 0u, "ok");

    if (program == NULL) {
        report_set(out_report, WAI_ERR_PARSE, 0u, "program is null");
        return WAI_ERR_PARSE;
    }
    if (program->count > 0u && program->code == NULL) {
        report_set(out_report, WAI_ERR_PARSE, 0u, "program code pointer is null");
        return WAI_ERR_PARSE;
    }

    for (size_t ip = 0; ip < program->count; ip++) {
        const wai_instruction *ins = &program->code[ip];
        if (ins->c != 0u) {
            report_set(out_report, WAI_ERR_PARSE, ip, "reserved operand c must be zero, got %u", (unsigned)ins->c);
            return WAI_ERR_PARSE;
        }
        if (!opcode_known(ins->opcode)) {
            report_set(out_report, WAI_ERR_BAD_OPCODE, ip, "unknown opcode 0x%02x", (unsigned)ins->opcode);
            return WAI_ERR_BAD_OPCODE;
        }

        switch ((wai_opcode)ins->opcode) {
            case WAI_OP_MOV_IMM:
            case WAI_OP_ADD_IMM:
            case WAI_OP_SUB_IMM:
            case WAI_OP_MUL_IMM:
            case WAI_OP_DIV_IMM:
            case WAI_OP_MOD_IMM:
            case WAI_OP_AND_IMM:
            case WAI_OP_OR_IMM:
            case WAI_OP_XOR_IMM:
            case WAI_OP_CMP_IMM:
            case WAI_OP_LOAD_ABS:
            case WAI_OP_STORE_ABS:
            case WAI_OP_PUSH:
            case WAI_OP_POP:
            case WAI_OP_PRINT:
            case WAI_OP_NOT:
            case WAI_OP_JZ:
            case WAI_OP_JNZ: {
                wai_error_code status = verify_reg(ins->a, ip, "operand", out_report);
                if (status != WAI_OK) { return status; }
                break;
            }
            case WAI_OP_SHL_IMM:
            case WAI_OP_SHR_IMM: {
                wai_error_code status = verify_reg(ins->a, ip, "operand", out_report);
                if (status != WAI_OK) { return status; }
                if (ins->imm < 0 || ins->imm > 63) {
                    report_set(out_report, WAI_ERR_BAD_SHIFT, ip, "shift count %lld is outside 0..63", (long long)ins->imm);
                    return WAI_ERR_BAD_SHIFT;
                }
                break;
            }
            case WAI_OP_MOV_REG:
            case WAI_OP_ADD_REG:
            case WAI_OP_SUB_REG:
            case WAI_OP_MUL_REG:
            case WAI_OP_DIV_REG:
            case WAI_OP_MOD_REG:
            case WAI_OP_AND_REG:
            case WAI_OP_OR_REG:
            case WAI_OP_XOR_REG:
            case WAI_OP_CMP_REG:
            case WAI_OP_LOAD_REG:
            case WAI_OP_STORE_REG:
            case WAI_OP_SHL_REG:
            case WAI_OP_SHR_REG: {
                wai_error_code status = verify_reg(ins->a, ip, "first", out_report);
                if (status != WAI_OK) { return status; }
                status = verify_reg(ins->b, ip, "second", out_report);
                if (status != WAI_OK) { return status; }
                break;
            }
            case WAI_OP_JMP:
            case WAI_OP_CALL:
            case WAI_OP_JE:
            case WAI_OP_JNE:
            case WAI_OP_HALT:
            case WAI_OP_RET:
            case WAI_OP_NOP:
                break;
            case WAI_OP_INVALID:
            default:
                report_set(out_report, WAI_ERR_BAD_OPCODE, ip, "invalid opcode 0x%02x", (unsigned)ins->opcode);
                return WAI_ERR_BAD_OPCODE;
        }

        switch ((wai_opcode)ins->opcode) {
            case WAI_OP_DIV_IMM:
            case WAI_OP_MOD_IMM:
                if (ins->imm == 0) {
                    report_set(out_report, WAI_ERR_DIV_ZERO, ip, "%s immediate divisor must not be zero", wai_opcode_name(ins->opcode));
                    return WAI_ERR_DIV_ZERO;
                }
                break;
            case WAI_OP_JMP:
            case WAI_OP_JZ:
            case WAI_OP_JNZ:
            case WAI_OP_CALL:
            case WAI_OP_JE:
            case WAI_OP_JNE: {
                wai_error_code status = verify_jump(program, ins, ip, out_report);
                if (status != WAI_OK) { return status; }
                break;
            }
            case WAI_OP_LOAD_ABS:
            case WAI_OP_STORE_ABS:
                if (!address_valid(ins->imm)) {
                    report_set(out_report, WAI_ERR_MEMORY_OOB, ip, "memory address %lld is outside 0..%u",
                               (long long)ins->imm, (unsigned)(WAI_MEMORY_SIZE - sizeof(wai_value)));
                    return WAI_ERR_MEMORY_OOB;
                }
                break;
            default:
                break;
        }
    }

    report_set(out_report, WAI_OK, 0u, "ok: %llu instructions verified", (unsigned long long)program->count);
    return WAI_OK;
}
