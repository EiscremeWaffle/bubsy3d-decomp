#include "controller_input.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

volatile uint8_t g_controller_packets[2][8];
int32_t g_input_stream_sample;

static int bios_phase;

void func_8006BA84(
    volatile uint8_t *first, int32_t first_size,
    volatile uint8_t *second, int32_t second_size
) {
    assert(bios_phase == 0);
    assert(first == g_controller_packets[0] && first_size == 8);
    assert(second == g_controller_packets[1] && second_size == 8);
    bios_phase = 1;
}

void func_8006BA94(void) {
    assert(bios_phase == 1);
    bios_phase = 2;
}

static void set_packet(unsigned int port, uint8_t status, uint8_t type, uint16_t raw) {
    g_controller_packets[port][0] = status;
    g_controller_packets[port][1] = type;
    g_controller_packets[port][2] = (uint8_t)(raw >> 8);
    g_controller_packets[port][3] = (uint8_t)raw;
}

int main(void) {
    uint32_t output;
    uint32_t value;
    static const int32_t samples[] = {INT32_MIN, -32768, -1, 0, 1, 32767, INT32_MAX};

    assert(func_8004FCBC() == 0 && bios_phase == 2);
    for (value = 0; value < sizeof(samples) / sizeof(samples[0]); ++value) {
        assert(func_8004FF28(samples[value]) == 0);
        assert(g_input_stream_sample == samples[value]);
    }
    set_packet(0, 0xFF, CONTROLLER_DIGITAL_TYPE, 0);
    set_packet(1, 0xFF, CONTROLLER_DIGITAL_TYPE, 0);
    assert(func_8004FCF8(&output) == CONTROLLER_NONE && output == 0);

    for (value = 1; value < 256; ++value) {
        set_packet(0, (uint8_t)value, CONTROLLER_DIGITAL_TYPE, 0);
        assert(func_8004FCF8(&output) == CONTROLLER_NONE && output == 0);
    }
    for (value = 0; value < 256; ++value) {
        if (value == CONTROLLER_DIGITAL_TYPE) {
            continue;
        }
        set_packet(0, 0, (uint8_t)value, 0);
        assert(func_8004FCF8(&output) == CONTROLLER_NONE && output == 0);
    }

    for (value = 0; value < 65536; ++value) {
        uint16_t first = (uint16_t)value;
        uint16_t second = (uint16_t)(value * 37u + 19u);
        uint32_t expected = (uint16_t)~first | ((uint32_t)(uint16_t)~second << 16);

        set_packet(0, 0, CONTROLLER_DIGITAL_TYPE, first);
        set_packet(1, 0, CONTROLLER_DIGITAL_TYPE, second);
        assert(func_8004FCF8(&output) == CONTROLLER_BOTH && output == expected);

        set_packet(1, 1, CONTROLLER_DIGITAL_TYPE, second);
        assert(func_8004FCF8(&output) == CONTROLLER_PORT_1_ONLY);
        assert(output == (uint16_t)~first);

        set_packet(0, 1, CONTROLLER_DIGITAL_TYPE, first);
        set_packet(1, 0, CONTROLLER_DIGITAL_TYPE, second);
        assert(func_8004FCF8(&output) == CONTROLLER_PORT_2_ONLY);
        assert(output == ((uint32_t)(uint16_t)~second << 16));
    }

    puts("Controller input: all 16-bit button patterns, both ports, status and type gates passed.");
    return 0;
}
