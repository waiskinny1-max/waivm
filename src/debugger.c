#include "wai/debugger.h"
#include "wai/disassembler.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *trim_line(char *line) {
    while (*line != '\0' && isspace((unsigned char)*line)) {
        line++;
    }
    char *end = line + strlen(line);
    while (end > line && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';
    return line;
}

static void print_help(FILE *out) {
    (void)fprintf(out,
        "commands:\n"
        "  help              show this help\n"
        "  regs              print registers\n"
        "  ip                print instruction pointer\n"
        "  dis               disassemble around ip\n"
        "  step | s          execute one instruction\n"
        "  continue | c      run until halt, error, or breakpoint\n"
        "  break | b <ip>    set breakpoint\n"
        "  clear <ip>        clear breakpoint\n"
        "  quit | q          exit debugger\n");
}

static void print_regs(const wai_vm *vm, FILE *out) {
    for (uint8_t i = 0; i < WAI_REGISTER_COUNT; i++) {
        (void)fprintf(out, "r%u=%lld%s", (unsigned)i, (long long)vm->regs[i], i == 7u ? "\n" : "  ");
    }
    (void)fprintf(out, "ip=%llu zf=%u halted=%u prints=%llu\n",
                  (unsigned long long)vm->ip,
                  (unsigned)vm->zf,
                  (unsigned)vm->halted,
                  (unsigned long long)vm->print_count);
}

static void dis_around(const wai_program *program, const wai_vm *vm, FILE *out) {
    size_t ip = (size_t)vm->ip;
    size_t start = ip > 2u ? ip - 2u : 0u;
    size_t end = ip + 3u;
    if (end > program->count) {
        end = program->count;
    }
    for (size_t i = start; i < end; i++) {
        (void)fprintf(out, "%c ", i == ip ? '>' : ' ');
        (void)wai_disassemble_instruction(&program->code[i], i, out);
    }
}

static int parse_index(const char *text, size_t limit, size_t *out) {
    if (text == NULL || *text == '\0') {
        return 0;
    }
    char *end = NULL;
    unsigned long long value = strtoull(text, &end, 0);
    if (end == text || *end != '\0' || value >= (unsigned long long)limit) {
        return 0;
    }
    *out = (size_t)value;
    return 1;
}

static wai_error_code do_step(wai_vm *vm, FILE *out) {
    uint64_t old_ip = vm->ip;
    wai_error_code status = wai_vm_step(vm);
    if (status != WAI_OK) {
        (void)fprintf(out, "runtime error at ip=%llu: %s\n",
                      (unsigned long long)old_ip,
                      wai_error_string(status));
        return status;
    }
    return WAI_OK;
}

wai_error_code wai_debugger_run(const wai_program *program, FILE *in, FILE *out) {
    if (program == NULL || in == NULL || out == NULL) {
        return WAI_ERR_PARSE;
    }

    unsigned char *breakpoints = calloc(program->count == 0u ? 1u : program->count, sizeof(*breakpoints));
    if (breakpoints == NULL) {
        return WAI_ERR_OOM;
    }

    wai_vm vm;
    wai_vm_init(&vm, program->code, (uint64_t)program->count);
    wai_vm_set_print_stream(&vm, out);

    (void)fprintf(out, "waivm debugger. type 'help' for commands.\n");
    char line[256];
    while (1) {
        (void)fprintf(out, "waidbg> ");
        fflush(out);
        if (fgets(line, sizeof(line), in) == NULL) {
            break;
        }
        char *cmd = trim_line(line);
        if (*cmd == '\0') {
            continue;
        }

        char *arg = cmd;
        while (*arg != '\0' && !isspace((unsigned char)*arg)) {
            arg++;
        }
        if (*arg != '\0') {
            *arg = '\0';
            arg = trim_line(arg + 1);
        } else {
            arg = NULL;
        }

        if (strcmp(cmd, "help") == 0) {
            print_help(out);
        } else if (strcmp(cmd, "regs") == 0) {
            print_regs(&vm, out);
        } else if (strcmp(cmd, "ip") == 0) {
            (void)fprintf(out, "%llu\n", (unsigned long long)vm.ip);
        } else if (strcmp(cmd, "dis") == 0) {
            dis_around(program, &vm, out);
        } else if (strcmp(cmd, "step") == 0 || strcmp(cmd, "s") == 0) {
            if (vm.halted != 0u) {
                (void)fprintf(out, "halted\n");
                continue;
            }
            if (do_step(&vm, out) != WAI_OK) {
                free(breakpoints);
                return vm.error;
            }
            dis_around(program, &vm, out);
        } else if (strcmp(cmd, "continue") == 0 || strcmp(cmd, "c") == 0) {
            while (vm.halted == 0u) {
                if (vm.ip < (uint64_t)program->count && breakpoints[(size_t)vm.ip] != 0u) {
                    (void)fprintf(out, "breakpoint hit at %llu\n", (unsigned long long)vm.ip);
                    break;
                }
                if (do_step(&vm, out) != WAI_OK) {
                    free(breakpoints);
                    return vm.error;
                }
            }
            if (vm.halted != 0u) {
                (void)fprintf(out, "halted\n");
            } else {
                dis_around(program, &vm, out);
            }
        } else if (strcmp(cmd, "break") == 0 || strcmp(cmd, "b") == 0) {
            size_t ip = 0;
            if (!parse_index(arg, program->count, &ip)) {
                (void)fprintf(out, "invalid breakpoint\n");
            } else {
                breakpoints[ip] = 1u;
                (void)fprintf(out, "breakpoint set at %zu\n", ip);
            }
        } else if (strcmp(cmd, "clear") == 0) {
            size_t ip = 0;
            if (!parse_index(arg, program->count, &ip)) {
                (void)fprintf(out, "invalid breakpoint\n");
            } else {
                breakpoints[ip] = 0u;
                (void)fprintf(out, "breakpoint cleared at %zu\n", ip);
            }
        } else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            break;
        } else {
            (void)fprintf(out, "unknown command: %s\n", cmd);
        }
    }

    free(breakpoints);
    return WAI_OK;
}
