#include "cycle_movement_setup.h"
#include "queue_update.h"

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

extern int32_t func_800516F8(void **output);
extern int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t *output
);
extern int32_t func_80054AF0(const int16_t *component, int32_t *output);
extern int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t *input
);

int32_t func_80032A2C(uint8_t mode) {
    void *component;
    uint32_t current[4], direction[4], next[4], other[4];
    (void)mode;

    if (g_unknown_800BE024 == 0) return 0;
    if (g_unknown_800BE020 == 0 || g_unknown_80186108 == 0) {
        g_unknown_80186110 = 0;
        return 0;
    }
    if (g_unknown_80186114 == g_unknown_80186110) {
        g_unknown_80186114 = 0;
        g_unknown_80186108 = 0;
        g_unknown_80186109 = 1;
        if (g_unknown_80186144 != 0) g_unknown_80186144();
        return 0;
    }
    func_800516F8(&component);
    func_80054B20(component, 0, (int32_t *)current);
    func_80054AF0(component, (int32_t *)direction);
    next[0] = current[0] + g_unknown_80186118[0];
    next[1] = current[1] + g_unknown_80186118[1];
    next[2] = current[2] + g_unknown_80186118[2];
    func_80054C7C(component, 0, (const int32_t *)next);
    func_80054B20(g_unknown_800BE528, 0, (int32_t *)other);
    other[0] += g_unknown_80186148[0];
    other[1] += g_unknown_80186148[1];
    other[2] += g_unknown_80186148[2];
    func_80054C7C(g_unknown_800BE528, 0, (const int32_t *)other);
    func_800550A4(component, g_unknown_800BE528);
    g_unknown_80186114++;
    return 1;
}
