#include <stdint.h>

extern int32_t func_80076554(int32_t value);
extern int32_t func_800764D0(int32_t value);

int32_t func_8005359C(int32_t value, int32_t *output) {
    *output = func_80076554(value);
    return 0;
}

int32_t func_800535CC(int32_t value, int32_t *output) {
    *output = func_800764D0(value);
    return 0;
}
