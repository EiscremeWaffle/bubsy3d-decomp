#include "secondary_controller_measure.h"
#include "heading_timed_records.h"
#include "input_events.h"
#include "secondary_controller_confirm.h"
#include "secondary_controller_entry.h"
#include "sequence_queries.h"

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

extern int32_t func_8003AA1C(void **output);
extern int32_t func_80054B20(
    const ComponentTransformView *component, const ComponentBasisView *basis, int32_t output[3]
);
extern int32_t func_80054AF0(const int16_t *component, int32_t *output);
extern int32_t func_80062D4C(const int32_t *left, const int32_t *right, int32_t *output);
extern int32_t func_8001007C(int32_t left, int32_t right, int32_t *output);

static int32_t add_wrapped(int32_t left, int32_t right) {
    uint32_t value = (uint32_t)left + (uint32_t)right;
    return value <= INT32_MAX ? (int32_t)value : (int32_t)((int64_t)value - 4294967296LL);
}

int32_t secondary_controller_measure(const void *component, SecondaryControllerMeasure *output) {
    void *context;
    int32_t quarter_distance;

    output->eligible = 0;
    func_8002CB2C(5);
    func_8003AA1C(&context);
    func_80054B20((const ComponentTransformView *)component, 0, output->target);
    output->target[1] = add_wrapped(output->target[1], 80);
    func_80054B20((const ComponentTransformView *)&g_secondary_controller_basis, 0, output->position);
    func_80054AF0((const int16_t *)g_secondary_controller_basis.elements, output->direction);
    func_80062D4C(output->position, output->target, &output->distance);
    func_8001007C(output->direction[0], output->distance, &output->scaled[0]);
    func_8001007C(output->direction[1], output->distance, &output->scaled[1]);
    func_8001007C(output->direction[2], output->distance, &output->scaled[2]);
    output->position[0] = add_wrapped(output->position[0], output->scaled[0]);
    output->position[1] = add_wrapped(output->position[1], output->scaled[1]);
    output->position[2] = add_wrapped(output->position[2], output->scaled[2]);
    func_80062D4C(output->position, output->target, &output->adjusted_distance);
    if (g_secondary_controller_flag_506 != 1 &&
        ((ControllerGameContext *)context)->toggle_13 == 0 &&
        ((ControllerGameContext *)context)->unknown_00[0x11] != 2) {
        output->eligible = 1;
    }
    func_80054B20((const ComponentTransformView *)&g_secondary_controller_basis, 0, output->position);
    func_80054B20((const ComponentTransformView *)component, 0, output->target);
    func_80062D4C(output->position, output->target, &output->distance);
    func_8001007C(output->direction[0], 48, &output->scaled[0]);
    func_8001007C(output->direction[1], 48, &output->scaled[1]);
    func_8001007C(output->direction[2], 48, &output->scaled[2]);
    func_8001BC08();
    func_8001C6BC(5, 2, 1);
    quarter_distance = output->distance / 4;
    if (output->distance < 0 && output->distance % 4 != 0) --quarter_distance;
    return output->adjusted_distance < quarter_distance;
}
