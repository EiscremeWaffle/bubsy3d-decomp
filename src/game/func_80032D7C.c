#include "queue_update.h"

extern int32_t func_800516F8(void **output);

int32_t func_80032D7C(void) {
    void *reference_component;

    if (g_unknown_800BE024 == 0) {
        return 1;
    }

    func_800516F8(&reference_component);
    func_80054A6C(reference_component, &g_unknown_80186154);
    func_8003ABE8();
    return 1;
}
