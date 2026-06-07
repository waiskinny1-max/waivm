#include "wai/assembler.h"
#include "wai/disassembler.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *source =
        "mov r0, 2\n"
        "add r0, 3\n"
        "mod r0, 5\n"
        "xor r0, 7\n"
        "shl r0, 1\n"
        "store [8], r0\n"
        "load r1, [8]\n"
        "print r1\n"
        "halt\n";

    wai_program program;
    wai_assembler_result result = wai_assemble_source(source, &program);
    if (result.error != WAI_OK) {
        return 1;
    }

    FILE *tmp = tmpfile();
    if (tmp == NULL) {
        wai_program_free(&program);
        return 1;
    }
    if (wai_disassemble_program(&program, tmp) != WAI_OK) {
        fclose(tmp);
        wai_program_free(&program);
        return 1;
    }
    rewind(tmp);
    char buffer[512];
    size_t n = fread(buffer, 1u, sizeof(buffer) - 1u, tmp);
    buffer[n] = '\0';
    fclose(tmp);
    wai_program_free(&program);

    if (strstr(buffer, "0000  mov r0, 2") == NULL) {
        (void)fprintf(stderr, "missing mov disassembly\n%s\n", buffer);
        return 1;
    }
    if (strstr(buffer, "0002  mod r0, 5") == NULL) {
        (void)fprintf(stderr, "missing mod disassembly\n%s\n", buffer);
        return 1;
    }
    if (strstr(buffer, "0003  xor r0, 7") == NULL) {
        (void)fprintf(stderr, "missing xor disassembly\n%s\n", buffer);
        return 1;
    }
    if (strstr(buffer, "0004  shl r0, 1") == NULL) {
        (void)fprintf(stderr, "missing shl disassembly\n%s\n", buffer);
        return 1;
    }
    if (strstr(buffer, "0005  store [8], r0") == NULL) {
        (void)fprintf(stderr, "missing store disassembly\n%s\n", buffer);
        return 1;
    }
    if (strstr(buffer, "0006  load r1, [8]") == NULL) {
        (void)fprintf(stderr, "missing load disassembly\n%s\n", buffer);
        return 1;
    }
    if (strstr(buffer, "0007  print r1") == NULL) {
        (void)fprintf(stderr, "missing print disassembly\n%s\n", buffer);
        return 1;
    }
    return 0;
}
