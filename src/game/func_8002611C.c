#include "move_preparation.h"

extern void *g_move_preparation_transform;
extern const char g_move_table_condition[];
extern const char g_move_error_condition[];
extern const char g_game_source_path[];

extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);
extern uint8_t func_80028C18(int32_t context, int32_t *count, const MovePreparationEntry **table);
extern void func_800286F8(int32_t context, uint8_t *selector);
extern int32_t func_80062D4C(const MovePreparationVector *left, const MovePreparationVector *right, int32_t *output);
extern int32_t func_80054C7C(void *transform, int32_t mode, const MovePreparationVector *input);
extern int32_t func_800546F4(void *transform, MovePreparationBasis *output);
extern int32_t func_800550A4(void *transform, void *global_transform);
extern int32_t func_8005541C(void *transform, const void *input);
extern int32_t func_80054AF0(void *transform, MovePreparationVector *output);
extern int32_t func_800549E8(void *transform, const MovePreparationBasis *input);

int32_t func_8002611C(MovePreparationActor *actor, const void *scale_input,
    uint32_t operation_flags) {
    MovePreparationBasis basis;
    MovePreparationVector payload;
    MovePreparationVector direction;
    int32_t table_count;
    const MovePreparationEntry *table = 0;
    uint8_t selector;
    MovePreparationInfo *info = actor->move_info_28;
    void *transform;
    uint32_t force_transform;

    if (func_80028C18(actor->context_0C, &table_count, &table) != 1) {
        func_800556D0(1, g_move_table_condition, g_game_source_path, 0xE41);
    }
    info->index = 0;
    info->mode_1C = 1;
    info->counter = 0;
    payload = table[info->index].payload[0];
    info->target.x = (int32_t)((uint32_t)info->origin_58.x + (uint32_t)payload.x);
    info->target.y = (int32_t)((uint32_t)info->origin_58.y + (uint32_t)payload.y);
    info->target.z = (int32_t)((uint32_t)info->origin_58.z + (uint32_t)payload.z);
    func_800286F8(actor->context_0C, &selector);
    if (selector == 1) {
        info->stride_30 = table[info->index].selector * 2;
    }
    if (func_80062D4C(&info->origin_58, &info->target, &info->separation_18) != 0) {
        func_800556D0(0, g_move_error_condition, g_game_source_path, 0xE55);
    }
    transform = actor->component_1C->transform_14;
    if (func_80054C7C(g_move_preparation_transform, 0, &info->target) != 0) {
        return 0;
    }
    if (func_800546F4(transform, &basis) != 0) {
        return 0;
    }
    if (func_800550A4(transform, g_move_preparation_transform) != 0) {
        return 0;
    }
    if (func_8005541C(transform, scale_input) != 0) {
        goto failed;
    }
    force_transform = operation_flags & 1;
    if (force_transform != 0 || (operation_flags & 4) != 0) {
        if (func_80054AF0(transform, &direction) != 0) {
            return 0;
        }
        if ((uint32_t)direction.y + 0xCCBu >= 0x1997u || force_transform != 0) {
            info->transform_88 = direction;
            if (func_800549E8(transform, &basis) == 0) {
                goto complete;
            }
failed:
            return 0;
        }
    }
    info->transform_88.x = 0;
    info->transform_88.y = 0;
    info->transform_88.z = 0;
complete:
    return 1;
}
