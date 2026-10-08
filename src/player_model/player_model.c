#include "player_model.h"

#define PLAYER_MODEL_FORM_FLAG 0x10u

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
extern int32_t func_800283E0(void);
extern int32_t func_8001BA10(const char *path, void **asset_out, int32_t previous_value);
extern void func_800556D0(int32_t failed, const char *condition, const char *source_path, uint32_t source_line);
extern volatile int32_t g_player_model_resource_value;

void level_select_player_model_resource(void) {
    register int32_t resource_status __asm__("$16");
    register uintptr_t config_base __asm__("$2");
    register uint32_t flags __asm__("$2");
    register uintptr_t name_base __asm__("$4");
    register const char *path __asm__("$4");
    register uintptr_t size_base __asm__("$6");
    register int32_t previous_value __asm__("$6");
    register uintptr_t asset_base __asm__("$5");
    register void **asset_out __asm__("$5");
    register uintptr_t condition_base __asm__("$5");
    register uintptr_t source_base __asm__("$6");

    if ((uint8_t)func_800283E0() == 0) {
        __asm__ volatile("" : : : "$2", "$4", "memory");
        config_base = 0x800C0000u;
        __asm__ volatile("" : "=r"(config_base) : "0"(config_base));
        flags = (*(const LevelModelConfig * volatile *)(config_base - 0x1968))->flags_2A2A;
        flags &= PLAYER_MODEL_FORM_FLAG;
        __asm__ volatile("" : "=r"(flags) : "0"(flags) : "$4", "memory");
        name_base = 0x800C0000u;
        __asm__ volatile("" : "=r"(name_base) : "0"(name_base));
        path = (const char *)(name_base - 0x2058);
        __asm__ volatile("" : "=r"(path) : "0"(path));
        if (flags != 0) {
            __asm__ volatile("" : : : "$4", "memory");
            name_base = 0x80010000u;
            __asm__ volatile("" : "=r"(name_base) : "0"(name_base));
            path = (const char *)(name_base + 0x23C8);
            __asm__ volatile("" : "=r"(path) : "0"(path));
        }
    } else {
        __asm__ volatile("" : : : "$2", "$4", "memory");
        config_base = 0x800C0000u;
        __asm__ volatile("" : "=r"(config_base) : "0"(config_base));
        flags = (*(const LevelModelConfig * volatile *)(config_base - 0x1968))->flags_2A2A;
        flags &= PLAYER_MODEL_FORM_FLAG;
        __asm__ volatile("" : "=r"(flags) : "0"(flags) : "$4", "memory");
        name_base = 0x80010000u;
        __asm__ volatile("" : "=r"(name_base) : "0"(name_base));
        path = (const char *)(name_base + 0x23E0);
        __asm__ volatile("" : "=r"(path) : "0"(path));
        if (flags != 0) {
            __asm__ volatile("" : : : "$4", "memory");
            name_base = 0x80010000u;
            __asm__ volatile("" : "=r"(name_base) : "0"(name_base));
            path = (const char *)(name_base + 0x23D4);
            __asm__ volatile("" : "=r"(path) : "0"(path));
        }
    }
    __asm__ volatile("" : "=r"(path) : "0"(path) : "$5", "$6", "memory");
    size_base = 0x801E0000u;
    __asm__ volatile("" : "=r"(size_base) : "0"(size_base));
    previous_value = *(volatile int32_t *)(size_base - 0x75F4);
    asset_base = 0x801E0000u;
    __asm__ volatile("" : "=r"(asset_base) : "0"(asset_base));
    asset_out = (void **)(asset_base - 0x75F8);
    __asm__ volatile("" : "=r"(path), "=r"(asset_out), "=r"(previous_value) : "0"(path), "1"(asset_out), "2"(previous_value) : "memory");
    resource_status = func_8001BA10(path, asset_out, previous_value);
    if (resource_status == 0) {
        register int32_t failed __asm__("$4") = 1;
        __asm__ volatile("" : "=r"(failed) : "0"(failed) : "$5", "$6", "memory");
        condition_base = 0x800C0000u;
        __asm__ volatile("" : "=r"(condition_base) : "0"(condition_base));
        condition_base -= 0x2050;
        __asm__ volatile("" : "=r"(condition_base) : "0"(condition_base));
        source_base = 0x80010000u;
        __asm__ volatile("" : "=r"(source_base) : "0"(source_base));
        func_800556D0(failed, (const char *)condition_base, (const char *)(source_base + 0x239C), 0x495);
    }
    __asm__ volatile("" : : : "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "memory");
    g_player_model_resource_value = resource_status;
}
#else
int32_t level_select_player_model_resource(
    PlayerModelState *state,
    PlayerModelLoadResource load_resource,
    PlayerModelAssertFailure assert_failure
) {
    const int is_pliskin = (state->config->flags_2A2A & PLAYER_MODEL_FORM_FLAG) != 0;
    const char *path;

    if (state->is_swimming != 0) {
        path = is_pliskin ? "PLISWIM.TZP" : "BUBSWIM.TZP";
    } else {
        path = is_pliskin ? "PLISKIN.TZP" : "BUB.TZP";
    }

    const int32_t resource_status = load_resource(path, &state->asset, state->asset_size);
    if (resource_status == 0) {
        assert_failure(1, "errorFlag == EGSBoolTrue", "../f/level.c", 0x495);
    }

    state->asset_size = resource_status;
    return resource_status;
}
#endif

void level_get_player_model(const PlayerModelState *state, void **asset_out) {
    *asset_out = state->asset;
}
