#include "actor_context.h"
#include "game_transition.h"
#include "grounded_case37.h"
#include "level_records.h"

extern int32_t func_80054B20(void *component, int32_t mode, int32_t *translation);

int32_t func_800288D8(int32_t index) {
    GameLevelPosition position;
    int32_t event_parameter;
    int32_t initial_parameter = 11;
    uint8_t *context;
    GroundedCase37ComponentView *object;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register void *unused_context __asm__("$5");
    register uint8_t *returned_context __asm__("$2");
    register const volatile int32_t *record_count __asm__("$5");
    /* The getter ignores both arguments; keep the incoming a1 and call schedule. */
    __asm__ volatile("" : "=r"(unused_context) : "r"(initial_parameter));
#else
    void *unused_context = 0;
    uint8_t *returned_context;
    const volatile int32_t *record_count;
#endif

    event_parameter = initial_parameter;
    returned_context = func_80026648((void *)(uintptr_t)(uint32_t)index, unused_context);
    record_count = &g_level_record_count;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    /* Retain the count address and getter result in their original registers. */
    __asm__ volatile("" : "=r"(record_count), "=r"(returned_context)
                     : "0"(record_count), "1"(returned_context));
#endif
    context = returned_context;
    if (index >= *record_count || index < 0) {
        return 0;
    }

    g_level_records[index].flag_20 = 0;
    if (index >= *record_count) {
        object = func_80040FC0(index);
    } else {
        object = g_level_records[index].object_1C;
    }

    if (context[0x0C] != 0) {
        func_80054B20(object->owner_1C->transform_14, 0, position.words);
        func_8001B558(
            0x1A, 4, 0xFFFE, 0xFFFE,
            &event_parameter, &position.words[0], &position.words[1], &position.words[2]
        );
    }
    return 1;
}
