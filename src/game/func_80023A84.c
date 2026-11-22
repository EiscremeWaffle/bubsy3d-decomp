#include "game_transition.h"
#include "grounded_transform.h"
#include "input_events.h"
#include "level_records.h"
#include "reset_resources.h"
#include "texture_font_slots.h"

extern void func_8002DDFC(void);
extern void func_80028E50(void);
extern void func_8005FBE4(void);
extern void func_8002C9D4(int32_t value);
extern void func_800486D0(const char *text, int32_t width, int32_t height, void *output);
extern void func_8002BCD4(void);
extern void func_8002BAD4(int32_t index);
extern void func_8002D778(void);
extern void func_8001B0A0(void);
extern void func_8005F374(void);
extern int32_t func_800516F8(void **output);
extern int32_t func_80054AF0(const int16_t *component, int32_t *output);
extern int32_t func_80054B20(void *component, int32_t mode, int32_t *output);
extern int32_t func_80054C7C(void *component, int32_t mode, const int32_t *input);
extern void func_8002C370();
extern void func_800430E8(int32_t **indices, int32_t *count);

extern const char g_unknown_8001211C[];
extern int32_t g_unknown_800BDEE0;
extern int32_t g_unknown_800BDEE4;
extern int32_t g_unknown_800BDEE8;
extern int8_t g_unknown_800BE478;
extern int8_t g_resource_output_800BE478;

typedef struct Func80023A84Scratch {
    GroundedTransformVector direction;
    int32_t direction_padding;
    GroundedTransformVector rotated_direction;
    int32_t rotated_direction_padding;
    GroundedTransformVector translation;
    int32_t translation_padding;
    void *reference_component;
    int32_t *indices;
    int32_t count;
    int32_t padding[3];
} Func80023A84Scratch;

void func_80023A84(void) {
    Func80023A84Scratch scratch;
    int32_t index;
    void *actor;
    void *owner;
    void *component;

    actor = func_80028764(0);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=m"(scratch.padding));
#endif
    owner = *(void **)((uint8_t *)actor + 0x1C);
    component = *(void **)((uint8_t *)owner + 0x14);

    func_8001BC08();
    func_8002DDFC();
    func_80048488();
    func_80028E50();
    func_8005FBE4();
    func_8002C9D4(1);
    func_800486D0(g_unknown_8001211C, 0xC8, 0xF0, &g_resource_output_800BE478);
    func_800481D4(g_unknown_800BE478, 0x1000, 0x1000);
    func_800483C0(g_unknown_800BE478);
    func_8002C370();
    func_8002BCD4();
    func_8002BAD4(0xF);
    func_8002D778();

    func_800516F8(&scratch.reference_component);
    func_80054AF0(component, scratch.direction.values);
    func_80054B20(component, 0, scratch.translation.values);
    func_8002E858(
        &scratch.direction,
        g_unknown_800BDEE0,
        g_unknown_800BDEE4 + scratch.translation.values[1],
        &scratch.rotated_direction
    );
    func_80054C7C(scratch.reference_component, 0, scratch.rotated_direction.values);
    func_8002E954(&scratch.translation, g_unknown_800BDEE8);
    func_8002EA80();

    func_80045094();
    for (index = 0; index < 4; index++) {
        func_8001B0A0();
    }

    func_800430E8(&scratch.indices, &scratch.count);
    index = 0;
    if (scratch.count > 0) {
        do {
            int32_t record_index = scratch.indices[index];
            void *object;

            if (record_index < g_level_record_count) {
                goto level_record;
            }
            object = func_80040FC0(record_index);
            goto object_ready;

level_record:
            object = g_level_records[record_index].object_1C;
object_ready:
            if ((((uint8_t *)object)[0x2D] & 0x40) != 0) {
                func_8001B558(
                    0x31, 5, 0xFFFE,
                    (int32_t)(uintptr_t)*(void **)((uint8_t *)object + 0x0C),
                    0, 0, 0, 0
                );
            }
            index++;
        } while (index < scratch.count);
    }
    func_8005F374();
}
