#include "wai/bytecode.h"
#include <stdlib.h>
#include <string.h>

void wai_program_init(wai_program *program) {
    program->code = NULL;
    program->count = 0;
    program->capacity = 0;
}

void wai_program_free(wai_program *program) {
    if (program == NULL) {
        return;
    }
    free(program->code);
    program->code = NULL;
    program->count = 0;
    program->capacity = 0;
}

wai_error_code wai_program_push(wai_program *program, wai_instruction instruction) {
    if (program->count == program->capacity) {
        size_t new_capacity = program->capacity == 0 ? 16u : program->capacity * 2u;
        wai_instruction *new_code = realloc(program->code, new_capacity * sizeof(*new_code));
        if (new_code == NULL) {
            return WAI_ERR_OOM;
        }
        program->code = new_code;
        program->capacity = new_capacity;
    }

    memcpy(&program->code[program->count], &instruction, sizeof(instruction));
    program->count += 1u;
    return WAI_OK;
}
