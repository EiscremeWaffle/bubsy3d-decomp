#include "component_fields.h"
#include "grounded_case37.h"
#include "level_records.h"
#include "movement_reset.h"
#include "proximity_list.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

uint8_t func_80022604(const void *actor, uint8_t selector);
uint8_t func_80022674(const void *actor, uint8_t selector);
void func_800691AC(uint16_t value);

int32_t g_level_record_count;
GameLevelRecord *volatile g_level_records;
GameAuxiliaryObjectSlot g_auxiliary_object_slots[8];
ProximityObject *g_reference_object;
GroundedCase37ObjectView *g_grounded_case37_object;
uint16_t g_l0_gp_6C2;

static MovementResetObject *reset_object;
static MovementResetComponent *reset_calls[2];
static MovementResetComponent *reset_replacement;
static unsigned int reset_count;
static int mutate_reset;

void func_80028600(MovementResetObject *object, MovementResetComponent *component) {
    assert(object == reset_object && reset_count < 2);
    assert(object->field_2C == 0 && object->field_10 == 0);
    assert(object->field_04 == 0 && object->field_08 == 0 && object->field_14 == 0);
    reset_calls[reset_count++] = component;
    if (mutate_reset && reset_count == 1) {
        object->component_24 = reset_replacement;
        object->field_18 = 0x87654321u;
    }
}

static uint32_t signed_halfword_bits(uint32_t value) {
    value &= 0xFFFFu;
    return value & 0x8000u ? value | 0xFFFF0000u : value;
}

static int32_t signed_word(uint32_t bits) {
    int32_t value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static void check_packing(uint32_t first, uint32_t second, uint32_t third, uint32_t flags) {
    uint32_t component[11];
    uint32_t before[11];
    uint32_t expected = flags | (second << 16) | (signed_halfword_bits(first) << 8) |
        signed_halfword_bits(third);
    uint32_t result;
    unsigned int index;

    for (index = 0; index < 11; index++) {
        component[index] = 0x12345678u + index;
    }
    memcpy(before, component, sizeof(component));
    result = (uint32_t)func_800532CC(
        component, signed_word(first), signed_word(second), signed_word(third), flags
    );
    assert(result == expected && component[9] == expected);
    for (index = 0; index < 11; index++) {
        if (index != 9) {
            assert(component[index] == before[index]);
        }
    }
}

static void test_component_helpers(void) {
    static const uint32_t values[] = {
        0, 1, 0x7Fu, 0x80u, 0xFFu, 0x100u, 0x7FFFu,
        0x8000u, 0xFFFFu, 0x10000u, 0x7FFFFFFFu, 0xFFFFFFFFu
    };
    static const uint32_t flags[] = {0, 1, 0x80u, 0x80000000u, 0xFFFFFFFFu};
    uint32_t component[11] = {0};
    uint32_t random_state = 0x532CCu;
    unsigned int a, b, c, f;
    int32_t output;

    for (a = 0; a < 256; a++) {
        ((uint8_t *)component)[4] = (uint8_t)a;
        output = -1;
        assert(func_80052AD4(component, &output) == 0);
        assert(output == (int32_t)(a & 3u));
    }
    component[1] = 0xA5A5A5A5u;
    assert(func_80052AD4(component, (int32_t *)&component[1]) == 0);
    assert(component[1] == (0xA5u & 3u));
    for (a = 0; a < sizeof(values) / sizeof(values[0]); a++) {
        for (b = 0; b < sizeof(values) / sizeof(values[0]); b++) {
            for (c = 0; c < sizeof(values) / sizeof(values[0]); c++) {
                for (f = 0; f < sizeof(flags) / sizeof(flags[0]); f++) {
                    check_packing(values[a], values[b], values[c], flags[f]);
                }
            }
        }
    }
    for (a = 0; a < 20000; a++) {
        uint32_t inputs[4];
        for (b = 0; b < 4; b++) {
            random_state = random_state * 1664525u + 1013904223u;
            inputs[b] = random_state;
        }
        check_packing(inputs[0], inputs[1], inputs[2], inputs[3]);
    }
}

static void test_actor_bit_queries(void) {
    static const uint32_t states[] = {0, UINT32_MAX, 0x55555555u, 0xAAAAAAAAu};
    uint32_t actor[8] = {0};
    unsigned int selector, state;

    for (selector = 0; selector < 256; selector++) {
        uint32_t mask = 1u << (selector & 31u);
        for (state = 0; state < sizeof(states) / sizeof(states[0]); state++) {
            actor[1] = states[state];
            actor[4] = ~states[state];
            assert(func_80022674(actor, (uint8_t)selector) == ((states[state] & mask) != 0));
            assert(func_80022604(actor, (uint8_t)selector) == (((~states[state]) & mask) != 0));
            assert(actor[1] == states[state] && actor[4] == ~states[state]);
        }
    }
}

static void test_auxiliary_lookup(void) {
    GameLevelRecord records[4] = {{0}};
    int objects[4];
    int32_t count, index;

    assert(sizeof(GameAuxiliaryObjectSlot) == 0x34);
    for (index = 0; index < 4; index++) {
        records[index].object_1C = &objects[index];
    }
    g_level_records = records;
    for (count = 0; count <= 4; count++) {
        g_level_record_count = count;
        assert(func_80028D48() == count);
        for (index = 0; index < count; index++) {
            assert(func_80028764(index) == &objects[index]);
        }
        for (index = 0; index < 8; index++) {
            assert(func_80040FC0(count + index) == &g_auxiliary_object_slots[index]);
            assert(func_80028764(count + index) == &g_auxiliary_object_slots[index]);
        }
    }
    g_level_record_count = -3;
    assert(func_80028D48() == -3);
    assert(func_80028764(0) == &g_auxiliary_object_slots[3]);
    assert(func_80028764(-1) == 0);
}

static void test_global_helpers(void) {
    GroundedCase37ObjectView first = {0}, second = {0};
    ProximityObject reference = {0};
    uint32_t value;

    g_grounded_case37_object = &first;
    func_80037B14(&second, 0);
    assert(g_grounded_case37_object == &first);
    func_80037B14(&first, &second);
    assert(g_grounded_case37_object == &second);
    func_80037B14(&second, 0);
    assert(g_grounded_case37_object == 0);
    func_80037B14(0, &first);
    assert(g_grounded_case37_object == &first);
    func_80053270(&reference);
    assert(func_8005327C() == &reference);
    func_80053270(0);
    assert(func_8005327C() == 0);
    for (value = 0; value <= UINT16_MAX; value++) {
        func_800691AC((uint16_t)value);
        assert(g_l0_gp_6C2 == value);
    }
}

static void test_movement_reset(void) {
    MovementResetObject object;
    MovementResetComponent first, second, replacement;
    unsigned int initial_second, change, first_is_null;

#if UINTPTR_MAX == UINT32_MAX
    _Static_assert(offsetof(MovementResetObject, component_1C) == 0x1C, "first component offset");
    _Static_assert(offsetof(MovementResetObject, component_20) == 0x20, "selected component offset");
    _Static_assert(offsetof(MovementResetObject, component_24) == 0x24, "second component offset");
    _Static_assert(offsetof(MovementResetObject, field_2C) == 0x2C, "state byte offset");
#endif
    for (first_is_null = 0; first_is_null < 2; first_is_null++) {
        for (initial_second = 0; initial_second < 2; initial_second++) {
            for (change = 0; change < 3; change++) {
                MovementResetComponent *expected_second;

                memset(&object, 0xA5, sizeof(object));
                object.component_20 = first_is_null ? 0 : &first;
                object.component_24 = initial_second ? &second : 0;
                object.field_18 = 0x12345678u;
                reset_object = &object;
                reset_count = 0;
                mutate_reset = change != 0;
                reset_replacement = change == 1 ? &replacement : 0;
                expected_second = mutate_reset ? reset_replacement : object.component_24;
                func_80028698(&object);
                assert(object.component_1C == (first_is_null ? 0 : &first));
                assert(reset_calls[0] == (first_is_null ? 0 : &first));
                assert(reset_count == (expected_second != 0 ? 2u : 1u));
                if (expected_second != 0) {
                    assert(reset_calls[1] == expected_second);
                }
                assert(object.component_24 == expected_second);
                assert(object.field_18 == (mutate_reset ? 0x87654321u : 0x12345678u));
                assert(object.field_28 == 0xA5A5A5A5u && object.field_30 == 0xA5A5A5A5u);
            }
        }
    }
}

int main(void) {
    test_component_helpers();
    test_actor_bit_queries();
    test_auxiliary_lookup();
    test_global_helpers();
    test_movement_reset();
    puts("Grounded low-level helpers: component packing, flags, lookup, globals and reset passed");
    return 0;
}
