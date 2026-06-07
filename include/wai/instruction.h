#ifndef WAI_INSTRUCTION_H
#define WAI_INSTRUCTION_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t wai_value;

typedef enum wai_opcode {
    WAI_OP_INVALID = 0,
    WAI_OP_MOV_IMM = 1,
    WAI_OP_MOV_REG = 2,
    WAI_OP_ADD_IMM = 3,
    WAI_OP_ADD_REG = 4,
    WAI_OP_SUB_IMM = 5,
    WAI_OP_SUB_REG = 6,
    WAI_OP_MUL_IMM = 7,
    WAI_OP_MUL_REG = 8,
    WAI_OP_DIV_IMM = 9,
    WAI_OP_DIV_REG = 10,
    WAI_OP_JMP = 11,
    WAI_OP_JZ = 12,
    WAI_OP_JNZ = 13,
    WAI_OP_PRINT = 14,
    WAI_OP_HALT = 15
} wai_opcode;

#if defined(_MSC_VER)
#pragma pack(push, 1)
typedef struct wai_instruction {
    uint8_t opcode;
    uint8_t a;
    uint8_t b;
    uint8_t c;
    wai_value imm;
} wai_instruction;
#pragma pack(pop)
#else
typedef struct __attribute__((packed)) wai_instruction {
    uint8_t opcode;
    uint8_t a;
    uint8_t b;
    uint8_t c;
    wai_value imm;
} wai_instruction;
#endif

const char *wai_opcode_name(uint8_t opcode);
int wai_register_is_valid(uint8_t reg);

#ifdef __cplusplus
}
#endif

#endif
