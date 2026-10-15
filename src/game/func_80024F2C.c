#include <stdint.h>

typedef struct MoveInfoView {
    int32_t words[0xA4 / sizeof(int32_t)];
} MoveInfoView;

typedef struct ComponentHolderView {
    uint8_t padding[0x14];
    void *component_transform;
} ComponentHolderView;

typedef struct MoveConsumerObjectView {
    uint8_t padding_00[0x1C];
    ComponentHolderView * volatile component_holder;
    uint8_t padding_20[8];
    MoveInfoView * volatile move_info;
} MoveConsumerObjectView;

extern const char g_move_info_condition[];
extern const char g_game_source_path[];
extern int32_t func_800556D0(int32_t failed, const char *condition, const char *source_path, uint32_t line);
extern int32_t func_800548A0(void *component_transform, const int32_t *rotation);
extern int32_t func_800536F4(const int32_t *input, int32_t *output);
extern int32_t func_80010038(const int32_t *left, const int32_t *right, int32_t *output);
extern int32_t func_80054B20(void *component_transform, int32_t mode, int32_t *output);
extern int32_t func_80054C7C(void *component_transform, int32_t mode, const int32_t *translation);

void func_80024F2C(const MoveConsumerObjectView *object) {
    ComponentHolderView *component_holder = object->component_holder;
    register MoveInfoView *move_info __asm__("$17") = object->move_info;
    int32_t translation_scratch[4];
    int32_t rotation_scratch[4];
    register int32_t *rotation_48 __asm__("$19");
    register int32_t *translation_38 __asm__("$18") = move_info->words + 0x38 / sizeof(int32_t);
    register void *component_value __asm__("$20") = component_holder->component_transform;
    int32_t archive_0;
    int32_t archive_1;
    int32_t archive_2;
    int32_t archive_3;

    if (move_info == 0) {
        register int32_t failed __asm__("$4");
        __asm__ volatile("addu $4,$zero,$zero" : "=r"(failed));
        func_800556D0(failed, g_move_info_condition, g_game_source_path, 0xB12);
        return;
    }

    if ((rotation_48 = move_info->words + 0x48 / sizeof(int32_t))[0] != 0 ||
        rotation_48[1] != 0 || rotation_48[2] != 0) {
        func_800548A0(component_value, rotation_48);
    }

    if (move_info->words[0x98 / sizeof(int32_t)] != 0 ||
        move_info->words[0x9C / sizeof(int32_t)] != 0 ||
        move_info->words[0xA0 / sizeof(int32_t)] != 0) {
        func_800536F4(move_info->words + 0x98 / sizeof(int32_t), rotation_scratch);
        func_80010038(
            move_info->words + 0x38 / sizeof(int32_t),
            rotation_scratch,
            move_info->words + 0x38 / sizeof(int32_t)
        );
        move_info->words[0xA0 / sizeof(int32_t)] = 0;
        move_info->words[0x9C / sizeof(int32_t)] = 0;
        move_info->words[0x98 / sizeof(int32_t)] = 0;
    }

        func_80054B20(component_value, 0, move_info->words + 0x68 / sizeof(int32_t));

    if (translation_38[0] != 0 || translation_38[1] != 0 || translation_38[2] != 0) {
        func_80010038(move_info->words + 0x68 / sizeof(int32_t), translation_38, translation_scratch);
        func_80054C7C(component_value, 0, translation_scratch);
    }

    archive_0 = translation_38[0];
    archive_1 = translation_38[1];
    archive_2 = translation_38[2];
    archive_3 = translation_38[3];
    move_info->words[0x78 / sizeof(int32_t)] = archive_0;
    move_info->words[0x7C / sizeof(int32_t)] = archive_1;
    move_info->words[0x80 / sizeof(int32_t)] = archive_2;
    move_info->words[0x84 / sizeof(int32_t)] = archive_3;
    translation_38[2] = 0;
    translation_38[1] = 0;
    translation_38[0] = 0;
    rotation_48[2] = 0;
    rotation_48[1] = 0;
    rotation_48[0] = 0;
}
