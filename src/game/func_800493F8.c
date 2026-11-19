#include "actor_context.h"
#include "controller_result.h"
#include "heading_timed_records.h"
#include "secondary_controller_helpers.h"

extern int32_t func_80050650(void *payload);
extern void func_80028EA8(void);

void func_800493F8(void *argument0, void *argument1) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register void *incoming0 __asm__("$4") = argument0;
    register void *incoming1 __asm__("$5") = argument1;
    register ControllerResultContext *returned __asm__("$2");
#else
    void *incoming0 = argument0;
    void *incoming1 = argument1;
#endif
    ControllerResultContext *context;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    returned = func_80026648(incoming0, incoming1);
    /* The matched leaf getter preserves both argument registers. */
    __asm__ volatile("" : "=r"(incoming0), "=r"(incoming1), "=r"(returned)
                     : "0"(incoming0), "1"(incoming1), "2"(returned));
    context = returned;
#else
    context = func_80026648(incoming0, incoming1);
#endif
    func_8002C370(incoming0, incoming1);
    func_80050650(g_secondary_controller_display);
    g_secondary_controller_flag_505 = 0;
    g_secondary_controller_flag_507 = 0;
    context->unknown_0D[0] = 0;
    func_80028EA8();
    func_8002CB2C(5);
    func_8002BAD4(5);
}
