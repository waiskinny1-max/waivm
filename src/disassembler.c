#include "wai/disassembler.h"

wai_error_code wai_disassemble_program(const wai_program *program, FILE *out) {
    if (program == NULL || out == NULL) {
        return WAI_ERR_PARSE;
    }
    for (size_t i = 0; i < program->count; i++) {
        const wai_instruction *ins = &program->code[i];
        (void)fprintf(out, "%04zu  %-8s a=%u b=%u imm=%lld\n",
                      i,
                      wai_opcode_name(ins->opcode),
                      (unsigned)ins->a,
                      (unsigned)ins->b,
                      (long long)ins->imm);
    }
    return WAI_OK;
}
