#include "controller_callback_helpers.h"

extern uint8_t func_800283E0(void);
extern void func_8002BC9C(void);
extern void func_8002CA88(int32_t value, int32_t duration);

void func_8002C9D4(int32_t value) {
    if ((uint8_t)value == 0) {
        if (g_unknown_8009F58C != 0) {
            func_8002CA88(5, -1);
        }
        if (func_800283E0() == 1) {
            func_8002BC9C();
            func_8002CA88(15, -1);
        }
    }
    g_unknown_800BDFD2 = (uint8_t)value;
}
