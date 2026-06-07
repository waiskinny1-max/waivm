#include "wai/assembler.h"
#include "wai/vm.h"
#include <stdio.h>

static int run_source_last_print(const char *source, wai_value expected) {
    wai_program program;
    wai_assembler_result result = wai_assemble_source(source, &program);
    if (result.error != WAI_OK) {
        (void)fprintf(stderr, "assemble failed: %s\n", result.message);
        return 1;
    }

    wai_vm vm;
    wai_vm_init(&vm, program.code, (uint64_t)program.count);
    wai_vm_set_print_stream(&vm, NULL);
    wai_error_code error = wai_vm_execute(&vm);
    wai_program_free(&program);

    if (error != WAI_OK) {
        (void)fprintf(stderr, "runtime failed: %s\n", wai_error_string(error));
        return 1;
    }
    if (vm.print_count != 1u || vm.last_print != expected) {
        (void)fprintf(stderr, "expected print %lld, got %lld count=%llu\n",
                      (long long)expected,
                      (long long)vm.last_print,
                      (unsigned long long)vm.print_count);
        return 1;
    }
    return 0;
}

int main(void) {
    int failures = 0;

    failures += run_source_last_print(
        "mov r0, 10\n"
        "mov r1, 0\n"
        "loop:\n"
        "add r1, r0\n"
        "sub r0, 1\n"
        "jnz r0, loop\n"
        "print r1\n"
        "halt\n",
        55);

    failures += run_source_last_print(
        "mov r0, 5\n"
        "mov r1, 1\n"
        "loop:\n"
        "mul r1, r0\n"
        "sub r0, 1\n"
        "jnz r0, loop\n"
        "print r1\n"
        "halt\n",
        120);


    failures += run_source_last_print(
        "mov r0, 42\n"
        "store [0], r0\n"
        "mov r1, 0\n"
        "load r2, [r1]\n"
        "print r2\n"
        "halt\n",
        42);

    failures += run_source_last_print(
        "mov r0, 7\n"
        "push r0\n"
        "mov r0, 0\n"
        "pop r1\n"
        "print r1\n"
        "halt\n",
        7);

    failures += run_source_last_print(
        "mov r0, 21\n"
        "call double\n"
        "print r0\n"
        "halt\n"
        "double:\n"
        "mul r0, 2\n"
        "ret\n",
        42);

    failures += run_source_last_print(
        "mov r0, 10\n"
        "cmp r0, 10\n"
        "je equal\n"
        "mov r1, 0\n"
        "jmp done\n"
        "equal:\n"
        "mov r1, 1\n"
        "done:\n"
        "print r1\n"
        "halt\n",
        1);


    failures += run_source_last_print(
        "mov r0, 6\n"
        "mov r1, 3\n"
        "and r0, r1\n"
        "shl r0, 4\n"
        "xor r0, 7\n"
        "mod r0, 10\n"
        "print r0\n"
        "halt\n",
        9);

    failures += run_source_last_print(
        "mov r0, 8\n"
        "shr r0, 1\n"
        "or r0, 1\n"
        "print r0\n"
        "halt\n",
        5);

    wai_instruction bad_div[] = {
        {.opcode = WAI_OP_MOV_IMM, .a = 0, .b = 0, .c = 0, .imm = 7},
        {.opcode = WAI_OP_DIV_IMM, .a = 0, .b = 0, .c = 0, .imm = 0},
        {.opcode = WAI_OP_HALT, .a = 0, .b = 0, .c = 0, .imm = 0},
    };
    wai_vm vm;
    wai_vm_init(&vm, bad_div, 3u);
    wai_vm_set_print_stream(&vm, NULL);
    wai_error_code error = wai_vm_execute(&vm);
    if (error != WAI_ERR_DIV_ZERO) {
        (void)fprintf(stderr, "expected division by zero error\n");
        failures += 1;
    }

    wai_instruction bad_shift[] = {
        {.opcode = WAI_OP_MOV_IMM, .a = 0, .b = 0, .c = 0, .imm = 7},
        {.opcode = WAI_OP_SHL_IMM, .a = 0, .b = 0, .c = 0, .imm = 64},
        {.opcode = WAI_OP_HALT, .a = 0, .b = 0, .c = 0, .imm = 0},
    };
    wai_vm_init(&vm, bad_shift, 3u);
    wai_vm_set_print_stream(&vm, NULL);
    error = wai_vm_execute(&vm);
    if (error != WAI_ERR_BAD_SHIFT) {
        (void)fprintf(stderr, "expected bad shift error, got %s\n", wai_error_string(error));
        failures += 1;
    }

    return failures == 0 ? 0 : 1;
}
