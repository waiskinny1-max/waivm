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
    WAI_OP_HALT = 15,
    WAI_OP_LOAD_ABS = 16,
    WAI_OP_LOAD_REG = 17,
    WAI_OP_STORE_ABS = 18,
    WAI_OP_STORE_REG = 19,
    WAI_OP_PUSH = 20,
    WAI_OP_POP = 21,
    WAI_OP_CALL = 22,
    WAI_OP_RET = 23,
    WAI_OP_CMP_IMM = 24,
    WAI_OP_CMP_REG = 25,
    WAI_OP_JE = 26,
    WAI_OP_JNE = 27,
    WAI_OP_NOP = 28,
    WAI_OP_MOD_IMM = 29,
    WAI_OP_MOD_REG = 30,
    WAI_OP_AND_IMM = 31,
    WAI_OP_AND_REG = 32,
    WAI_OP_OR_IMM = 33,
    WAI_OP_OR_REG = 34,
    WAI_OP_XOR_IMM = 35,
    WAI_OP_XOR_REG = 36,
    WAI_OP_NOT = 37,
    WAI_OP_SHL_IMM = 38,
    WAI_OP_SHL_REG = 39,
    WAI_OP_SHR_IMM = 40,
    WAI_OP_SHR_REG = 41
} wai_opcode;

#define WAI_OPCODE_MAX WAI_OP_SHR_REG

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
