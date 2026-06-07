#include "wai/assembler.h"
#include "wai/vm.h"
#include <stdio.h>
#include <string.h>

typedef enum cli_status {
    CLI_OK = 0,
    CLI_ERR = 1
} cli_status;

static void print_usage(FILE *out) {
    (void)fprintf(out,
        "waivm v0.1\n"
        "\n"
        "Usage:\n"
        "  waivm run <file.wai>\n"
        "  waivm help\n"
        "\n"
        "Reserved for later versions:\n"
        "  waivm asm <input.wai> -o <output.waibc>\n"
        "  waivm dis <file.waibc>\n"
        "  waivm debug <file>\n"
        "  waivm info <file.waibc>\n");
}

int wai_cli_main(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(stdout);
        return CLI_OK;
    }

    if (strcmp(argv[1], "run") == 0) {
        if (argc != 3) {
            (void)fprintf(stderr, "error: run expects exactly one input file\n");
            print_usage(stderr);
            return CLI_ERR;
        }

        const char *path = argv[2];
        wai_program program;
        wai_assembler_result assembly = wai_assemble_file(path, &program);
        if (assembly.error != WAI_OK) {
            if (assembly.line > 0) {
                (void)fprintf(stderr, "assemble error:%d: %s\n", assembly.line, assembly.message);
            } else {
                (void)fprintf(stderr, "assemble error: %s\n", assembly.message);
            }
            return CLI_ERR;
        }

        wai_vm vm;
        wai_vm_init(&vm, program.code, (uint64_t)program.count);
        wai_error_code execution = wai_vm_execute(&vm);
        wai_program_free(&program);
        if (execution != WAI_OK) {
            (void)fprintf(stderr, "runtime error at ip=%llu: %s\n",
                          (unsigned long long)vm.ip,
                          wai_error_string(execution));
            return CLI_ERR;
        }
        return CLI_OK;
    }

    if (strcmp(argv[1], "asm") == 0 || strcmp(argv[1], "dis") == 0 ||
        strcmp(argv[1], "debug") == 0 || strcmp(argv[1], "info") == 0) {
        (void)fprintf(stderr, "error: '%s' is reserved but not implemented in v0.1\n", argv[1]);
        return CLI_ERR;
    }

    (void)fprintf(stderr, "error: unknown command '%s'\n", argv[1]);
    print_usage(stderr);
    return CLI_ERR;
}
