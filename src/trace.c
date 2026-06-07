#include "wai/trace.h"
#include "wai/vm.h"
#include <stdio.h>

static void print_register_summary(const wai_vm *vm, FILE *out) {
    (void)fprintf(out,
                  "ip=%llu zf=%u sp=%llu r0=%lld r1=%lld r2=%lld r3=%lld r4=%lld r5=%lld r6=%lld r7=%lld",
                  (unsigned long long)vm->ip,
                  (unsigned)vm->zf,
                  (unsigned long long)vm->sp,
                  (long long)vm->regs[0],
                  (long long)vm->regs[1],
                  (long long)vm->regs[2],
                  (long long)vm->regs[3],
                  (long long)vm->regs[4],
                  (long long)vm->regs[5],
                  (long long)vm->regs[6],
                  (long long)vm->regs[7]);
}

wai_error_code wai_trace_program(const wai_program *program, FILE *out) {
    if (program == NULL || out == NULL) {
        return WAI_ERR_PARSE;
    }

    wai_vm vm;
    wai_vm_init(&vm, program->code, (uint64_t)program->count);
    wai_vm_set_print_stream(&vm, NULL);

    (void)fprintf(out, "trace start: %llu instructions\n", (unsigned long long)program->count);

    uint64_t steps = 0;
    while (vm.halted == 0u) {
        if (steps >= WAI_TRACE_MAX_STEPS) {
            (void)fprintf(out, "trace stopped: step limit %u reached\n", (unsigned)WAI_TRACE_MAX_STEPS);
            return WAI_ERR_UNSUPPORTED;
        }
        if (vm.ip >= vm.code_count) {
            (void)fprintf(out, "trace error before step %llu: ip %llu out of bounds\n",
                          (unsigned long long)steps,
                          (unsigned long long)vm.ip);
            return WAI_ERR_IP_OUT_OF_BOUNDS;
        }

        uint64_t before_ip = vm.ip;
        uint64_t before_print_count = vm.print_count;
        const wai_instruction *ins = &program->code[before_ip];
        (void)fprintf(out, "%04llu  %-8s a=%u b=%u imm=%lld\n",
                      (unsigned long long)before_ip,
                      wai_opcode_name(ins->opcode),
                      (unsigned)ins->a,
                      (unsigned)ins->b,
                      (long long)ins->imm);

        wai_error_code status = wai_vm_step(&vm);
        (void)fprintf(out, "      => ");
        print_register_summary(&vm, out);
        if (vm.print_count != before_print_count) {
            (void)fprintf(out, " print=%lld", (long long)vm.last_print);
        }
        (void)fprintf(out, "\n");
        if (status != WAI_OK) {
            (void)fprintf(out, "trace error after step %llu: %s\n",
                          (unsigned long long)steps,
                          wai_error_string(status));
            return status;
        }
        steps += 1u;
    }

    (void)fprintf(out, "trace end: %llu steps, %llu prints\n",
                  (unsigned long long)steps,
                  (unsigned long long)vm.print_count);
    return WAI_OK;
}
