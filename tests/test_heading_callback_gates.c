#include "heading_callback_gates.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

volatile uint8_t g_grounded_case_event;
int16_t g_actor_environment;
uint8_t g_heading_context_gate;
const char g_heading_gate_condition[] = "condition";
const char g_heading_gate_source_path[] = "source";

static unsigned assertions;
static uint8_t replacement;

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0);
    assert(condition == g_heading_gate_condition);
    assert(source == g_heading_gate_source_path);
    assert(line == 0x8D7);
    ++assertions;
    g_heading_context_gate = replacement;
}

int main(void) {
    unsigned value;

    for (value = 0; value < 256; ++value) {
        g_grounded_case_event = (uint8_t)value;
        assert(func_8002E824() == value);
    }
    func_8002E7D8();
    assert(func_8002E824() == 0);
    func_8002E7FC();
    assert(func_8002E824() == 1);

    for (value = 0; value < 65536; ++value) {
        g_actor_environment = (int16_t)value;
        g_heading_context_gate = (uint8_t)value;
        replacement = (uint8_t)(value ^ 0xA5);
        assertions = 0;
        assert(func_8002B620() == (value == 19 ? (uint8_t)value : replacement));
        assert(assertions == (unsigned)(value != 19));
    }
    puts("heading callback gate tests passed");
    return 0;
}
