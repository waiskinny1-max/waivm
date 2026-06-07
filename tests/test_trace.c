#include "wai/assembler.h"
#include "wai/trace.h"
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
    const char *source =
        "mov r0, 2\n"
        "add r0, 3\n"
        "print r0\n"
        "halt\n";

    wai_program program;
    wai_assembler_result assembled = wai_assemble_source(source, &program);
    failures += require(assembled.error == WAI_OK, "assembler should accept trace source");

    FILE *tmp = tmpfile();
    if (tmp == NULL) {
        wai_program_free(&program);
        return 1;
    }

    failures += require(wai_trace_program(&program, tmp) == WAI_OK, "trace should execute cleanly");
    (void)fflush(tmp);
    rewind(tmp);

    char buffer[4096];
    size_t n = fread(buffer, 1u, sizeof(buffer) - 1u, tmp);
    buffer[n] = '\0';
    fclose(tmp);
    wai_program_free(&program);

    failures += require(strstr(buffer, "trace start") != NULL, "trace output should include start marker");
    failures += require(strstr(buffer, "mov.imm") != NULL, "trace output should include opcode names");
    failures += require(strstr(buffer, "print=5") != NULL, "trace output should include print value");
    failures += require(strstr(buffer, "trace end") != NULL, "trace output should include end marker");

    return failures == 0 ? 0 : 1;
}
