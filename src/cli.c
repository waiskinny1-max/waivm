#include "wai/assembler.h"
#include "wai/bytecode.h"
#include "wai/debugger.h"
#include "wai/disassembler.h"
#include "wai/vm.h"
#include <stdio.h>
#include <string.h>

typedef enum cli_status {
    CLI_OK = 0,
    CLI_ERR = 1
} cli_status;

static void print_usage(FILE *out) {
    (void)fprintf(out,
        "waivm v2\n"
        "\n"
        "Usage:\n"
        "  waivm run <file.wai|file.waibc>\n"
        "  waivm asm <input.wai> -o <output.waibc>\n"
        "  waivm dis <file.wai|file.waibc>\n"
        "  waivm debug <file.wai|file.waibc>\n"
        "  waivm info <file.waibc>\n"
        "  waivm help\n");
}

static int load_program_auto(const char *path, wai_program *program, wai_bytecode_info *info) {
    if (wai_file_has_waibc_magic(path)) {
        wai_error_code status = wai_bytecode_read_file(path, program, info);
        if (status != WAI_OK) {
            (void)fprintf(stderr, "bytecode error: %s\n", wai_error_string(status));
            return CLI_ERR;
        }
        return CLI_OK;
    }

    wai_assembler_result assembly = wai_assemble_file(path, program);
    if (assembly.error != WAI_OK) {
        if (assembly.line > 0) {
            (void)fprintf(stderr, "assemble error:%d: %s\n", assembly.line, assembly.message);
        } else {
            (void)fprintf(stderr, "assemble error: %s\n", assembly.message);
        }
        return CLI_ERR;
    }
    if (info != NULL) {
        memset(info, 0, sizeof(*info));
    }
    return CLI_OK;
}

static int command_run(const char *path) {
    wai_program program;
    wai_bytecode_info info;
    if (load_program_auto(path, &program, &info) != CLI_OK) {
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

static int command_asm(int argc, char **argv) {
    if (argc != 5 || strcmp(argv[3], "-o") != 0) {
        (void)fprintf(stderr, "error: asm expects: waivm asm <input.wai> -o <output.waibc>\n");
        return CLI_ERR;
    }

    wai_program program;
    wai_assembler_result assembly = wai_assemble_file(argv[2], &program);
    if (assembly.error != WAI_OK) {
        if (assembly.line > 0) {
            (void)fprintf(stderr, "assemble error:%d: %s\n", assembly.line, assembly.message);
        } else {
            (void)fprintf(stderr, "assemble error: %s\n", assembly.message);
        }
        return CLI_ERR;
    }

    wai_error_code status = wai_bytecode_write_file(argv[4], &program);
    wai_program_free(&program);
    if (status != WAI_OK) {
        (void)fprintf(stderr, "bytecode write error: %s\n", wai_error_string(status));
        return CLI_ERR;
    }
    return CLI_OK;
}

static int command_dis(const char *path) {
    wai_program program;
    wai_bytecode_info info;
    if (load_program_auto(path, &program, &info) != CLI_OK) {
        return CLI_ERR;
    }
    wai_error_code status = wai_disassemble_program(&program, stdout);
    wai_program_free(&program);
    if (status != WAI_OK) {
        (void)fprintf(stderr, "disassemble error: %s\n", wai_error_string(status));
        return CLI_ERR;
    }
    return CLI_OK;
}

static int command_debug(const char *path) {
    wai_program program;
    wai_bytecode_info info;
    if (load_program_auto(path, &program, &info) != CLI_OK) {
        return CLI_ERR;
    }
    wai_error_code status = wai_debugger_run(&program, stdin, stdout);
    wai_program_free(&program);
    if (status != WAI_OK) {
        (void)fprintf(stderr, "debugger error: %s\n", wai_error_string(status));
        return CLI_ERR;
    }
    return CLI_OK;
}

static int command_info(const char *path) {
    wai_bytecode_info info;
    wai_error_code status = wai_bytecode_read_info(path, &info);
    if (status != WAI_OK) {
        (void)fprintf(stderr, "info error: %s\n", wai_error_string(status));
        return CLI_ERR;
    }
    (void)printf("magic: WAI0\n");
    (void)printf("version: %u\n", (unsigned)info.version);
    (void)printf("header_size: %u\n", (unsigned)info.header_size);
    (void)printf("instruction_size: %u\n", (unsigned)info.instruction_size);
    (void)printf("register_count: %u\n", (unsigned)info.register_count);
    (void)printf("flags: 0x%08x\n", (unsigned)info.flags);
    (void)printf("code_count: %llu\n", (unsigned long long)info.code_count);
    (void)printf("data_size: %llu\n", (unsigned long long)info.data_size);
    return CLI_OK;
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
        return command_run(argv[2]);
    }

    if (strcmp(argv[1], "asm") == 0) {
        return command_asm(argc, argv);
    }

    if (strcmp(argv[1], "dis") == 0) {
        if (argc != 3) {
            (void)fprintf(stderr, "error: dis expects exactly one input file\n");
            return CLI_ERR;
        }
        return command_dis(argv[2]);
    }

    if (strcmp(argv[1], "debug") == 0) {
        if (argc != 3) {
            (void)fprintf(stderr, "error: debug expects exactly one input file\n");
            return CLI_ERR;
        }
        return command_debug(argv[2]);
    }

    if (strcmp(argv[1], "info") == 0) {
        if (argc != 3) {
            (void)fprintf(stderr, "error: info expects exactly one bytecode file\n");
            return CLI_ERR;
        }
        return command_info(argv[2]);
    }

    (void)fprintf(stderr, "error: unknown command '%s'\n", argv[1]);
    print_usage(stderr);
    return CLI_ERR;
}
