#ifndef WAI_VERIFIER_H
#define WAI_VERIFIER_H

#include <stddef.h>
#include "wai/bytecode.h"
#include "wai/error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wai_verify_report {
    wai_error_code error;
    size_t ip;
    char message[256];
} wai_verify_report;

wai_error_code wai_verify_program(const wai_program *program, wai_verify_report *out_report);

#ifdef __cplusplus
}
#endif

#endif
