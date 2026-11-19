#ifndef BUBSY3D_LEVEL_RECORDS_H
#define BUBSY3D_LEVEL_RECORDS_H

#include "move_preparation.h"

typedef struct GameLevelPosition {
    int32_t words[4];
} GameLevelPosition;

typedef struct GameLevelRecord {
    uint8_t unknown_00[4];
    uint8_t selector_04;
    uint8_t unknown_05[3];
    const uint16_t *rotation_08;
    const GameLevelPosition *position_0C;
    int32_t table_count_10;
    const MovePreparationEntry *table_14;
    int32_t list_id_18;
    void *object_1C;
    uint8_t flag_20;
    uint8_t unknown_21[3];
} GameLevelRecord;

typedef struct GameLevelList {
    int16_t unknown_00;
    int16_t count_02;
    int32_t *indices_04;
} GameLevelList;

typedef struct GameLevelListInput {
    uint8_t unknown_00[0x0C];
    int32_t record_id_0C;
} GameLevelListInput;

typedef struct GameAuxiliaryObjectSlot {
    uint8_t bytes[0x34];
} GameAuxiliaryObjectSlot;

extern int32_t g_level_record_count;
extern GameLevelRecord *volatile g_level_records;
extern int32_t g_level_list_count;
extern GameLevelList *volatile g_level_lists;
extern GameAuxiliaryObjectSlot g_auxiliary_object_slots[];

uint8_t func_800286F8(int32_t index, uint8_t *selector);
void *func_80028764(int32_t index);
int32_t func_800288D8(int32_t index);
int32_t func_80028B5C(int32_t index, GameLevelPosition *output);
int32_t func_80028D48(void);
void *func_80040FC0(int32_t index);
uint8_t func_80028C18(int32_t index, int32_t *count, const MovePreparationEntry **table);
uint8_t func_80028C84(int32_t index, int32_t *list_id);
int32_t *func_80028CD4(int32_t list_id, int32_t *count);
int32_t func_80027E24(GameLevelListInput *input);

#endif
