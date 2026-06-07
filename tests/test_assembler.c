#include "wai/assembler.h"
#include "wai/instruction.h"
#include <stdio.h>

static int require(int condition, const char *message) {
    if (!condition) {
        (void)fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

int main(void) {
    const char *source =
        "mov r0, 2\n"
        "mov r1, 3\n"
        "add r0, r1\n"
        "print r0\n"
        "halt\n";

    wai_program program;
    wai_assembler_result result = wai_assemble_source(source, &program);
    int failures = 0;
    failures += require(result.error == WAI_OK, "assembler should accept valid source");
    failures += require(program.count == 5u, "program should have 5 instructions");
    failures += require(program.code[0].opcode == WAI_OP_MOV_IMM, "first opcode should be mov immediate");
    failures += require(program.code[2].opcode == WAI_OP_ADD_REG, "third opcode should be add register");
    wai_program_free(&program);

    const char *bad_source = "wat r0, 1\n";
    result = wai_assemble_source(bad_source, &program);
    failures += require(result.error == WAI_ERR_PARSE, "assembler should reject unknown instruction");

    return failures == 0 ? 0 : 1;
}
