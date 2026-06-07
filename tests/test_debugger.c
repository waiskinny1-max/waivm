#include "wai/assembler.h"
#include "wai/debugger.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *source =
        "mov r0, 2\n"
        "add r0, 3\n"
        "push r0\n"
        "store [0], r0\n"
        "print r0\n"
        "halt\n";

    wai_program program;
    wai_assembler_result result = wai_assemble_source(source, &program);
    if (result.error != WAI_OK) {
        return 1;
    }

    FILE *input = tmpfile();
    FILE *output = tmpfile();
    if (input == NULL || output == NULL) {
        wai_program_free(&program);
        if (input != NULL) { fclose(input); }
        if (output != NULL) { fclose(output); }
        return 1;
    }
    (void)fputs("s\ns\ns\nregs\nstack 1\nmem 0 8\nc\nq\n", input);
    rewind(input);
    wai_error_code status = wai_debugger_run(&program, input, output);
    wai_program_free(&program);
    fclose(input);
    if (status != WAI_OK) {
        fclose(output);
        return 1;
    }

    rewind(output);
    char buffer[2048];
    size_t n = fread(buffer, 1u, sizeof(buffer) - 1u, output);
    buffer[n] = '\0';
    fclose(output);
    if (strstr(buffer, "r0=5") == NULL || strstr(buffer, "fff8") == NULL || strstr(buffer, "0000:") == NULL || strstr(buffer, "halted") == NULL) {
        (void)fprintf(stderr, "unexpected debugger output:\n%s\n", buffer);
        return 1;
    }
    return 0;
}
