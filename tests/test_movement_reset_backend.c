#include "component_rotation.h"
#include "level_records.h"
#include "movement_reset.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct ComponentTransformView;
struct ComponentBasisView;

static MovementResetObject *active_object;
static MovementResetComponent *active_component;
static ComponentRotationMatrix original_transform;
static ComponentRotationMatrix replacement_transform;
static const ComponentRotationMatrix expected_rotation = {
    {0, 1, 0x7FFF, 0x8000, 0xFFFF, 5, 6, 7, 8},
    0x1234,
    {INT32_MIN, 0, INT32_MAX}
};
static int32_t initial_index;
static unsigned int callback_count;
static int mutate_callbacks;

int32_t func_80028B5C(int32_t index, GameLevelPosition *output) {
    assert(callback_count++ == 0 && index == initial_index);
    output->words[0] = INT32_MIN;
    output->words[1] = -7;
    output->words[2] = INT32_MAX;
    output->words[3] = 123;
    if (mutate_callbacks) {
        active_component->transform_14 = &replacement_transform;
        active_object->record_id_0C = -2;
    }
    return 0;
}

int32_t func_80054C7C(
    struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    const int32_t input[3]
) {
    void *expected_transform = mutate_callbacks ? &replacement_transform : &original_transform;
    assert(callback_count++ == 1 && (void *)transform == expected_transform && basis == 0);
    assert(input[0] == INT32_MIN && input[1] == -7 && input[2] == INT32_MAX);
    if (mutate_callbacks) active_object->record_id_0C = -3;
    return 0x18;
}

int32_t func_800289E8(int32_t index, ComponentRotationMatrix *output) {
    assert(callback_count++ == 2 && index == (mutate_callbacks ? -3 : initial_index));
    *output = expected_rotation;
    if (mutate_callbacks) {
        active_component->transform_14 = &original_transform;
        active_object->record_id_0C = -4;
    }
    return 0;
}

int32_t func_800549E8(ComponentRotationMatrix *transform, const ComponentRotationMatrix *rotation) {
    ComponentRotationMatrix *expected_transform = mutate_callbacks ? &replacement_transform : &original_transform;
    assert(callback_count++ == 3 && transform == expected_transform);
    assert(memcmp(rotation, &expected_rotation, sizeof(expected_rotation)) == 0);
    if (mutate_callbacks) active_object->record_id_0C = -5;
    return 0x18;
}

int32_t func_800288D8(int32_t index) {
    assert(callback_count++ == 4 && index == (mutate_callbacks ? -5 : initial_index));
    if (mutate_callbacks) {
        active_component->flags_04 ^= 0x5Au;
        active_component->field_10 = 0xDEADBEEFu;
    }
    return 0;
}

int main(void) {
    static const int32_t indices[] = {0, 7, -1, INT32_MIN, INT32_MAX};
    MovementResetObject object;
    MovementResetComponent component;
    unsigned int flags, index, mutation;

#if UINTPTR_MAX == UINT32_MAX
    _Static_assert(offsetof(MovementResetComponent, flags_04) == 0x04, "component flag offset");
    _Static_assert(offsetof(MovementResetComponent, field_10) == 0x10, "component state offset");
    _Static_assert(offsetof(MovementResetComponent, transform_14) == 0x14, "component transform offset");
    _Static_assert(offsetof(MovementResetObject, record_id_0C) == 0x0C, "record index offset");
#endif
    active_object = &object;
    active_component = &component;
    for (index = 0; index < sizeof(indices) / sizeof(indices[0]); index++) {
        for (mutation = 0; mutation < 2; mutation++) {
            for (flags = 0; flags < 256; flags++) {
                memset(&object, 0xA5, sizeof(object));
                memset(&component, 0xA5, sizeof(component));
                object.record_id_0C = indices[index];
                component.flags_04 = (uint8_t)flags;
                component.field_10 = 0x12345678u;
                component.transform_14 = &original_transform;
                initial_index = indices[index];
                mutate_callbacks = (int)mutation;
                callback_count = 0;

                func_80028600(&object, &component);

                assert(callback_count == 5);
                assert(component.field_10 == 0);
                assert(component.flags_04 == ((flags ^ (mutation ? 0x5Au : 0)) & 0xDFu));
                assert(component.transform_14 == &original_transform);
                assert(object.record_id_0C == (mutation ? -5 : initial_index));
                for (unsigned int byte = 0; byte < sizeof(component.unknown_00); byte++) {
                    assert(component.unknown_00[byte] == 0xA5);
                }
                for (unsigned int byte = 0; byte < sizeof(component.unknown_05); byte++) {
                    assert(component.unknown_05[byte] == 0xA5);
                }
            }
        }
    }
    puts("Movement reset backend: callback order, live indices, transform sampling and flags passed");
    return 0;
}
