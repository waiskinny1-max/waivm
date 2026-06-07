#include "wai/bytecode.h"
#include <stdio.h>
#include <string.h>

static int require(int condition, const char *message) {
    if (!condition) {
        (void)fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int write_bytes(const char *path, const unsigned char *data, size_t size) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        return 0;
    }
    int ok = fwrite(data, 1u, size, file) == size;
    ok = fclose(file) == 0 && ok;
    return ok;
}

static void write_u16_le(unsigned char *dst, unsigned value) {
    dst[0] = (unsigned char)(value & 0xffu);
    dst[1] = (unsigned char)((value >> 8u) & 0xffu);
}

static void write_u64_le(unsigned char *dst, unsigned long long value) {
    for (size_t i = 0; i < 8u; i++) {
        dst[i] = (unsigned char)((value >> (i * 8u)) & 0xffu);
    }
}

static void make_header(unsigned char *out, unsigned version, unsigned long long count) {
    memset(out, 0, 32u);
    memcpy(out, "WAI0", 4u);
    write_u16_le(out + 4u, version);
    write_u16_le(out + 6u, 32u);
    write_u16_le(out + 8u, 12u);
    write_u16_le(out + 10u, 8u);
    write_u64_le(out + 16u, count);
}

int main(void) {
    int failures = 0;
    wai_program program;

    unsigned char truncated_header[6] = {'W', 'A', 'I', '0', 5, 0};
    failures += require(write_bytes("truncated_header.waibc", truncated_header, sizeof(truncated_header)), "write truncated header fixture");
    failures += require(wai_bytecode_read_file("truncated_header.waibc", &program, NULL) == WAI_ERR_PARSE,
                        "truncated header should fail");
    (void)remove("truncated_header.waibc");

    unsigned char header[32];
    make_header(header, WAI_BYTECODE_VERSION + 1u, 0u);
    failures += require(write_bytes("bad_version.waibc", header, sizeof(header)), "write bad version fixture");
    failures += require(wai_bytecode_read_file("bad_version.waibc", &program, NULL) == WAI_ERR_PARSE,
                        "bad version should fail");
    (void)remove("bad_version.waibc");

    make_header(header, WAI_BYTECODE_VERSION, 1u);
    failures += require(write_bytes("truncated_instruction.waibc", header, sizeof(header)), "write truncated instruction fixture");
    failures += require(wai_bytecode_read_file("truncated_instruction.waibc", &program, NULL) == WAI_ERR_PARSE,
                        "truncated instruction should fail");
    (void)remove("truncated_instruction.waibc");

    unsigned char bad_opcode[44];
    make_header(bad_opcode, WAI_BYTECODE_VERSION, 1u);
    memset(bad_opcode + 32u, 0, 12u);
    bad_opcode[32u] = 0xffu;
    failures += require(write_bytes("bad_opcode_v5.waibc", bad_opcode, sizeof(bad_opcode)), "write bad opcode fixture");
    failures += require(wai_bytecode_read_file("bad_opcode_v5.waibc", &program, NULL) == WAI_ERR_BAD_OPCODE,
                        "bad opcode should fail");
    (void)remove("bad_opcode_v5.waibc");

    unsigned char bad_reg[44];
    make_header(bad_reg, WAI_BYTECODE_VERSION, 1u);
    memset(bad_reg + 32u, 0, 12u);
    bad_reg[32u] = (unsigned char)WAI_OP_PRINT;
    bad_reg[33u] = 9u;
    failures += require(write_bytes("bad_reg.waibc", bad_reg, sizeof(bad_reg)), "write bad register fixture");
    failures += require(wai_bytecode_read_file("bad_reg.waibc", &program, NULL) == WAI_ERR_BAD_REGISTER,
                        "bad register should fail");
    (void)remove("bad_reg.waibc");

    return failures == 0 ? 0 : 1;
}
