#include "actor_context.h"
#include "controller_callback_helpers.h"
#include "controller_result.h"

extern void func_8002DF24(const char *text);
extern void func_8002C604(int32_t value);
extern int32_t func_8002BD0C(int32_t value);
extern void func_8002CA88(int32_t value, int32_t duration);
extern void func_8002E1E0(int32_t value);
extern void func_8002E348(int32_t value);
extern void func_8002BC7C(void);
extern void func_8002C010(void);
extern void func_8002C1F0(void);
extern uint8_t func_800283E0(void);
extern void func_8002BC9C(void);

void func_8002C370(void *argument0, void *argument1) {
    ControllerResultContext *context = func_80026648(argument0, argument1);

    if (context->mode_0C == 1) {
        if (context->selection_14 != g_result_previous_selection) {
            const char *text = g_result_player_one_text;
            if (context->selection_14 != 0) {
                text = g_result_player_two_text;
            }
            func_8002DF24(text);
            g_result_previous_selection = context->selection_14;
        }
        func_8002C604(6);
        func_8002C604(7);
        func_8002CA88(17, func_8002BD0C(17));
        func_8002E1E0(0);
        func_8002E348(0);
    } else {
        func_8002BC7C();
        func_8002C010();
        func_8002C1F0();
        if (g_unknown_8009F58C != 0) {
            func_8002CA88(5, -1);
        }
        if (func_800283E0() == 1) {
            func_8002BC9C();
            func_8002CA88(15, -1);
        }
    }
}
