#ifndef WAI_BYTECODE_H
#define WAI_BYTECODE_H

#include <stddef.h>
#include "wai/instruction.h"
#include "wai/error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wai_program {
    wai_instruction *code;
    size_t count;
    size_t capacity;
} wai_program;

void wai_program_init(wai_program *program);
void wai_program_free(wai_program *program);
wai_error_code wai_program_push(wai_program *program, wai_instruction instruction);

#ifdef __cplusplus
}
#endif

#endif
