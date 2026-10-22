#include <stdint.h>

typedef struct GteTransformMatrix {
    uint32_t words[8];
} GteTransformMatrix;

extern const volatile GteTransformMatrix g_camera_view_transform;
extern void func_8007754C(void);
extern void func_800775EC(void);
extern void *func_80076890(GteTransformMatrix *view_matrix, const void *object_transform);
extern void func_80076CCC(const GteTransformMatrix *light_matrix);

void func_800722C8(const void *object_transform) {
    register const volatile GteTransformMatrix *source_matrix __asm__("$5");
    volatile GteTransformMatrix view_matrix;
    register GteTransformMatrix *view_matrix_value __asm__("$16");

    __asm__ volatile ("la %0, g_camera_view_transform" : "=r" (source_matrix));
    {
        uint32_t matrix_word_0 = source_matrix->words[0];
        uint32_t matrix_word_1 = source_matrix->words[1];
        uint32_t matrix_word_2 = source_matrix->words[2];
        view_matrix.words[0] = matrix_word_0;
        view_matrix.words[1] = matrix_word_1;
        view_matrix.words[2] = matrix_word_2;
    }
    {
        uint32_t matrix_word_3 = source_matrix->words[3];
        uint32_t matrix_word_4 = source_matrix->words[4];
        uint32_t matrix_word_5 = source_matrix->words[5];
        view_matrix.words[3] = matrix_word_3;
        view_matrix.words[4] = matrix_word_4;
        view_matrix.words[5] = matrix_word_5;
    }
    {
        uint32_t matrix_word_6 = source_matrix->words[6];
        uint32_t matrix_word_7 = source_matrix->words[7];
        view_matrix.words[6] = matrix_word_6;
        view_matrix.words[7] = matrix_word_7;
    }

    func_8007754C();
    view_matrix_value = (GteTransformMatrix *)&view_matrix;
    func_80076890(view_matrix_value, object_transform);
    func_800775EC();
    func_80076CCC(view_matrix_value);
}