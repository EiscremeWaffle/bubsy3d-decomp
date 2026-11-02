#include "move_preparation.h"

extern void *g_move_preparation_transform;
extern const char g_move_context_condition[];
extern const char g_move_info_condition[];
extern const char g_move_table_condition[];
extern const char g_move_error_condition[];
extern const char g_move_tag_condition[];
extern const char g_game_source_path[];

extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);
extern uint8_t func_80028C18(int32_t context, int32_t *count, const MovePreparationEntry **table);
extern void func_800286F8(int32_t context, uint8_t *selector);
extern int32_t func_80054B20(void *transform, int32_t mode, MovePreparationVector *output);
extern int32_t func_80062D4C(const MovePreparationVector *left, const MovePreparationVector *right, int32_t *output);
extern int32_t func_80054C7C(void *transform, int32_t mode, const MovePreparationVector *input);
extern int32_t func_800546F4(void *transform, MovePreparationBasis *output);
extern int32_t func_800550A4(void *transform, void *global_transform);
extern int32_t func_8005541C(void *transform, const void *input);
extern int32_t func_80054AF0(void *transform, MovePreparationVector *output);
extern int32_t func_800549E8(void *transform, const MovePreparationBasis *input);
extern void func_80022944(MovePreparationActor *actor);
extern void func_80024EBC(MovePreparationActor *actor);

int32_t func_80025A98(MovePreparationActor *actor, const void *basis_input,
    uint32_t operation_flags, uint8_t *completion) {
    MovePreparationVector translation;
    MovePreparationBasis basis;
    MovePreparationVector payload;
    MovePreparationVector transform_values;
    int32_t table_count;
    const MovePreparationEntry *table = 0;
    uint8_t selector;
    MovePreparationInfo *info = actor->move_info_28;
    void *transform = 0;
    uint8_t done;
    uint32_t force_transform;

    if (actor->context_0C == 0) {
        func_800556D0(0, g_move_context_condition, g_game_source_path, 0xD27);
    }
    if (info == 0) {
        func_800556D0(1, g_move_info_condition, g_game_source_path, 0xD2A);
    }
    *completion = 0;
    if ((uint32_t)info->index + 2u < 2u) {
        return 1;
    }
    if (info->separation_18 > 0) {
        goto notify_and_enqueue;
    }
    if (func_80028C18(actor->context_0C, &table_count, &table) != 1) {
        func_800556D0(1, g_move_table_condition, g_game_source_path, 0xD3A);
    }
    if (table[info->index].tag == 10) {
        info->counter = table[info->index].limit;
    } else {
        info->counter = (int32_t)((uint32_t)info->counter + 1u);
    }
    if (info->counter >= table[info->index].limit) {
        if (info->mode_1C == 1) {
            info->index = (int32_t)((uint32_t)info->index + 1u);
            if (info->index == table_count) {
                *completion = 1;
                info->mode_1C = 0;
                info->index = (int32_t)((uint32_t)info->index - 1u);
                return 1;
            }
            info->counter = 0;
        }
        if (info->mode_1C == 0 || info->mode_1C == 2) {
            done = 0;
            do {
                info->index--;
                if (info->index < 0) {
                    info->mode_1C = 1;
                    done = 1;
                    info->index = 0;
                    info->counter = 0;
                } else if (table[info->index].tag == 10) {
                    info->counter = (int32_t)((uint32_t)table[info->index].limit - 1u);
                    done = 1;
                }
            } while (!done);
        }
    }
    while (info->separation_18 == 0) {
        if (actor->component_1C == 0) {
            return 0;
        }
        transform = actor->component_1C->transform_14;
        payload = table[info->index].payload[info->counter];
        info->target.x = (int32_t)((uint32_t)info->origin_58.x + (uint32_t)payload.x);
        info->target.y = (int32_t)((uint32_t)info->origin_58.y + (uint32_t)payload.y);
        info->target.z = (int32_t)((uint32_t)info->origin_58.z + (uint32_t)payload.z);
        func_800286F8(actor->context_0C, &selector);
        if (selector == 1) {
            info->stride_30 = table[info->index].selector * 2;
        }
        if (func_80054B20(transform, 0, &translation) != 0) {
            return 0;
        }
        if (func_80062D4C(&translation, &info->target, &info->separation_18) != 0) {
            func_800556D0(0, g_move_error_condition, g_game_source_path, 0xDA7);
        }
        if (info->separation_18 == 0) {
            if (table[info->index].tag != 10) {
                func_800556D0(0, g_move_tag_condition, g_game_source_path, 0xDAD);
            }
            if (table[info->index].tag == 10) {
                if (info->counter == 0) {
                    info->counter = 1;
                } else {
                    info->counter = 0;
                }
            } else {
                break;
            }
        }
    }
    if (func_80054C7C(g_move_preparation_transform, 0, &info->target) != 0) {
        return 0;
    }
    if (func_800546F4(transform, &basis) != 0) {
        return 0;
    }
    if (func_800550A4(transform, g_move_preparation_transform) != 0) {
        return 0;
    }
    if (func_8005541C(transform, basis_input) != 0) {
        return 0;
    }
    force_transform = operation_flags & 1;
    if (force_transform != 0 || (operation_flags & 4) != 0) {
        if (func_80054AF0(transform, &transform_values) != 0) {
            return 0;
        }
        if ((uint32_t)transform_values.y + 0xCCBu >= 0x1997u || force_transform != 0) {
            info->transform_88 = transform_values;
            if (func_800549E8(transform, &basis) != 0) {
                return 0;
            }
            goto notify_and_enqueue;
        }
    }
    info->transform_88.x = 0;
    info->transform_88.y = 0;
    info->transform_88.z = 0;
notify_and_enqueue:
    func_80022944(actor);
    if (info->pending_38.x != 0 || info->pending_38.y != 0 || info->pending_38.z != 0) {
        func_80024EBC(actor);
    }
    return 1;
}
