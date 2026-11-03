#include "move_preparation.h"

extern void *g_move_preparation_transform;
extern const char g_move_table_condition[];
extern const char g_game_source_path[];

extern uint8_t func_80028C18(int32_t context, int32_t *count, const MovePreparationEntry **table);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);
extern uint8_t func_800222EC(MovePreparationInfo **info, const MovePreparationVector *origin);
extern int32_t func_80054624(void **transform);

int32_t func_80025FFC(MovePreparationActor *actor, const MovePreparationVector *origin,
    const void *scale_input, uint32_t operation_flags) {
    int32_t table_count;
    const MovePreparationEntry *table = 0;

    if (actor->move_info_28 != 0) {
        return 1;
    }
    if (func_80028C18(actor->context_0C, &table_count, &table) != 1) {
        func_800556D0(1, g_move_table_condition, g_game_source_path, 0xE08);
    }
    if (table_count == 0) {
        return 1;
    }
    if (table[0].limit == 0) {
        return 1;
    }
    if (func_800222EC(&actor->move_info_28, origin) == 0) {
        return 0;
    }
    if (actor->context_0C == 0) {
        return 1;
    }
    if (g_move_preparation_transform == 0) {
        if (func_80054624(&g_move_preparation_transform) != 0) {
            return 0;
        }
    }
    return (uint8_t)func_8002611C(actor, scale_input, operation_flags) != 0;
}
