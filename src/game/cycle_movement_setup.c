#include "cycle_movement_setup.h"
#include "queue_update.h"

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

extern int32_t func_800516F8(void **output);
extern int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t *output
);
extern int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t *input
);

static int32_t signed_word(uint32_t value) {
    return value <= INT32_MAX ? (int32_t)value : (int32_t)((int64_t)value - 4294967296LL);
}

static int32_t movement_step(uint32_t destination, uint32_t current, int32_t duration) {
    int32_t delta = signed_word(destination - current);
    int32_t magnitude = delta < 0 ? signed_word(0u - (uint32_t)delta) : delta;

    if (delta == 0) return 0;
    if (magnitude <= duration) return delta < 0 ? -1 : 1;
    return delta / duration;
}

void cycle_movement_setup(
    const GroundedTransformVector *direction, int32_t distance, uint32_t height,
    int32_t duration, void (*callback)(void)
) {
    cycle_movement_setup_with_target(direction, distance, height, duration, callback, 0);
}

void cycle_movement_setup_with_target(
    const GroundedTransformVector *direction, int32_t distance, uint32_t height,
    int32_t duration, void (*callback)(void), const GroundedTransformVector *target
) {
    GroundedTransformVector current, destination, saved_target;
    void *component;
    int i;

    if (g_unknown_800BE024 == 0 || g_unknown_800BE020 == 0 || duration <= 0) return;
    if (g_unknown_80186108 == 1 && g_unknown_80186109 == 0) return;

    func_800516F8(&component);
    func_80054B20(component, 0, current.values);
    if (target != 0) {
        func_80054B20(g_unknown_800BE528, 0, saved_target.values);
        func_80054C7C(g_unknown_800BE528, 0, target->values);
    }
    func_8002E858(direction, distance, height, &destination);
    if (target != 0) {
        func_80054C7C(g_unknown_800BE528, 0, saved_target.values);
    }
    for (i = 0; i < 3; i++) {
        g_unknown_80186118[i] = (uint32_t)movement_step(
            (uint32_t)destination.values[i], (uint32_t)current.values[i], duration
        );
    }
    if (target != 0) {
        func_80054B20(g_unknown_800BE528, 0, saved_target.values);
        for (i = 0; i < 3; i++) {
            g_unknown_80186148[i] = (uint32_t)movement_step(
                (uint32_t)target->values[i], (uint32_t)saved_target.values[i], duration
            );
        }
    } else {
        for (i = 0; i < 3; i++) g_unknown_80186148[i] = 0;
    }
    g_unknown_80186110 = (uint32_t)duration;
    g_unknown_80186114 = 0;
    g_unknown_80186108 = 1;
    g_unknown_80186109 = 0;
    g_unknown_80186144 = callback;
    g_unknown_800BE020 = 1;
    g_unknown_800BE022 = 0;
}
