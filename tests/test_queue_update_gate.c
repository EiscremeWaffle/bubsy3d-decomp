#include "queue_update.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

uint8_t g_unknown_800BE024;
ComponentRotationMatrix g_unknown_80186154;

static void *resolved_source;
static int call_count;
static int mutate_flag;

int32_t func_800516F8(void **output) {
    assert(call_count == 0);
    ++call_count;
    *output = resolved_source;
    if (mutate_flag) {
        g_unknown_800BE024 = 0;
    }
    return -1;
}

int32_t func_80054A6C(
    ComponentRotationMatrix *source, const ComponentRotationMatrix *destination
) {
    assert(call_count == 1);
    ++call_count;
    assert(source == resolved_source);
    assert(destination == &g_unknown_80186154);
    return -1;
}

int32_t func_8003ABE8(void) {
    assert(call_count == 2);
    ++call_count;
    return -1;
}

int main(void) {
    ComponentRotationMatrix data;
    int flag;

    for (flag = 0; flag <= 255; ++flag) {
        g_unknown_800BE024 = (uint8_t)flag;
        resolved_source = &data;
        mutate_flag = 0;
        call_count = 0;
        assert(func_80032D7C() == 1);
        assert(call_count == (flag == 0 ? 0 : 3));
    }

    g_unknown_800BE024 = 1;
    resolved_source = 0;
    mutate_flag = 1;
    call_count = 0;
    assert(func_80032D7C() == 1);
    assert(g_unknown_800BE024 == 0);
    assert(call_count == 3);

    puts("Queue-update gate: all byte values, callback order and mutations passed.");
    return 0;
}
