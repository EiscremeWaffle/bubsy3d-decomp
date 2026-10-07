#include "player_model.h"

#ifndef BUBSY3D_MATCH_ORIGINAL_L0
extern void *g_player_model_asset;
extern volatile uint8_t g_player_model_mode;
#endif

uint8_t level_get_player_model_mode(void) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uintptr_t global_base __asm__("$2") = 0x801E0000u;
    __asm__ volatile("" : "+r"(global_base));
    return *(volatile const uint8_t *)(global_base - 0x7610u);
#else
    return g_player_model_mode;
#endif
}

void *level_get_player_model_global(void **asset_out) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uintptr_t global_base __asm__("$2") = 0x801E0000u;
    __asm__ volatile("" : "+r"(global_base));
    void *asset = *(void * volatile *)(global_base - 0x75F8u);
#else
    void *asset = g_player_model_asset;
#endif
    *asset_out = asset;
    return asset;
}
