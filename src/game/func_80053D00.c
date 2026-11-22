#include <stdint.h>

extern int32_t func_8005385C(const int32_t *input, int32_t *output);
extern int32_t func_80053A08(const int32_t *input, int32_t scale, int32_t *output);
extern int32_t func_80053B70(const int32_t *input, int32_t scale, int32_t *output);

int32_t func_80053D00(const int32_t *input, int32_t scale, int32_t *output) {
    int32_t length;
    int32_t scaled[4];

    func_8005385C(input, &length);
    func_80053A08(input, scale, scaled);
    func_80053B70(scaled, length, output);
    return 0;
}
