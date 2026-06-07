#include "wai/assembler.h"
#include "wai/verifier.h"
#include <stdio.h>
#include <string.h>

static int require(int condition, const char *message) {
    if (!condition) {
        (void)fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static wai_instruction instr(wai_opcode opcode) {
    wai_instruction out;
    out.opcode = (uint8_t)opcode;
    out.a = 0;
    out.b = 0;
    out.c = 0;
    out.imm = 0;
    return out;
}

int main(void) {
    int failures = 0;

    const char *source =
        "mov r0, 3\n"
        "loop:\n"
        "sub r0, 1\n"
        "jnz r0, loop\n"
        "print r0\n"
        "halt\n";

    wai_program program;
    wai_assembler_result assembled = wai_assemble_source(source, &program);
    failures += require(assembled.error == WAI_OK, "assembler should accept valid source");

    wai_verify_report report;
    failures += require(wai_verify_program(&program, &report) == WAI_OK, "valid program should verify");
    failures += require(strstr(report.message, "instructions verified") != NULL, "verification report should describe success");
    wai_program_free(&program);

    wai_instruction bad_reg[] = {
        {.opcode = WAI_OP_PRINT, .a = 9, .b = 0, .c = 0, .imm = 0},
    };
    program.code = bad_reg;
    program.count = 1u;
    program.capacity = 1u;
    failures += require(wai_verify_program(&program, &report) == WAI_ERR_BAD_REGISTER, "bad register should fail verifier");
    failures += require(report.ip == 0u, "bad register report should point to ip 0");

    wai_instruction bad_jump[] = {
        {.opcode = WAI_OP_JMP, .a = 0, .b = 0, .c = 0, .imm = 99},
    };
    program.code = bad_jump;
    program.count = 1u;
    failures += require(wai_verify_program(&program, &report) == WAI_ERR_BAD_JUMP, "bad jump should fail verifier");

    wai_instruction bad_reserved[] = { instr(WAI_OP_NOP) };
    bad_reserved[0].c = 1u;
    program.code = bad_reserved;
    program.count = 1u;
    failures += require(wai_verify_program(&program, &report) == WAI_ERR_PARSE, "reserved operand should fail verifier");

    wai_instruction bad_shift[] = {
        {.opcode = WAI_OP_SHL_IMM, .a = 0, .b = 0, .c = 0, .imm = 64},
    };
    program.code = bad_shift;
    program.count = 1u;
    failures += require(wai_verify_program(&program, &report) == WAI_ERR_BAD_SHIFT, "bad shift should fail verifier");

    wai_instruction bad_mem[] = {
        {.opcode = WAI_OP_LOAD_ABS, .a = 0, .b = 0, .c = 0, .imm = 65535},
    };
    program.code = bad_mem;
    program.count = 1u;
    failures += require(wai_verify_program(&program, &report) == WAI_ERR_MEMORY_OOB, "bad memory address should fail verifier");

    return failures == 0 ? 0 : 1;
}
