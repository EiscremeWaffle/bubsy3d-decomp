/* Heading effect updates for the controller callback. */
#include "component_fields.h"
#include "controller_callback_helpers.h"
#include "heading_common_gates.h"
#include "heading_effects.h"
#include "heading_entry_gates.h"
#include "sequence_queries.h"

#ifndef BUBSY3D_MATCH_ORIGINAL_L0
#include <stdio.h>
#include <stdlib.h>
#endif

struct ComponentBasisView;
struct ComponentTransformView;

extern void func_8001AE88(uint32_t *output);
extern void func_8002CA88(int32_t value, int32_t duration);
extern void func_8002CB2C(int32_t value);
extern int32_t func_80052BDC(void *handle, int32_t mode);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
extern void *g_heading_second_handle;
#define HEADING_SECOND_HANDLE g_heading_second_handle
#define KEEP_VALUE(value) __asm__ volatile("" : "=r"(value) : "0"(value))
#define KEEP_INPUT(value) __asm__ volatile("" : : "r"(value))
#else
#define HEADING_SECOND_HANDLE g_actor_handles[1]
#define KEEP_VALUE(value) ((void)0)
#define KEEP_INPUT(value) ((void)0)
#endif
extern int32_t func_80054B20(
    const struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    int32_t output[3]
);
extern int32_t func_80054C7C(
    struct ComponentTransformView *transform,
    const struct ComponentBasisView *basis,
    const int32_t input[3]
);

void func_80039AB4(BubsyGroundedActor *actor, void *transform) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t effect __asm__("$18") = g_heading_active_byte;
    register uint32_t current_effect __asm__("$2");
    register HeadingComponentView *handle __asm__("$17");
    register int32_t sequence __asm__("$19") = -1;
    register int32_t offset __asm__("$16");
    register uint32_t duration __asm__("$20");
    register uint32_t completion_effect __asm__("$3");
    register uint32_t completion_two __asm__("$2");
#else
    uint32_t effect = g_heading_active_byte;
    uint32_t current_effect;
    HeadingComponentView *handle = 0;
    int32_t sequence = -1;
    int32_t offset = 0;
    uint32_t duration = 0;
    uint32_t completion_effect;
    uint32_t completion_two;
#endif
    int32_t position[3];
    uint32_t now;
    uint8_t previous;

    func_80052BDC(g_actor_handles[0], 1);
    func_80052BDC(HEADING_SECOND_HANDLE, 1);

#ifndef BUBSY3D_MATCH_ORIGINAL_L0
    if (effect > 4) {
        fprintf(stderr, "unsupported heading effect: %u\n", effect);
        abort();
    }
#endif

    KEEP_VALUE(effect);
    current_effect = (uint8_t)effect;
    KEEP_VALUE(current_effect);
    if (current_effect == 0) {
        if (g_heading_effect_started != 0) {
            if (g_heading_effect_previous == 3) {
                func_8001C700(4, 14);
                func_800532CC(actor->component_1C, 1, 0, 0, 0);
            }
            if (g_runtime_mode == 0) {
                func_8001C700(5, 5);
            } else {
                func_8001C700(5, 3);
            }
            func_8002CB2C(5);
            g_heading_effect_started = 0;
        }
        return;
    }

    previous = g_heading_effect_previous;
    if (current_effect != previous) {
        g_heading_effect_started = 0;
        if (previous == 2 || previous == 3) {
            if (g_runtime_mode == 0) {
                func_8001C700(5, 5);
            } else {
                func_8001C700(5, 3);
            }
            func_8002CB2C(5);
        }
        if (g_heading_effect_previous == 3) {
            func_8001C700(4, 14);
            func_800532CC(actor->component_1C, 1, 0, 0, 0);
        }
    }
    g_heading_effect_previous = (uint8_t)effect;

    if ((uint8_t)effect != 4 && g_heading_effect_latched == 1) {
        void *first_handle = g_actor_handles[0];
        g_heading_effect_latched = 0;
        g_heading_effect_started = 0;
        func_80052BDC(first_handle, 1);
        return;
    }

    if ((uint8_t)effect == 3) {
        handle = 0;
        duration = 1200;
    } else if ((uint8_t)effect == 1) {
        uint32_t effect_mode = g_runtime_mode;
        KEEP_VALUE(effect_mode);
        KEEP_VALUE(sequence);
        sequence = -1;
        if (effect_mode == 0) {
            offset = 0x40;
        } else {
            offset = 0x80;
            KEEP_VALUE(offset);
        }
        handle = g_actor_handles[0];
        duration = g_heading_effect_duration;
    } else if ((uint8_t)effect == 2) {
        if (g_runtime_mode == 0) {
            sequence = 5;
            offset = 0x20;
        } else {
            sequence = 3;
            offset = 0x60;
        }
        handle = HEADING_SECOND_HANDLE;
        duration = 1200;
    } else if ((uint8_t)effect == 4) {
        uint32_t effect_mode = g_runtime_mode;
        if (effect_mode != 0) {
            offset = 0x80;
        } else {
            offset = 0x40;
            KEEP_VALUE(offset);
        }
        handle = g_actor_handles[0];
        duration = 0xFFFFFF;
        g_heading_effect_latched = 1;
    }

    if (handle != 0) {
        func_80054B20(transform, 0, position);
        position[1] = (int32_t)((uint32_t)position[1] + (uint32_t)offset);
        func_80054C7C(handle->transform_14, 0, position);
        KEEP_INPUT(offset);
    }

    if (g_heading_effect_started == 0) {
        if (effect - 2U < 2U) {
            func_8002CA88(5, 20);
        }
        if (handle != 0) {
            func_80052BDC(handle, 0);
        }
        func_8001AE88(&g_heading_effect_started);
        KEEP_VALUE(sequence);
        if ((uint8_t)effect == 3) {
            func_8001C6BC(4, 14, -1);
            func_800532CC(actor->component_1C, 1, 0, 0, 0x40000000U);
            KEEP_INPUT(effect);
            KEEP_INPUT(sequence);
        }
        if ((int16_t)sequence != -1) {
            if (g_runtime_mode == 0) {
                func_8001C700(5, 5);
            } else {
                func_8001C700(5, 3);
            }
            KEEP_VALUE(sequence);
            func_8001C6BC(5, (int16_t)sequence, -1);
        }
        return;
    }

    offset = 0;
    if ((uint8_t)effect == 1 || (uint8_t)effect == 4) {
        func_8001AE88(&now);
        if (now - g_heading_effect_started > duration) {
            KEEP_INPUT(duration);
            offset = 1;
        }
    } else if (g_unknown_8009F58C == 0) {
        offset = 1;
    }

    if ((uint8_t)offset != 0) {
        completion_effect = (uint8_t)effect;
        if (completion_effect == 3) {
            if (g_runtime_state_61 != 2) {
                func_800532CC(actor->component_1C, 1, 0, 0, 0);
            }
            func_8001C700(4, 14);
            KEEP_VALUE(effect);
            completion_effect = (uint8_t)effect;
        }
        completion_two = 2;
        KEEP_INPUT(offset);
        KEEP_VALUE(sequence);
        if (completion_effect == completion_two) {
            func_8002CB2C(5);
            KEEP_INPUT(effect);
        }
        g_heading_active_byte = 0;
        g_heading_effect_started = 0;
        if ((int16_t)sequence != -1) {
            KEEP_INPUT(sequence);
            func_8001C700(5, (int16_t)sequence);
        }
    } else if (handle != 0) {
        func_80052BDC(handle, 0);
    }
}
