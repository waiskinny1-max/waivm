#ifndef WAI_ERROR_H
#define WAI_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum wai_error_code {
    WAI_OK = 0,
    WAI_ERR_IO = 1,
    WAI_ERR_PARSE = 2,
    WAI_ERR_OOM = 3,
    WAI_ERR_BAD_OPCODE = 4,
    WAI_ERR_IP_OUT_OF_BOUNDS = 5,
    WAI_ERR_BAD_REGISTER = 6,
    WAI_ERR_DIV_ZERO = 7,
    WAI_ERR_BAD_JUMP = 8,
    WAI_ERR_UNSUPPORTED = 9
} wai_error_code;

const char *wai_error_string(wai_error_code code);

#ifdef __cplusplus
}
#endif

#endif
