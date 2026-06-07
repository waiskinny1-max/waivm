#include "wai/error.h"

const char *wai_error_string(wai_error_code code) {
    switch (code) {
        case WAI_OK: return "ok";
        case WAI_ERR_IO: return "I/O error";
        case WAI_ERR_PARSE: return "parse error";
        case WAI_ERR_OOM: return "out of memory";
        case WAI_ERR_BAD_OPCODE: return "bad opcode";
        case WAI_ERR_IP_OUT_OF_BOUNDS: return "instruction pointer out of bounds";
        case WAI_ERR_BAD_REGISTER: return "bad register";
        case WAI_ERR_DIV_ZERO: return "division by zero";
        case WAI_ERR_BAD_JUMP: return "bad jump target";
        case WAI_ERR_UNSUPPORTED: return "unsupported feature";
        case WAI_ERR_MEMORY_OOB: return "memory access out of bounds";
        case WAI_ERR_STACK_OVERFLOW: return "stack overflow";
        case WAI_ERR_STACK_UNDERFLOW: return "stack underflow";
        default: return "unknown error";
    }
}
