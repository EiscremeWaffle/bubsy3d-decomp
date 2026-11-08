#include "game_transition.h"

extern GameTransitionEventRecord g_transition_event_records[100];
extern int32_t g_transition_event_count;
extern GameTransitionStaticHandlers g_transition_static_handlers[256];
extern const GameTransitionModeHandlers *g_transition_mode_handlers;
extern const GameTransitionHeadingTable *g_transition_heading_handlers;
extern const char g_transition_overflow_condition[];
extern const char g_transition_source_path[];

extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

void func_8001B558(
    int32_t event_id_input,
    int32_t mode_input,
    uintptr_t flags_input,
    int32_t heading_input,
    void *argument_4_input,
    void *argument_5_input,
    void *argument_6_input,
    void *argument_7_input
) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t flags __asm__("$19") = (int32_t)flags_input;
    register int32_t heading __asm__("$17") = heading_input;
    register int32_t event_id __asm__("$18") = event_id_input;
    register int32_t mode __asm__("$20") = mode_input;
    register void *argument_4 __asm__("$21") = argument_4_input;
    register void *argument_5 __asm__("$22") = argument_5_input;
    void *argument_6 = argument_6_input;
    register void *argument_7 __asm__("$23") = argument_7_input;
    int32_t record_index = -1;
    GameTransitionEventRecord *record;
#else
    int32_t event_id = event_id_input;
    int32_t mode = mode_input;
    int32_t flags = (int32_t)flags_input;
    int32_t heading = heading_input;
    void *argument_4 = argument_4_input;
    void *argument_5 = argument_5_input;
    void *argument_6 = argument_6_input;
    void *argument_7 = argument_7_input;
    int32_t record_index = -1;
    GameTransitionEventRecord *record;
#endif
    GameTransitionPayload payload;
    GameTransitionPayloadHandler payload_handler;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t current_count __asm__("$3");
    register int32_t has_capacity __asm__("$2");
    register uint32_t event_count __asm__("$5");
    register uint32_t match_event_id __asm__("$6");
    int32_t scan_index;
    int32_t record_position;
#else
    int32_t current_count;
    int32_t has_capacity;
    uint32_t event_count;
    uint32_t match_event_id;
    int32_t scan_index;
    int32_t record_position;
#endif
    for (scan_index = 0; scan_index < 100; ++scan_index) {
        if (g_transition_event_records[scan_index].event_id == 0) {
            record_index = scan_index;
            break;
        }
    }
    if (record_index == -1) {
        func_800556D0(1, g_transition_overflow_condition, g_transition_source_path, 0x1F9);
    }
    record = &g_transition_event_records[record_index];

    if (flags == 0xFFFF) {
        if (g_transition_static_handlers[(uint8_t)event_id].flags_ffff != 0) {
            g_transition_static_handlers[(uint8_t)event_id].flags_ffff(
                record, argument_4, argument_5, argument_6, argument_7);
        }
    } else if (flags == 0xFFFE) {
        if (g_transition_mode_handlers[(uint8_t)event_id].flags_fffe != 0) {
            g_transition_mode_handlers[(uint8_t)event_id].flags_fffe(
                record, argument_4, argument_5, argument_6, argument_7);
        }
    }

    if ((uint8_t)mode == 5) {
        if (heading == 0xFFFF) {
            g_transition_static_handlers[(uint8_t)event_id].mode_5_heading_ffff(record);
            goto done;
        }
        if (heading == 0xFFFE) {
            g_transition_mode_handlers[(uint8_t)event_id].heading.heading_fffe(record);
            goto done;
        }
        if (heading == 0xFFFD) {
            payload.event_id = (uint8_t)event_id;
            payload.mode = (uint8_t)mode;
            payload.flags = flags;
            payload.heading = heading;
            payload_handler = g_transition_mode_handlers[(uint8_t)event_id].heading.heading_fffd;
            payload_handler(&payload);
            goto done;
        }
        {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register const GameTransitionHeadingTable *heading_base __asm__("$3") = g_transition_heading_handlers;
#else
            const GameTransitionHeadingTable *heading_base = g_transition_heading_handlers;
#endif
            payload.event_id = (uint8_t)event_id;
            payload.mode = (uint8_t)mode;
            payload.flags = flags;
            payload.heading = heading;
            /* Operand order preserves the original address-register allocation. */
            payload_handler = (heading + heading_base->entries)->handler;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            __asm__("" : "=r"(payload_handler) : "0"(payload_handler), "r"(heading_base));
#endif
        }
        if (payload_handler != 0) {
            payload_handler(&payload);
        }
        goto done;
    }

    current_count = g_transition_event_count;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__("" : "=r"(current_count) : "0"(current_count));
#endif
    has_capacity = current_count < 100;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__("" : "=r"(has_capacity) : "0"(has_capacity));
#endif
    event_count = (uint32_t)current_count;
    if (!has_capacity) {
        goto done;
    }
    scan_index = 0;
    match_event_id = (uint8_t)event_id;
    record_position = 0;
    /* Keep the scan count and record cursor separate for the original loop layout. */
    for (; scan_index < 100;) {
        uint16_t active_event_id;
        if (event_count == 0) {
            break;
        }
        active_event_id = g_transition_event_records[record_position].event_id;
        ++scan_index;
        if (active_event_id != 0) {
            --event_count;
            if (active_event_id == match_event_id &&
                g_transition_event_records[record_position].flags == flags &&
                g_transition_event_records[record_position].heading == heading &&
                heading != 0xFFFE) {
                goto done;
            }
        }
        ++record_position;
    }

    {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register uint32_t queued_event_id __asm__("$2") = (uint8_t)event_id;
        __asm__("" : "=r"(queued_event_id) : "0"(queued_event_id));
        record->event_id = (uint16_t)queued_event_id;
#else
        record->event_id = (uint8_t)event_id;
#endif
    }
    {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register uint32_t mode_byte __asm__("$2") = (uint8_t)mode;
        register int32_t *count_address __asm__("$3");
        __asm__ volatile("" : "=r"(mode_byte) : "0"(mode_byte) : "memory");
        count_address = &g_transition_event_count;
        __asm__("" : "=r"(count_address) : "0"(count_address));
#else
        uint32_t mode_byte = (uint8_t)mode;
        int32_t *count_address = &g_transition_event_count;
#endif
        record->mode = (uint16_t)mode_byte;
        record->flags = flags;
        record->heading = heading;
        *count_address = (int32_t)((uint32_t)*count_address + 1u);
    }

done:
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : : "r"(event_id), "r"(mode), "r"(flags), "r"(heading));
#endif
    return;
}
