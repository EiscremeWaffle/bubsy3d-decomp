#include "controller_callback_helpers.h"

extern void func_80062C54(void *state);
extern void func_80082EFC(void *allocation);

void func_80027F40(void) {
    func_80062C54(g_unknown_800BE4F8);
    func_80082EFC(g_player_model_asset);
}
