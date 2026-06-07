#ifndef WAI_DISASSEMBLER_H
#define WAI_DISASSEMBLER_H

#include <stdio.h>
#include "wai/bytecode.h"

#ifdef __cplusplus
extern "C" {
#endif

wai_error_code wai_disassemble_instruction(const wai_instruction *instruction, size_t ip, FILE *out);
wai_error_code wai_disassemble_program(const wai_program *program, FILE *out);

#ifdef __cplusplus
}
#endif

#endif
