#include "queue_update.h"

extern void func_8003AAF0(void);
extern void func_800516F8(void **output);
extern void func_800334C4(void);
extern void func_80033CF4(void);
extern void func_8005476C(void *source, void *destination);
extern void func_80032EFC(void);
extern void func_80032DF4(void);

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
int32_t func_80032C8C(mode_input)
register int32_t mode_input __asm__("$16");
{
#else
int32_t func_80032C8C(int32_t mode_input) {
#endif
    uint8_t mode = (uint8_t)mode_input;
    void *reference_component;

    if (g_unknown_800BE024 == 0) {
        return 1;
    }

    func_8003AAF0();
    func_800516F8(&reference_component);
    if (mode == 0) {
        if (g_unknown_80186174 == 1) {
            func_800334C4();
        }
    }
    func_80033CF4();
    func_8005476C(reference_component, &g_unknown_80186154);
    if (mode == 0) {
        if (g_unknown_800BE023 == 1) {
            func_80032EFC();
        }
    }
    func_80032DF4();
    return 1;
}
