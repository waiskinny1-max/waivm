#define _POSIX_C_SOURCE 200809L
#include "wai/assembler.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct label_def {
    char name[64];
    size_t target;
} label_def;

typedef struct pending_ref {
    size_t instruction_index;
    char name[64];
    int line;
} pending_ref;

typedef struct label_table {
    label_def *labels;
    size_t label_count;
    size_t label_capacity;
    pending_ref *pending;
    size_t pending_count;
    size_t pending_capacity;
} label_table;

typedef enum mem_operand_kind {
    MEM_OPERAND_INVALID = 0,
    MEM_OPERAND_ABS,
    MEM_OPERAND_REG
} mem_operand_kind;

typedef struct mem_operand {
    mem_operand_kind kind;
    uint8_t reg;
    wai_value address;
} mem_operand;

static void result_set(wai_assembler_result *result, wai_error_code error, int line, const char *message) {
    result->error = error;
    result->line = line;
    if (message == NULL) {
        result->message[0] = '\0';
        return;
    }
    (void)snprintf(result->message, sizeof(result->message), "%s", message);
}

static char *read_entire_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        return NULL;
    }
    rewind(file);
    char *buffer = malloc((size_t)length + 1u);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    size_t read_count = fread(buffer, 1u, (size_t)length, file);
    fclose(file);
    if (read_count != (size_t)length) {
        free(buffer);
        return NULL;
    }
    buffer[read_count] = '\0';
    return buffer;
}

static char *trim(char *text) {
    while (*text != '\0' && isspace((unsigned char)*text)) {
        text++;
    }
    if (*text == '\0') {
        return text;
    }
    char *end = text + strlen(text) - 1u;
    while (end > text && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return text;
}

static int is_identifier(const char *text) {
    if (text == NULL || *text == '\0') {
        return 0;
    }
    if (!(isalpha((unsigned char)*text) || *text == '_')) {
        return 0;
    }
    text++;
    while (*text != '\0') {
        if (!(isalnum((unsigned char)*text) || *text == '_')) {
            return 0;
        }
        text++;
    }
    return 1;
}

static int parse_register(const char *token, uint8_t *out) {
    if (token == NULL || token[0] != 'r' || token[1] == '\0' || token[2] != '\0') {
        return 0;
    }
    if (token[1] < '0' || token[1] > '7') {
        return 0;
    }
    *out = (uint8_t)(token[1] - '0');
    return 1;
}

static int parse_i64(const char *token, wai_value *out) {
    if (token == NULL || *token == '\0') {
        return 0;
    }
    errno = 0;
    char *end = NULL;
    long long value = strtoll(token, &end, 0);
    if (errno == ERANGE || end == token || *end != '\0') {
        return 0;
    }
    *out = (wai_value)value;
    return 1;
}

static int parse_memory_operand(const char *token, mem_operand *out) {
    size_t len = token == NULL ? 0u : strlen(token);
    if (len < 3u || token[0] != '[' || token[len - 1u] != ']') {
        return 0;
    }
    char inner[64];
    if (len - 2u >= sizeof(inner)) {
        return 0;
    }
    memcpy(inner, token + 1u, len - 2u);
    inner[len - 2u] = '\0';
    char *trimmed = trim(inner);

    uint8_t reg = 0;
    wai_value address = 0;
    if (parse_register(trimmed, &reg)) {
        out->kind = MEM_OPERAND_REG;
        out->reg = reg;
        out->address = 0;
        return 1;
    }
    if (parse_i64(trimmed, &address)) {
        out->kind = MEM_OPERAND_ABS;
        out->reg = 0;
        out->address = address;
        return 1;
    }
    return 0;
}

static void label_table_free(label_table *table) {
    free(table->labels);
    free(table->pending);
    table->labels = NULL;
    table->pending = NULL;
    table->label_count = 0;
    table->label_capacity = 0;
    table->pending_count = 0;
    table->pending_capacity = 0;
}

static int label_find(const label_table *table, const char *name, size_t *out_target) {
    for (size_t i = 0; i < table->label_count; i++) {
        if (strcmp(table->labels[i].name, name) == 0) {
            *out_target = table->labels[i].target;
            return 1;
        }
    }
    return 0;
}

static wai_error_code label_add(label_table *table, const char *name, size_t target) {
    size_t unused = 0;
    if (label_find(table, name, &unused)) {
        return WAI_ERR_PARSE;
    }
    if (table->label_count == table->label_capacity) {
        size_t new_capacity = table->label_capacity == 0 ? 16u : table->label_capacity * 2u;
        label_def *new_labels = realloc(table->labels, new_capacity * sizeof(*new_labels));
        if (new_labels == NULL) {
            return WAI_ERR_OOM;
        }
        table->labels = new_labels;
        table->label_capacity = new_capacity;
    }
    (void)snprintf(table->labels[table->label_count].name, sizeof(table->labels[table->label_count].name), "%s", name);
    table->labels[table->label_count].target = target;
    table->label_count += 1u;
    return WAI_OK;
}

static wai_error_code pending_add(label_table *table, size_t instruction_index, const char *name, int line) {
    if (table->pending_count == table->pending_capacity) {
        size_t new_capacity = table->pending_capacity == 0 ? 16u : table->pending_capacity * 2u;
        pending_ref *new_pending = realloc(table->pending, new_capacity * sizeof(*new_pending));
        if (new_pending == NULL) {
            return WAI_ERR_OOM;
        }
        table->pending = new_pending;
        table->pending_capacity = new_capacity;
    }
    table->pending[table->pending_count].instruction_index = instruction_index;
    table->pending[table->pending_count].line = line;
    (void)snprintf(table->pending[table->pending_count].name, sizeof(table->pending[table->pending_count].name), "%s", name);
    table->pending_count += 1u;
    return WAI_OK;
}

static int tokenize(char *line, char **tokens, size_t max_tokens) {
    size_t count = 0;
    char *cursor = line;
    while (*cursor != '\0') {
        while (*cursor != '\0' && (isspace((unsigned char)*cursor) || *cursor == ',')) {
            cursor++;
        }
        if (*cursor == '\0') {
            break;
        }
        if (count >= max_tokens) {
            return -1;
        }
        tokens[count++] = cursor;
        while (*cursor != '\0' && !isspace((unsigned char)*cursor) && *cursor != ',') {
            cursor++;
        }
        if (*cursor != '\0') {
            *cursor = '\0';
            cursor++;
        }
    }
    return (int)count;
}

static wai_instruction make_instr(wai_opcode opcode) {
    wai_instruction instr;
    instr.opcode = (uint8_t)opcode;
    instr.a = 0;
    instr.b = 0;
    instr.c = 0;
    instr.imm = 0;
    return instr;
}

static wai_error_code parse_two_operand_alu(const char *mnemonic, char **tokens, int token_count, wai_instruction *out) {
    if (token_count != 3) {
        return WAI_ERR_PARSE;
    }
    uint8_t dst = 0;
    if (!parse_register(tokens[1], &dst)) {
        return WAI_ERR_PARSE;
    }

    uint8_t src = 0;
    wai_value imm = 0;
    int rhs_is_reg = parse_register(tokens[2], &src);
    int rhs_is_imm = parse_i64(tokens[2], &imm);

    if (!rhs_is_reg && !rhs_is_imm) {
        return WAI_ERR_PARSE;
    }

    if (strcmp(mnemonic, "mov") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_MOV_REG : WAI_OP_MOV_IMM);
    } else if (strcmp(mnemonic, "add") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_ADD_REG : WAI_OP_ADD_IMM);
    } else if (strcmp(mnemonic, "sub") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_SUB_REG : WAI_OP_SUB_IMM);
    } else if (strcmp(mnemonic, "mul") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_MUL_REG : WAI_OP_MUL_IMM);
    } else if (strcmp(mnemonic, "div") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_DIV_REG : WAI_OP_DIV_IMM);
    } else if (strcmp(mnemonic, "mod") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_MOD_REG : WAI_OP_MOD_IMM);
    } else if (strcmp(mnemonic, "and") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_AND_REG : WAI_OP_AND_IMM);
    } else if (strcmp(mnemonic, "or") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_OR_REG : WAI_OP_OR_IMM);
    } else if (strcmp(mnemonic, "xor") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_XOR_REG : WAI_OP_XOR_IMM);
    } else if (strcmp(mnemonic, "shl") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_SHL_REG : WAI_OP_SHL_IMM);
    } else if (strcmp(mnemonic, "shr") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_SHR_REG : WAI_OP_SHR_IMM);
    } else if (strcmp(mnemonic, "cmp") == 0) {
        *out = make_instr(rhs_is_reg ? WAI_OP_CMP_REG : WAI_OP_CMP_IMM);
    } else {
        return WAI_ERR_PARSE;
    }

    out->a = dst;
    if (rhs_is_reg) {
        out->b = src;
    } else {
        out->imm = imm;
    }
    return WAI_OK;
}

static wai_error_code parse_load(char **tokens, int token_count, wai_instruction *out) {
    if (token_count != 3) {
        return WAI_ERR_PARSE;
    }
    uint8_t dst = 0;
    mem_operand mem;
    if (!parse_register(tokens[1], &dst) || !parse_memory_operand(tokens[2], &mem)) {
        return WAI_ERR_PARSE;
    }
    *out = make_instr(mem.kind == MEM_OPERAND_REG ? WAI_OP_LOAD_REG : WAI_OP_LOAD_ABS);
    out->a = dst;
    if (mem.kind == MEM_OPERAND_REG) {
        out->b = mem.reg;
    } else {
        out->imm = mem.address;
    }
    return WAI_OK;
}

static wai_error_code parse_store(char **tokens, int token_count, wai_instruction *out) {
    if (token_count != 3) {
        return WAI_ERR_PARSE;
    }
    mem_operand mem;
    uint8_t src = 0;
    if (!parse_memory_operand(tokens[1], &mem) || !parse_register(tokens[2], &src)) {
        return WAI_ERR_PARSE;
    }
    *out = make_instr(mem.kind == MEM_OPERAND_REG ? WAI_OP_STORE_REG : WAI_OP_STORE_ABS);
    out->a = src;
    if (mem.kind == MEM_OPERAND_REG) {
        out->b = mem.reg;
    } else {
        out->imm = mem.address;
    }
    return WAI_OK;
}

static wai_error_code parse_one_register(wai_opcode opcode, char **tokens, int token_count, wai_instruction *out) {
    if (token_count != 2) {
        return WAI_ERR_PARSE;
    }
    uint8_t reg = 0;
    if (!parse_register(tokens[1], &reg)) {
        return WAI_ERR_PARSE;
    }
    *out = make_instr(opcode);
    out->a = reg;
    return WAI_OK;
}

static wai_error_code parse_label_jump(wai_opcode opcode, char **tokens, int token_count, label_table *labels, wai_program *program, int line_number, wai_instruction *out) {
    if (token_count != 2 || !is_identifier(tokens[1])) {
        return WAI_ERR_PARSE;
    }
    *out = make_instr(opcode);
    return pending_add(labels, program->count, tokens[1], line_number);
}

static wai_error_code resolve_pending(const label_table *labels, wai_program *program, wai_assembler_result *result) {
    for (size_t i = 0; i < labels->pending_count; i++) {
        size_t target = 0;
        if (!label_find(labels, labels->pending[i].name, &target)) {
            char message[256];
            (void)snprintf(message, sizeof(message), "unknown label '%s'", labels->pending[i].name);
            result_set(result, WAI_ERR_PARSE, labels->pending[i].line, message);
            return WAI_ERR_PARSE;
        }
        if (target > (size_t)INT64_MAX) {
            result_set(result, WAI_ERR_PARSE, labels->pending[i].line, "jump target is too large");
            return WAI_ERR_PARSE;
        }
        program->code[labels->pending[i].instruction_index].imm = (wai_value)target;
    }
    return WAI_OK;
}

wai_assembler_result wai_assemble_source(const char *source, wai_program *out_program) {
    wai_assembler_result result;
    result_set(&result, WAI_OK, 0, "ok");

    if (source == NULL || out_program == NULL) {
        result_set(&result, WAI_ERR_PARSE, 0, "source or output program is null");
        return result;
    }

    wai_program_init(out_program);
    label_table labels;
    memset(&labels, 0, sizeof(labels));

    char *copy = strdup(source);
    if (copy == NULL) {
        result_set(&result, WAI_ERR_OOM, 0, "out of memory");
        return result;
    }

    int line_number = 0;
    char *save = NULL;
    for (char *raw_line = strtok_r(copy, "\n", &save); raw_line != NULL; raw_line = strtok_r(NULL, "\n", &save)) {
        line_number++;
        char *comment = strchr(raw_line, ';');
        if (comment != NULL) {
            *comment = '\0';
        }
        char *line = trim(raw_line);
        if (*line == '\0') {
            continue;
        }

        size_t len = strlen(line);
        if (len > 0u && line[len - 1u] == ':') {
            line[len - 1u] = '\0';
            char *label = trim(line);
            if (!is_identifier(label)) {
                result_set(&result, WAI_ERR_PARSE, line_number, "invalid label syntax");
                goto fail;
            }
            wai_error_code label_status = label_add(&labels, label, out_program->count);
            if (label_status == WAI_ERR_OOM) {
                result_set(&result, WAI_ERR_OOM, line_number, "out of memory");
                goto fail;
            }
            if (label_status != WAI_OK) {
                result_set(&result, WAI_ERR_PARSE, line_number, "duplicate label");
                goto fail;
            }
            continue;
        }

        char *tokens[4] = {0};
        int token_count = tokenize(line, tokens, 4u);
        if (token_count < 0) {
            result_set(&result, WAI_ERR_PARSE, line_number, "too many tokens");
            goto fail;
        }
        if (token_count == 0) {
            continue;
        }

        wai_instruction instr = make_instr(WAI_OP_INVALID);
        wai_error_code status = WAI_OK;
        const char *mnemonic = tokens[0];

        if (strcmp(mnemonic, "mov") == 0 || strcmp(mnemonic, "add") == 0 ||
            strcmp(mnemonic, "sub") == 0 || strcmp(mnemonic, "mul") == 0 ||
            strcmp(mnemonic, "div") == 0 || strcmp(mnemonic, "mod") == 0 ||
            strcmp(mnemonic, "and") == 0 || strcmp(mnemonic, "or") == 0 ||
            strcmp(mnemonic, "xor") == 0 || strcmp(mnemonic, "shl") == 0 ||
            strcmp(mnemonic, "shr") == 0 || strcmp(mnemonic, "cmp") == 0) {
            status = parse_two_operand_alu(mnemonic, tokens, token_count, &instr);
        } else if (strcmp(mnemonic, "load") == 0) {
            status = parse_load(tokens, token_count, &instr);
        } else if (strcmp(mnemonic, "store") == 0) {
            status = parse_store(tokens, token_count, &instr);
        } else if (strcmp(mnemonic, "push") == 0) {
            status = parse_one_register(WAI_OP_PUSH, tokens, token_count, &instr);
        } else if (strcmp(mnemonic, "pop") == 0) {
            status = parse_one_register(WAI_OP_POP, tokens, token_count, &instr);
        } else if (strcmp(mnemonic, "jmp") == 0) {
            status = parse_label_jump(WAI_OP_JMP, tokens, token_count, &labels, out_program, line_number, &instr);
        } else if (strcmp(mnemonic, "call") == 0) {
            status = parse_label_jump(WAI_OP_CALL, tokens, token_count, &labels, out_program, line_number, &instr);
        } else if (strcmp(mnemonic, "je") == 0) {
            status = parse_label_jump(WAI_OP_JE, tokens, token_count, &labels, out_program, line_number, &instr);
        } else if (strcmp(mnemonic, "jne") == 0) {
            status = parse_label_jump(WAI_OP_JNE, tokens, token_count, &labels, out_program, line_number, &instr);
        } else if (strcmp(mnemonic, "jz") == 0 || strcmp(mnemonic, "jnz") == 0) {
            if (token_count != 3 || !is_identifier(tokens[2])) {
                status = WAI_ERR_PARSE;
            } else {
                uint8_t reg = 0;
                if (!parse_register(tokens[1], &reg)) {
                    status = WAI_ERR_PARSE;
                } else {
                    instr = make_instr(strcmp(mnemonic, "jz") == 0 ? WAI_OP_JZ : WAI_OP_JNZ);
                    instr.a = reg;
                    status = pending_add(&labels, out_program->count, tokens[2], line_number);
                }
            }
        } else if (strcmp(mnemonic, "print") == 0) {
            status = parse_one_register(WAI_OP_PRINT, tokens, token_count, &instr);
        } else if (strcmp(mnemonic, "ret") == 0) {
            if (token_count != 1) {
                status = WAI_ERR_PARSE;
            } else {
                instr = make_instr(WAI_OP_RET);
            }
        } else if (strcmp(mnemonic, "nop") == 0) {
            if (token_count != 1) {
                status = WAI_ERR_PARSE;
            } else {
                instr = make_instr(WAI_OP_NOP);
            }
        } else if (strcmp(mnemonic, "not") == 0) {
            status = parse_one_register(WAI_OP_NOT, tokens, token_count, &instr);
        } else if (strcmp(mnemonic, "halt") == 0) {
            if (token_count != 1) {
                status = WAI_ERR_PARSE;
            } else {
                instr = make_instr(WAI_OP_HALT);
            }
        } else {
            status = WAI_ERR_PARSE;
        }

        if (status == WAI_ERR_OOM) {
            result_set(&result, WAI_ERR_OOM, line_number, "out of memory");
            goto fail;
        }
        if (status != WAI_OK) {
            char message[256];
            (void)snprintf(message, sizeof(message), "invalid instruction near '%s'", tokens[0]);
            result_set(&result, WAI_ERR_PARSE, line_number, message);
            goto fail;
        }

        status = wai_program_push(out_program, instr);
        if (status != WAI_OK) {
            result_set(&result, status, line_number, wai_error_string(status));
            goto fail;
        }
    }

    if (resolve_pending(&labels, out_program, &result) != WAI_OK) {
        goto fail;
    }

    free(copy);
    label_table_free(&labels);
    return result;

fail:
    free(copy);
    label_table_free(&labels);
    wai_program_free(out_program);
    return result;
}

wai_assembler_result wai_assemble_file(const char *path, wai_program *out_program) {
    wai_assembler_result result;
    result_set(&result, WAI_OK, 0, "ok");
    char *source = read_entire_file(path);
    if (source == NULL) {
        result_set(&result, WAI_ERR_IO, 0, "could not read source file");
        return result;
    }
    result = wai_assemble_source(source, out_program);
    free(source);
    return result;
}
