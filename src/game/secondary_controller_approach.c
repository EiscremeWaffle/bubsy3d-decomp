#include "secondary_controller_approach.h"
#include "secondary_controller_entry.h"
#include <string.h>

extern int32_t func_8001007C(int32_t left, int32_t right, int32_t *output);
extern int32_t func_80054C7C(
    ComponentRotationMatrix *component, const ComponentRotationMatrix *basis, const int32_t translation[3]
);
typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;
extern int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t output[3]
);
extern int32_t func_80052BDC(void *component, int32_t mode);

static int32_t add_wrapped(int32_t left, int32_t right) {
    uint32_t value = (uint32_t)left + (uint32_t)right;
    return value <= INT32_MAX ? (int32_t)value : (int32_t)((int64_t)value - 4294967296LL);
}

int32_t secondary_controller_approach(
    const int32_t position[4], const int32_t direction[3],
    SecondaryControllerApproach *output
) {
    MovePreparationComponent *handle = g_extra_model_handle;

    output->component = handle->transform_14;
    memcpy(output->position.words, position, sizeof(output->position.words));
    g_secondary_controller_approach_actor.reset = 0;
    func_8001007C(direction[0], 672, &output->offset[0]);
    func_8001007C(direction[1], 672, &output->offset[1]);
    func_8001007C(direction[2], 672, &output->offset[2]);
    func_80054C7C(output->component, 0, output->position.words);
    output->position.words[0] = add_wrapped(output->position.words[0], output->offset[0]);
    output->position.words[1] = add_wrapped(output->position.words[1], output->offset[1]);
    output->position.words[2] = add_wrapped(output->position.words[2], output->offset[2]);
    func_8005536C(output->component, 0, output->position.words);
    func_80054B20((const ComponentTransformView *)output->component, 0, output->position.words);
    func_80052BDC(g_extra_model_handle, 0);
    g_secondary_controller_approach_actor.info_24 = &g_secondary_controller_approach_move_info;
    g_secondary_controller_approach_actor.handle_18 = g_extra_model_handle;
    func_800222EC(&g_secondary_controller_approach_actor.info_24, &output->position.vector);
    output->iteration = 0;
    return output->position.words[1] >= 0;
}

static void add_step(int32_t position[4], const int32_t step[3]) {
    int32_t x = add_wrapped(position[0], step[0]);
    int32_t y = add_wrapped(position[1], step[1]);
    int32_t z = add_wrapped(position[2], step[2]);

    position[0] = x;
    position[1] = y;
    position[2] = z;
}

void secondary_controller_approach_step(
    int32_t iteration, int32_t base[4], int32_t position[4], const int32_t step[3],
    ComponentRotationMatrix *anchor, ComponentRotationMatrix *component
) {
    if (iteration >= 19) {
        add_step(base, step);
        if (base[1] < 161) {
            base[1] = 160;
        }
        func_80054C7C(anchor, 0, base);
    }
    add_step(position, step);
    func_80054C7C(component, 0, position);
}
