#include <stdint.h>

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
__asm__(
    ".text\n"
    ".align 2\n"
    ".globl func_800100A0\n"
    ".type func_800100A0,@function\n"
    ".ent func_800100A0\n"
    "func_800100A0:\n"
    ".set noreorder\n"
    "addu $10,$zero,$zero\n"
    "lw $8,0($4)\n"
    "lw $9,0($5)\n"
    "nop\n"
    "mult $8,$9\n"
    "mfhi $8\n"
    "mflo $9\n"
    "sll $8,$8,20\n"
    "srl $9,$9,12\n"
    "addu $8,$8,$9\n"
    "addu $10,$10,$8\n"
    "lw $8,4($4)\n"
    "lw $9,4($5)\n"
    "nop\n"
    "mult $8,$9\n"
    "mfhi $8\n"
    "mflo $9\n"
    "sll $8,$8,20\n"
    "srl $9,$9,12\n"
    "addu $8,$8,$9\n"
    "addu $10,$10,$8\n"
    "lw $8,8($4)\n"
    "lw $9,8($5)\n"
    "nop\n"
    "mult $8,$9\n"
    "mfhi $8\n"
    "mflo $9\n"
    "sll $8,$8,20\n"
    "srl $9,$9,12\n"
    "addu $8,$8,$9\n"
    "addu $10,$10,$8\n"
    "sw $10,0($6)\n"
    "jr $31\n"
    "addu $2,$zero,$zero\n"
    ".set reorder\n"
    ".end func_800100A0\n"
    ".size func_800100A0,.-func_800100A0\n"
);
#else
int32_t func_800100A0(const int32_t *left, const int32_t *right, int32_t *output) {
    *output = (int32_t)(((long long)left[0] * right[0] >> 12) +
        ((long long)left[1] * right[1] >> 12) +
        ((long long)left[2] * right[2] >> 12));
    return 0;
}
#endif
