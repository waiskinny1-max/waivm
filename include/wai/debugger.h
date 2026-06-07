#ifndef WAI_DEBUGGER_H
#define WAI_DEBUGGER_H

#include <stdio.h>
#include "wai/bytecode.h"
#include "wai/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

wai_error_code wai_debugger_run(const wai_program *program, FILE *in, FILE *out);

#ifdef __cplusplus
}
#endif

#endif
