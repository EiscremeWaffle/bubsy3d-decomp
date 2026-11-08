#include "input_events.h"

#include <stdint.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stdio.h>

uint8_t g_actor_update_readiness;
uint8_t g_event_runtime_byte;
uint32_t g_runtime_sample_counter;

extern void func_8001AE88(uint32_t *output);
extern int32_t func_800222E0(void);
extern void func_8002C488(void);

static int32_t observed_indices[19];
static unsigned calls;

void func_8002BAD4(int32_t index) {
    assert(calls < sizeof(observed_indices) / sizeof(observed_indices[0]));
    observed_indices[calls++] = index;
    g_runtime_sample_counter += (uint32_t)index;
    g_actor_update_readiness = (uint8_t)index;
}

int main(void) {
    static const uint32_t samples[] = {0, 1, 0x7FFFFFFFu, 0x80000000u, UINT32_MAX};
    unsigned index;
    uint32_t output;

    for (index = 0; index < 256; ++index) {
        g_actor_update_readiness = (uint8_t)index;
        g_event_runtime_byte = (uint8_t)index;
        assert(func_800222E0() == (int32_t)index);
        assert(func_8001BCE4() == (uint8_t)index);
    }
    for (index = 0; index < sizeof(samples) / sizeof(samples[0]); ++index) {
        g_runtime_sample_counter = samples[index];
        output = 0xA5A5A5A5u;
        func_8001AE88(&output);
        assert(output == samples[index]);
        func_8001AE88(&g_runtime_sample_counter);
        assert(g_runtime_sample_counter == samples[index]);
    }

    calls = 0;
    g_runtime_sample_counter = 0;
    g_actor_update_readiness = 0xFF;
    func_8002C488();
    assert(calls == 19);
    for (index = 0; index < 19; ++index) {
        assert(observed_indices[index] == (int32_t)index);
    }
    assert(g_runtime_sample_counter == 171);
    assert(g_actor_update_readiness == 18);
    puts("Grounded runtime helpers: byte values, sample boundaries and 19 ordered callbacks passed.");
    return 0;
}
