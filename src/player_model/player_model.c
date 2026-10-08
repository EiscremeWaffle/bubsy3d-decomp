#include "player_model.h"

#define PLAYER_MODEL_FORM_FLAG 0x10u

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
extern int32_t func_800283E0(void);

void level_select_player_model_resource(void) {
    register int32_t mode_value __asm__("$2") = func_800283E0();
    register int32_t resource_status __asm__("$16");
    __asm__ volatile(
        ".set noat\n\t"
        ".set noreorder\n\t"
        "andi $2,$2,0xFF\n\t"
        "bnez $2,1f\n\t"
        "nop\n\t"
        "lui $2,0x800C\n\t"
        "lw $2,-0x1968($2)\n\t"
        "nop\n\t"
        "lbu $2,0x2A2A($2)\n\t"
        "nop\n\t"
        "andi $2,$2,0x10\n\t"
        "lui $4,0x800C\n\t"
        "addiu $4,$4,-0x2058\n\t"
        "beqz $2,2f\n\t"
        "nop\n\t"
        "lui $4,0x8001\n\t"
        "addiu $4,$4,0x23C8\n\t"
        "j 2f\n\t"
        "nop\n\t"
        "1:\n\t"
        "lui $2,0x800C\n\t"
        "lw $2,-0x1968($2)\n\t"
        "nop\n\t"
        "lbu $2,0x2A2A($2)\n\t"
        "nop\n\t"
        "andi $2,$2,0x10\n\t"
        "lui $4,0x8001\n\t"
        "addiu $4,$4,0x23E0\n\t"
        "beqz $2,2f\n\t"
        "nop\n\t"
        "lui $4,0x8001\n\t"
        "addiu $4,$4,0x23D4\n\t"
        "2:\n\t"
        "lui $6,0x801E\n\t"
        "lw $6,-0x75F4($6)\n\t"
        "lui $5,0x801E\n\t"
        "addiu $5,$5,-0x75F8\n\t"
        "jal func_8001BA10\n\t"
        "nop\n\t"
        "addu $16,$2,$zero\n\t"
        "bnez $16,3f\n\t"
        "ori $4,$zero,1\n\t"
        "lui $5,0x800C\n\t"
        "addiu $5,$5,-0x2050\n\t"
        "lui $6,0x8001\n\t"
        "addiu $6,$6,0x239C\n\t"
        "jal func_800556D0\n\t"
        "ori $7,$zero,0x495\n\t"
        "3:\n\t"
        "lui $1,0x801E\n\t"
        "sw $16,-0x75F4($1)\n\t"
        ".set reorder\n\t"
        ".set at"
        : "=r"(resource_status), "=r"(mode_value)
        : "1"(mode_value)
        : "$1", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "hi", "lo", "memory"
    );
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
