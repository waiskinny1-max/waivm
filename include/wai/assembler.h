#ifndef WAI_ASSEMBLER_H
#define WAI_ASSEMBLER_H

#include "wai/bytecode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wai_assembler_result {
    wai_error_code error;
    int line;
    char message[256];
} wai_assembler_result;

wai_assembler_result wai_assemble_source(const char *source, wai_program *out_program);
wai_assembler_result wai_assemble_file(const char *path, wai_program *out_program);

#ifdef __cplusplus
}
#endif

#endif
