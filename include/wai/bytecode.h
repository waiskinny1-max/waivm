#ifndef WAI_BYTECODE_H
#define WAI_BYTECODE_H

#include <stdint.h>
#include <stddef.h>
#include "wai/instruction.h"
#include "wai/error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WAI_BYTECODE_MAGIC "WAI0"
#define WAI_BYTECODE_VERSION 5u
#define WAI_BYTECODE_HEADER_SIZE 32u
#define WAI_BYTECODE_INSTRUCTION_SIZE 12u
#define WAI_BYTECODE_REGISTER_COUNT 8u

typedef struct wai_program {
    wai_instruction *code;
    size_t count;
    size_t capacity;
} wai_program;

typedef struct wai_bytecode_info {
    uint16_t version;
    uint16_t header_size;
    uint16_t instruction_size;
    uint16_t register_count;
    uint32_t flags;
    uint64_t code_count;
    uint64_t data_size;
} wai_bytecode_info;

void wai_program_init(wai_program *program);
void wai_program_free(wai_program *program);
wai_error_code wai_program_push(wai_program *program, wai_instruction instruction);

wai_error_code wai_bytecode_write_file(const char *path, const wai_program *program);
wai_error_code wai_bytecode_read_file(const char *path, wai_program *out_program, wai_bytecode_info *out_info);
wai_error_code wai_bytecode_read_info(const char *path, wai_bytecode_info *out_info);
int wai_file_has_waibc_magic(const char *path);

#ifdef __cplusplus
}
#endif

#endif
