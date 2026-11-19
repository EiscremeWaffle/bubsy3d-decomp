#include "transform_reset_backend.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

void *g_l0_gp_888;
int32_t g_transform_reset_status;
uint32_t g_transform_reset_snapshot[8];
TransformResetDescriptor g_transform_reset_descriptor;

int32_t func_80051678(void);
int32_t func_800516F8(void **output);

static uint32_t expected_snapshot[8];
static unsigned backend_calls;
static int mutate_backend;

int32_t func_800731C0(TransformResetDescriptor *descriptor) {
    backend_calls++;
    assert(descriptor == &g_transform_reset_descriptor);
    assert(g_transform_reset_status == 0);
    assert(memcmp(g_transform_reset_snapshot, expected_snapshot, sizeof(expected_snapshot)) == 0);
    if (mutate_backend) {
        g_transform_reset_status = 0x12345678;
        g_transform_reset_snapshot[0] = 0xABCDEF01u;
        g_l0_gp_888 = NULL;
    }
    return -1;
}

int main(void) {
    uint32_t source[8];
    uint32_t random_state = 0x51678u;
    unsigned iteration, index;

    for (iteration = 0; iteration < 1024; iteration++) {
        void *selected = NULL;
        for (index = 0; index < 8; index++) {
            random_state = random_state * 1664525u + 1013904223u;
            source[index] = random_state;
            g_transform_reset_snapshot[index] = ~random_state;
        }
        g_l0_gp_888 = iteration & 1 ? g_transform_reset_snapshot : source;
        memcpy(expected_snapshot, g_l0_gp_888, sizeof(expected_snapshot));
        g_transform_reset_status = -1;
        backend_calls = 0;
        mutate_backend = (iteration & 2) != 0;
        assert(func_800516F8(&selected) == 0 && selected == g_l0_gp_888);
        assert(func_80051678() == 0);
        assert(backend_calls == 1);
        if (mutate_backend) {
            expected_snapshot[0] = 0xABCDEF01u;
            assert(g_transform_reset_status == 0x12345678);
            assert(g_l0_gp_888 == NULL);
        } else {
            assert(g_transform_reset_status == 0);
        }
        assert(memcmp(g_transform_reset_snapshot, expected_snapshot, sizeof(expected_snapshot)) == 0);
    }
    puts("Transform reset: source snapshot, callback order and mutations passed");
    return 0;
}
