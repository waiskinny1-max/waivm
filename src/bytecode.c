#include "wai/bytecode.h"
#include "wai/vm.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
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
    if (program == NULL) {
        return WAI_ERR_PARSE;
    }
    if (program->count == program->capacity) {
        if (program->capacity > (SIZE_MAX / 2u)) {
            return WAI_ERR_OOM;
        }
        size_t new_capacity = program->capacity == 0 ? 16u : program->capacity * 2u;
        if (new_capacity > (SIZE_MAX / sizeof(*program->code))) {
            return WAI_ERR_OOM;
        }
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

static void write_u16_le(uint8_t *dst, uint16_t value) {
    dst[0] = (uint8_t)(value & 0xffu);
    dst[1] = (uint8_t)((value >> 8u) & 0xffu);
}

static void write_u32_le(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)(value & 0xffu);
    dst[1] = (uint8_t)((value >> 8u) & 0xffu);
    dst[2] = (uint8_t)((value >> 16u) & 0xffu);
    dst[3] = (uint8_t)((value >> 24u) & 0xffu);
}

static void write_u64_le(uint8_t *dst, uint64_t value) {
    for (size_t i = 0; i < 8u; i++) {
        dst[i] = (uint8_t)((value >> (i * 8u)) & 0xffu);
    }
}

static uint16_t read_u16_le(const uint8_t *src) {
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8u));
}

static uint32_t read_u32_le(const uint8_t *src) {
    return (uint32_t)src[0] |
           ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) |
           ((uint32_t)src[3] << 24u);
}

static uint64_t read_u64_le(const uint8_t *src) {
    uint64_t value = 0;
    for (size_t i = 0; i < 8u; i++) {
        value |= ((uint64_t)src[i]) << (i * 8u);
    }
    return value;
}

static void encode_instruction(const wai_instruction *ins, uint8_t *dst) {
    dst[0] = ins->opcode;
    dst[1] = ins->a;
    dst[2] = ins->b;
    dst[3] = ins->c;
    write_u64_le(dst + 4u, (uint64_t)ins->imm);
}

static wai_instruction decode_instruction(const uint8_t *src) {
    wai_instruction ins;
    ins.opcode = src[0];
    ins.a = src[1];
    ins.b = src[2];
    ins.c = src[3];
    ins.imm = (wai_value)read_u64_le(src + 4u);
    return ins;
}

static int opcode_known(uint8_t opcode) {
    return opcode >= (uint8_t)WAI_OP_MOV_IMM && opcode <= (uint8_t)WAI_OPCODE_MAX;
}

static int instruction_registers_valid(const wai_instruction *ins) {
    switch ((wai_opcode)ins->opcode) {
        case WAI_OP_MOV_IMM:
        case WAI_OP_ADD_IMM:
        case WAI_OP_SUB_IMM:
        case WAI_OP_MUL_IMM:
        case WAI_OP_DIV_IMM:
        case WAI_OP_MOD_IMM:
        case WAI_OP_AND_IMM:
        case WAI_OP_OR_IMM:
        case WAI_OP_XOR_IMM:
        case WAI_OP_SHL_IMM:
        case WAI_OP_SHR_IMM:
        case WAI_OP_JZ:
        case WAI_OP_JNZ:
        case WAI_OP_PRINT:
        case WAI_OP_LOAD_ABS:
        case WAI_OP_STORE_ABS:
        case WAI_OP_PUSH:
        case WAI_OP_POP:
        case WAI_OP_CMP_IMM:
        case WAI_OP_NOT:
            return wai_register_is_valid(ins->a);
        case WAI_OP_MOV_REG:
        case WAI_OP_ADD_REG:
        case WAI_OP_SUB_REG:
        case WAI_OP_MUL_REG:
        case WAI_OP_DIV_REG:
        case WAI_OP_MOD_REG:
        case WAI_OP_AND_REG:
        case WAI_OP_OR_REG:
        case WAI_OP_XOR_REG:
        case WAI_OP_SHL_REG:
        case WAI_OP_SHR_REG:
        case WAI_OP_LOAD_REG:
        case WAI_OP_STORE_REG:
        case WAI_OP_CMP_REG:
            return wai_register_is_valid(ins->a) && wai_register_is_valid(ins->b);
        case WAI_OP_JMP:
        case WAI_OP_CALL:
        case WAI_OP_JE:
        case WAI_OP_JNE:
        case WAI_OP_HALT:
        case WAI_OP_RET:
        case WAI_OP_NOP:
            return 1;
        case WAI_OP_INVALID:
        default:
            return 0;
    }
}

static wai_error_code validate_program(const wai_program *program) {
    if (program == NULL) {
        return WAI_ERR_PARSE;
    }
    for (size_t i = 0; i < program->count; i++) {
        const wai_instruction *ins = &program->code[i];
        if (ins->c != 0u) {
            return WAI_ERR_PARSE;
        }
        if (!opcode_known(ins->opcode)) {
            return WAI_ERR_BAD_OPCODE;
        }
        if (!instruction_registers_valid(ins)) {
            return WAI_ERR_BAD_REGISTER;
        }
        if ((ins->opcode == (uint8_t)WAI_OP_JMP ||
             ins->opcode == (uint8_t)WAI_OP_JZ ||
             ins->opcode == (uint8_t)WAI_OP_JNZ ||
             ins->opcode == (uint8_t)WAI_OP_CALL ||
             ins->opcode == (uint8_t)WAI_OP_JE ||
             ins->opcode == (uint8_t)WAI_OP_JNE) &&
            (ins->imm < 0 || (uint64_t)ins->imm >= (uint64_t)program->count)) {
            return WAI_ERR_BAD_JUMP;
        }
        if ((ins->opcode == (uint8_t)WAI_OP_LOAD_ABS ||
             ins->opcode == (uint8_t)WAI_OP_STORE_ABS) &&
            (ins->imm < 0 || (uint64_t)ins->imm > (uint64_t)(WAI_MEMORY_SIZE - sizeof(wai_value)))) {
            return WAI_ERR_MEMORY_OOB;
        }
        if ((ins->opcode == (uint8_t)WAI_OP_SHL_IMM ||
             ins->opcode == (uint8_t)WAI_OP_SHR_IMM) &&
            (ins->imm < 0 || ins->imm > 63)) {
            return WAI_ERR_BAD_SHIFT;
        }
    }
    return WAI_OK;
}

static void encode_header(uint8_t *header, uint64_t code_count) {
    memset(header, 0, WAI_BYTECODE_HEADER_SIZE);
    memcpy(header, WAI_BYTECODE_MAGIC, 4u);
    write_u16_le(header + 4u, (uint16_t)WAI_BYTECODE_VERSION);
    write_u16_le(header + 6u, (uint16_t)WAI_BYTECODE_HEADER_SIZE);
    write_u16_le(header + 8u, (uint16_t)WAI_BYTECODE_INSTRUCTION_SIZE);
    write_u16_le(header + 10u, (uint16_t)WAI_BYTECODE_REGISTER_COUNT);
    write_u32_le(header + 12u, 0u);
    write_u64_le(header + 16u, code_count);
    write_u64_le(header + 24u, 0u);
}

static wai_error_code decode_header(const uint8_t *header, wai_bytecode_info *info) {
    if (memcmp(header, WAI_BYTECODE_MAGIC, 4u) != 0) {
        return WAI_ERR_PARSE;
    }
    wai_bytecode_info decoded;
    decoded.version = read_u16_le(header + 4u);
    decoded.header_size = read_u16_le(header + 6u);
    decoded.instruction_size = read_u16_le(header + 8u);
    decoded.register_count = read_u16_le(header + 10u);
    decoded.flags = read_u32_le(header + 12u);
    decoded.code_count = read_u64_le(header + 16u);
    decoded.data_size = read_u64_le(header + 24u);

    if (decoded.version != WAI_BYTECODE_VERSION ||
        decoded.header_size != WAI_BYTECODE_HEADER_SIZE ||
        decoded.instruction_size != WAI_BYTECODE_INSTRUCTION_SIZE ||
        decoded.register_count != WAI_BYTECODE_REGISTER_COUNT ||
        decoded.data_size != 0u) {
        return WAI_ERR_PARSE;
    }
    if (decoded.code_count > (uint64_t)(SIZE_MAX / sizeof(wai_instruction))) {
        return WAI_ERR_OOM;
    }
    if (info != NULL) {
        *info = decoded;
    }
    return WAI_OK;
}

wai_error_code wai_bytecode_write_file(const char *path, const wai_program *program) {
    if (path == NULL || program == NULL) {
        return WAI_ERR_PARSE;
    }
    if (program->count > UINT64_MAX) {
        return WAI_ERR_OOM;
    }
    wai_error_code validation = validate_program(program);
    if (validation != WAI_OK) {
        return validation;
    }

    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        return WAI_ERR_IO;
    }

    uint8_t header[WAI_BYTECODE_HEADER_SIZE];
    encode_header(header, (uint64_t)program->count);
    if (fwrite(header, 1u, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        return WAI_ERR_IO;
    }

    uint8_t encoded[WAI_BYTECODE_INSTRUCTION_SIZE];
    for (size_t i = 0; i < program->count; i++) {
        encode_instruction(&program->code[i], encoded);
        if (fwrite(encoded, 1u, sizeof(encoded), file) != sizeof(encoded)) {
            fclose(file);
            return WAI_ERR_IO;
        }
    }

    if (fclose(file) != 0) {
        return WAI_ERR_IO;
    }
    return WAI_OK;
}

wai_error_code wai_bytecode_read_file(const char *path, wai_program *out_program, wai_bytecode_info *out_info) {
    if (path == NULL || out_program == NULL) {
        return WAI_ERR_PARSE;
    }
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return WAI_ERR_IO;
    }

    uint8_t header[WAI_BYTECODE_HEADER_SIZE];
    if (fread(header, 1u, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        return WAI_ERR_PARSE;
    }

    wai_bytecode_info info;
    wai_error_code status = decode_header(header, &info);
    if (status != WAI_OK) {
        fclose(file);
        return status;
    }

    wai_program_init(out_program);
    if (info.code_count > 0u) {
        out_program->code = calloc((size_t)info.code_count, sizeof(*out_program->code));
        if (out_program->code == NULL) {
            fclose(file);
            return WAI_ERR_OOM;
        }
        out_program->count = (size_t)info.code_count;
        out_program->capacity = (size_t)info.code_count;
    }

    uint8_t encoded[WAI_BYTECODE_INSTRUCTION_SIZE];
    for (size_t i = 0; i < out_program->count; i++) {
        if (fread(encoded, 1u, sizeof(encoded), file) != sizeof(encoded)) {
            wai_program_free(out_program);
            fclose(file);
            return WAI_ERR_PARSE;
        }
        out_program->code[i] = decode_instruction(encoded);
    }

    int trailing = fgetc(file);
    fclose(file);
    if (trailing != EOF) {
        wai_program_free(out_program);
        return WAI_ERR_PARSE;
    }

    status = validate_program(out_program);
    if (status != WAI_OK) {
        wai_program_free(out_program);
        return status;
    }
    if (out_info != NULL) {
        *out_info = info;
    }
    return WAI_OK;
}

wai_error_code wai_bytecode_read_info(const char *path, wai_bytecode_info *out_info) {
    if (path == NULL || out_info == NULL) {
        return WAI_ERR_PARSE;
    }
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return WAI_ERR_IO;
    }
    uint8_t header[WAI_BYTECODE_HEADER_SIZE];
    if (fread(header, 1u, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        return WAI_ERR_PARSE;
    }
    fclose(file);
    return decode_header(header, out_info);
}

int wai_file_has_waibc_magic(const char *path) {
    if (path == NULL) {
        return 0;
    }
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }
    uint8_t magic[4];
    size_t n = fread(magic, 1u, sizeof(magic), file);
    fclose(file);
    return n == sizeof(magic) && memcmp(magic, WAI_BYTECODE_MAGIC, 4u) == 0;
}
