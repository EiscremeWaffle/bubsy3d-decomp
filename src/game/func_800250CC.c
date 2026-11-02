#include <stdint.h>

typedef struct GameObjectView {
    uint8_t unused_00_to_1B[0x1C];
    void *component_1C;
} GameObjectView;

extern void *func_80028764(int32_t unused);
extern int32_t func_80052ABC(void *component, int32_t *sequence_key_out);
extern void func_80024F2C(void *object);
extern uint16_t g_move_request_count;
extern void *g_move_request_entries[];

void func_800250CC(void) {
    int32_t sequence_key[2];
    int16_t index;
    GameObjectView *game_object = func_80028764(0);

    func_80052ABC(game_object->component_1C, sequence_key);
    if (sequence_key[0] != 0x31F) {
        for (index = 0; index < (int16_t)g_move_request_count; index++) {
            func_80024F2C(g_move_request_entries[index]);
        }
    }
    g_move_request_count = 0;
}