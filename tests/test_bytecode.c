#include "wai/assembler.h"
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

int main(void) {
    int failures = 0;
    failures += require(sizeof(wai_instruction) == 12u, "instruction encoding must be 12 bytes");

    const char *source =
        "mov r0, 10\n"
        "mov r1, 0\n"
        "loop:\n"
        "add r1, r0\n"
        "sub r0, 1\n"
        "jnz r0, loop\n"
        "print r1\n"
        "halt\n";

    wai_program program;
    wai_assembler_result assembled = wai_assemble_source(source, &program);
    failures += require(assembled.error == WAI_OK, "assembler should produce program");

    const char *path = "test_sum.waibc";
    failures += require(wai_bytecode_write_file(path, &program) == WAI_OK, "bytecode write should succeed");

    wai_program loaded;
    wai_bytecode_info info;
    failures += require(wai_bytecode_read_file(path, &loaded, &info) == WAI_OK, "bytecode read should succeed");
    failures += require(info.version == WAI_BYTECODE_VERSION, "version should match");
    failures += require(info.code_count == (uint64_t)program.count, "code count should match");
    failures += require(loaded.count == program.count, "loaded count should match");
    if (loaded.count == program.count && loaded.count > 0u) {
        failures += require(memcmp(loaded.code, program.code, loaded.count * sizeof(wai_instruction)) == 0, "loaded instructions should match");
    }

    wai_program_free(&loaded);
    wai_program_free(&program);
    (void)remove(path);

    FILE *bad = fopen("bad_magic.waibc", "wb");
    if (bad == NULL) {
        return 1;
    }
    (void)fwrite("NOPE", 1u, 4u, bad);
    fclose(bad);
    wai_program bad_program;
    failures += require(wai_bytecode_read_file("bad_magic.waibc", &bad_program, NULL) == WAI_ERR_PARSE,
                        "bad magic should fail cleanly");
    (void)remove("bad_magic.waibc");

    uint8_t bad_opcode_file[44] = {
        'W', 'A', 'I', '0',
        WAI_BYTECODE_VERSION, 0, 32, 0, 12, 0, 8, 0,
        0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        255, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    bad = fopen("bad_opcode.waibc", "wb");
    if (bad == NULL) {
        return 1;
    }
    (void)fwrite(bad_opcode_file, 1u, sizeof(bad_opcode_file), bad);
    fclose(bad);
    failures += require(wai_bytecode_read_file("bad_opcode.waibc", &bad_program, NULL) == WAI_ERR_BAD_OPCODE,
                        "bad opcode should fail cleanly");
    (void)remove("bad_opcode.waibc");

    return failures == 0 ? 0 : 1;
}
