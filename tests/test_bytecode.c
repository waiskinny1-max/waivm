#include "wai/bytecode.h"
#include <stdio.h>

int main(void) {
    if (sizeof(wai_instruction) != 12u) {
        (void)fprintf(stderr, "instruction encoding must be 12 bytes\n");
        return 1;
    }

    wai_program program;
    wai_program_init(&program);
    wai_instruction halt = {.opcode = WAI_OP_HALT, .a = 0, .b = 0, .c = 0, .imm = 0};
    if (wai_program_push(&program, halt) != WAI_OK) {
        return 1;
    }
    if (program.count != 1u || program.code[0].opcode != WAI_OP_HALT) {
        wai_program_free(&program);
        return 1;
    }
    wai_program_free(&program);
    return 0;
}
