#include "component_rotation.h"

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
/* The original accesses the aligned MATRIX storage as both words and halfwords. */
typedef union GteMatrixWords {
    uint32_t words[5];
    int32_t signed_words[5];
    uint16_t halfwords[10];
    int16_t signed_halfwords[10];
} GteMatrixWords;

#define MATCH_KEEP(value) __asm__ volatile("" : "=r"(value) : "0"(value))
/* MVMVA: rotation matrix times V0, Q12 shift, no translation, signed clamp. */
#define GTE_ROTATE_Q12 ".word 0x4A486012"

#ifdef BUBSY3D_GTE_COMPOSE_FIRST
ComponentRotationMatrix *func_80076890(
    ComponentRotationMatrix *first_matrix, const ComponentRotationMatrix *second_matrix
)
#else
ComponentRotationMatrix *func_8007699C(
    const ComponentRotationMatrix *first_matrix, ComponentRotationMatrix *second_matrix
)
#endif
{
    const volatile GteMatrixWords *first = (const volatile GteMatrixWords *)first_matrix;
    const volatile GteMatrixWords *second = (const volatile GteMatrixWords *)second_matrix;
    register uint32_t packed_column __asm__("$8");
    register uint32_t column_word __asm__("$9");
    register int32_t column_z __asm__("$10");
    register uint32_t r00 __asm__("$11");
    register uint32_t r10 __asm__("$12");
    register uint32_t r20 __asm__("$13");
    register uint32_t r01 __asm__("$14");
    register uint32_t r11 __asm__("$15");
    register uint32_t r21 __asm__("$24");
    register uint32_t upper_mask __asm__("$1");
    register ComponentRotationMatrix *result __asm__("$2");
#ifdef BUBSY3D_GTE_COMPOSE_FIRST
    register volatile GteMatrixWords *output __asm__("$4") = (volatile GteMatrixWords *)first_matrix;
#else
    register volatile GteMatrixWords *output __asm__("$5") = (volatile GteMatrixWords *)second_matrix;
#endif

    packed_column = first->words[0];
    column_word = first->words[1];
    column_z = first->signed_words[2];
    r00 = first->words[3];
    r10 = first->words[4];
    /* The MIPS-I assembler rejects CTC2 mnemonics; these load control registers 0..4. */
    __asm__ volatile(
        ".word 0x48C80000\n\t"
        ".word 0x48C90800\n\t"
        ".word 0x48CA1000\n\t"
        ".word 0x48CB1800\n\t"
        ".word 0x48CC2000"
        : : "r"(packed_column), "r"(column_word), "r"(column_z), "r"(r00), "r"(r10) : "memory"
    );

    packed_column = second->halfwords[0];
    column_word = second->words[1];
    column_z = second->signed_words[3];
    __asm__ volatile("" : : "r"(packed_column), "r"(column_word), "r"(column_z) : "$1");
    upper_mask = 0xFFFF0000U;
    MATCH_KEEP(upper_mask);
    column_word &= upper_mask;
    packed_column |= column_word;
    __asm__ volatile(
        "mtc2 %0,$0\n\t"
        "mtc2 %1,$1\n\t"
        "nop\n\t"
        GTE_ROTATE_Q12
        : : "r"(packed_column), "r"(column_z) : "memory"
    );

    packed_column = second->halfwords[1];
    column_word = second->words[2];
    __asm__ volatile("" : : : "memory");
    column_z = ((const GteMatrixWords *)second)->signed_halfwords[7];
    MATCH_KEEP(column_z);
    MATCH_KEEP(column_word);
    column_word <<= 16;
    packed_column |= column_word;
    __asm__ volatile(
        "mfc2 %0,$9\n\t"
        "mfc2 %1,$10\n\t"
        "mfc2 %2,$11\n\t"
        "mtc2 %3,$0\n\t"
        "mtc2 %4,$1\n\t"
        "nop\n\t"
        GTE_ROTATE_Q12
        : "=r"(r00), "=r"(r10), "=r"(r20) : "r"(packed_column), "r"(column_z) : "memory"
    );

    packed_column = second->halfwords[2];
    column_word = second->words[2];
    column_z = second->signed_words[4];
    __asm__ volatile("" : : "r"(packed_column), "r"(column_word), "r"(column_z) : "$1");
    upper_mask = 0xFFFF0000U;
    MATCH_KEEP(upper_mask);
    column_word &= upper_mask;
    packed_column |= column_word;
    __asm__ volatile(
        "mfc2 %0,$9\n\t"
        "mfc2 %1,$10\n\t"
        "mfc2 %2,$11\n\t"
        "mtc2 %3,$0\n\t"
        "mtc2 %4,$1\n\t"
        "nop\n\t"
        GTE_ROTATE_Q12
        : "=r"(r01), "=r"(r11), "=r"(r21) : "r"(packed_column), "r"(column_z) : "memory"
    );

    r00 &= 0xFFFFU;
    r01 <<= 16;
    r01 |= r00;
    output->words[0] = r01;
    MATCH_KEEP(r20);
    MATCH_KEEP(r21);
    r20 &= 0xFFFFU;
    r21 <<= 16;
    r21 |= r20;
    output->words[3] = r21;
    __asm__ volatile(
        "mfc2 %0,$9\n\t"
        "mfc2 %1,$10"
        : "=r"(packed_column), "=r"(column_word) : : "memory"
    );
    packed_column &= 0xFFFFU;
    r10 <<= 16;
    packed_column |= r10;
    output->words[1] = packed_column;
    MATCH_KEEP(r11);
    MATCH_KEEP(column_word);
    r11 &= 0xFFFFU;
    column_word <<= 16;
    column_word |= r11;
    output->words[2] = column_word;
    __asm__ volatile("swc2 $11,16(%0)" : : "r"(output) : "memory");
    MATCH_KEEP(output);
    result = (ComponentRotationMatrix *)output;
    MATCH_KEEP(result);
    return result;
}
#else
static int32_t signed_matrix_element(uint16_t value) {
    return value < 0x8000U ? (int32_t)value : (int32_t)value - 0x10000;
}

static void compose_matrix(
    const ComponentRotationMatrix *first,
    const ComponentRotationMatrix *second,
    ComponentRotationMatrix *output
) {
    int32_t left[9];
    int32_t right[9];
    uint16_t values[9];
    unsigned int row;
    unsigned int column;
    unsigned int inner;

    for (row = 0; row < 9; row++) {
        left[row] = signed_matrix_element(first->elements[row]);
        right[row] = signed_matrix_element(second->elements[row]);
    }
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            int64_t total = 0;
            int64_t shifted;
            for (inner = 0; inner < 3; inner++) {
                total += (int64_t)left[row * 3 + inner] * right[inner * 3 + column];
            }
            shifted = total >= 0 ? total / 4096 : -((-total + 4095) / 4096);
            if (shifted < -32768) {
                shifted = -32768;
            } else if (shifted > 32767) {
                shifted = 32767;
            }
            values[row * 3 + column] = (uint16_t)shifted;
        }
    }
    for (row = 0; row < 9; row++) {
        output->elements[row] = values[row];
    }
    output->padding = values[8] & 0x8000U ? 0xFFFFU : 0;
}

ComponentRotationMatrix *func_80076890(
    ComponentRotationMatrix *first, const ComponentRotationMatrix *second
) {
    compose_matrix(first, second, first);
    return first;
}

ComponentRotationMatrix *func_8007699C(
    const ComponentRotationMatrix *first, ComponentRotationMatrix *second
) {
    compose_matrix(first, second, second);
    return second;
}
#endif
