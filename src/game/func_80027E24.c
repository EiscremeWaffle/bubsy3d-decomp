#include "bubsy_grounded_state.h"
#include "level_records.h"

extern const char g_level_list_condition[];
extern const char g_game_source_path[];
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

int32_t func_80027E24(GameLevelListInput *input) {
    int32_t list_id;
    int32_t count;
    int32_t *indices;
    int32_t i;

    func_80028C84(input->record_id_0C, &list_id);
    indices = func_80028CD4(list_id, &count);
    for (i = 0; i < count; ++i) {
        if (indices[i] != input->record_id_0C) {
            BubsyGroundedActor *actor = func_80028764(indices[i]);
            if (actor->state_00 == 0xF5) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                /* The original forwards the live register; the real getter ignores it. */
                register void *context __asm__("$5");
                __asm__ volatile("" : "=r"(context));
#else
                void *context = 0;
#endif
                return (uint8_t)func_8004B264(actor, context) != 0;
            }
        }
    }
    func_800556D0(0, g_level_list_condition, g_game_source_path, 0x12CA);
    return 1;
}
