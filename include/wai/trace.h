#ifndef WAI_TRACE_H
#define WAI_TRACE_H

#include <stdio.h>
#include "wai/bytecode.h"
#include "wai/error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WAI_TRACE_MAX_STEPS 1000000u

wai_error_code wai_trace_program(const wai_program *program, FILE *out);

#ifdef __cplusplus
}
#endif

#endif
