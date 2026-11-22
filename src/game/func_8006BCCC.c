#include <stdint.h>

extern volatile uint32_t g_unknown_800A5438;
extern const char g_unknown_80013EA8[];
extern void func_80082E4C(const char *message);
extern void func_8006BB74(int32_t value);
extern void func_8006BD68(int32_t value, int32_t argument);

void func_8006BCCC(uint32_t position, int32_t count) {
    volatile uint32_t remaining = (uint32_t)count << 15;

    while ((int32_t)g_unknown_800A5438 < (int32_t)position) {
        remaining = remaining - 1U;
        if (remaining == 0xFFFFFFFFU) {
            func_80082E4C(g_unknown_80013EA8);
            func_8006BB74(0);
            func_8006BD68(3, 0);
            break;
        }
    }
}
