#include <stdint.h>

typedef void (*GameAssertionHandler)(void);

extern const char g_assertion_format[];
extern const char g_assertion_halted[];
extern uint8_t g_assertion_print_enabled;
extern GameAssertionHandler g_assertion_handler;
extern void func_80082F0C(const char *format, ...);
extern void func_80082E3C(int32_t value);

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register const char *saved_condition __asm__("$16") = condition;
    register const char *saved_source __asm__("$17") = source;
    register uint32_t saved_line __asm__("$18") = line;
#else
    const char *saved_condition = condition;
    const char *saved_source = source;
    uint32_t saved_line = line;
#endif

    if ((uint8_t)failed != 0) {
        func_80082F0C(g_assertion_format);
        func_80082F0C(g_assertion_halted);
        if (g_assertion_handler != 0) {
            g_assertion_handler();
        } else {
            func_80082E3C(-1);
        }
    }

    if (g_assertion_print_enabled == 1) {
        func_80082F0C(g_assertion_format, saved_condition, saved_source, saved_line);
    }
}
