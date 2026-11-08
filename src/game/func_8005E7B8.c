#include "sequence_limit.h"

extern int32_t func_8008E1DC(int32_t first_value, int32_t second_value);

int32_t func_8005E7B8(int32_t first_value, int32_t second_value) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t narrowed_first __asm__("$17") = (int16_t)first_value;
    register int32_t narrowed_second __asm__("$16") = (int16_t)second_value;
#else
    int32_t narrowed_first = (int16_t)first_value;
    int32_t narrowed_second = (int16_t)second_value;
#endif
    int32_t result = func_8005D5A8(narrowed_first, narrowed_second);

    if (result != 0) {
        return result;
    }
    func_8008E1DC(narrowed_first, narrowed_second);
    return 0;
}