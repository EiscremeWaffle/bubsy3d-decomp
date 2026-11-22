#include "actor_context.h"
#include "game_transition.h"

typedef struct GameObjectQueueView {
    uint8_t unused_00_to_0F[0x10];
    int32_t pending_value;
    uint8_t unused_14_to_1F[0x0C];
    void *state;
} GameObjectQueueView;

typedef struct GameObjectQueueStateView {
    uint8_t unused_00_to_1F[0x20];
    int32_t pending_value;
} GameObjectQueueStateView;

extern uint32_t g_queue_counter_144;
extern uint32_t g_queue_counter_148;
extern int32_t g_queue_counter_14C;
extern uint32_t g_queue_value_150;
extern uint8_t g_queue_mode;
extern int16_t g_queue_transition_state;
extern const char g_queue_message[];
extern GameTransitionQueue g_transition_queue_input;
extern GameTransitionQueue g_transition_queue_snapshot;
extern GameTransitionStaticHandlers g_transition_static_handlers[256];
extern const GameTransitionModeHandlers *g_transition_mode_handlers;
extern const GameTransitionHeadingTable *g_transition_heading_handlers;

extern void *func_80028764(int32_t index);
extern int32_t func_8003AA1C(void **output);
extern void func_800443F8(void);
extern void func_8003539C(void *actor, int32_t update);
extern void func_80055680(const char *message);
extern void func_80032D7C(void);
extern void func_8001B8B8(GameTransitionQueue *input, GameTransitionQueue *output);
extern void func_8001B044(void);
extern uint32_t func_8006BB84(int32_t mode);
extern void func_800250CC(void);
extern uint32_t func_800439EC(void);
extern void func_80044268(void);
extern void func_8001B0A0(void);
extern void func_80029164(void);
extern int32_t func_80032C8C(int32_t mode);

void func_8001B1E4(void *actor, void *context) {
    void *actor_context = func_80026648(actor, context);
    GameObjectQueueView *object = func_80028764(0);
    uint8_t dispatch_generated_event = 0;
    int32_t snapshot_count;
    volatile GameTransitionEventRecord *current_record;
    union {
        void *record;
        uint32_t reserved[4];
    } runtime_storage;

    func_8003AA1C(&runtime_storage.record);
    if (*((uint8_t *)runtime_storage.record + 0x29) == 2) {
        ++g_queue_counter_14C;
    } else {
        g_queue_counter_14C = 0;
    }

    if (g_queue_counter_148 + 0x14 < g_queue_counter_144) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile(".set noreorder");
#endif
        func_800443F8();
        object->pending_value = 0;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile(".set reorder");
#endif
        func_8003539C(object, 1);
    } else if (g_queue_counter_14C >= 0x3D) {
        *((uint8_t *)runtime_storage.record + 0x29) = 0;
        ((GameObjectQueueStateView *)object->state)->pending_value = 0;
        func_80055680(g_queue_message);
    }

    if (g_queue_mode != 1) {
        ++g_queue_counter_144;
    }
    func_80032D7C();
    func_8001B8B8(&g_transition_queue_input, &g_transition_queue_snapshot);
    snapshot_count = (int32_t)g_transition_queue_snapshot.count;

    /* Target-only instructions preserve the original MIPS-I load and slot schedule. */
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("nop\n\t.set noreorder");
#endif
    if (snapshot_count > 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register int32_t record_index __asm__("$16") = 0;
        register int32_t record_offset __asm__("$17") = 0;
#else
        int32_t record_index = 0;
        int32_t record_offset = 0;
#endif
        do {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register int32_t event_id __asm__("$2");
            register int32_t expected_event_id __asm__("$3");
            register int32_t heading __asm__("$5");
#else
            uint32_t event_id;
            uint32_t expected_event_id = 4;
            int32_t heading;
#endif
            current_record = (volatile GameTransitionEventRecord *)
                ((uintptr_t)g_transition_queue_snapshot.records + record_offset);
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            __asm__ volatile(
                "ori %0, $zero, 4\n\t"
                "lhu %1, 0(%2)"
                : "=r"(expected_event_id), "=r"(event_id)
                : "r"(current_record));
            heading = current_record->heading;
#else
            event_id = current_record->event_id;
            heading = current_record->heading;
#endif

            if (event_id == expected_event_id) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                __asm__ volatile("nop");
#endif
                if (current_record->payload.opaque[0] == 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                    __asm__ volatile("nop");
#endif
                    dispatch_generated_event = 1;
                }
            }

            if (g_transition_queue_snapshot.count == 0) {
                break;
            }
            {
                GameTransitionRecordHandler record_handler;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                __asm__ volatile("nop");
#endif
                if (heading == 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                    __asm__ volatile("nop");
#endif
                    g_queue_counter_148 = g_queue_counter_144;
                }

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                __asm__ volatile("" : : : "memory");
#endif
                if (((uint32_t)g_queue_mode - 1u) < 2u &&
                    current_record->mode != 0) {
                    func_8001B044();
                } else if (heading == 0xFFFF) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                    register int32_t handler_event_id __asm__("$2");
                    __asm__ volatile(
                        "lhu %0, 0(%1)"
                        : "=r"(handler_event_id)
                        : "r"(current_record));
#else
                    int32_t handler_event_id = current_record->event_id;
#endif
                    record_handler =
                        g_transition_static_handlers[handler_event_id]
                            .mode_5_heading_ffff;
                    goto dispatch_record;
                } else if (heading == 0xFFFE || heading == 0xFFFD) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                    register int32_t handler_event_id __asm__("$3");
                    __asm__ volatile(
                        "lhu %0, 0(%1)"
                        : "=r"(handler_event_id)
                        : "r"(current_record));
#else
                    int32_t handler_event_id = current_record->event_id;
#endif
                    record_handler =
                        g_transition_mode_handlers[handler_event_id]
                            .heading.heading_fffe;
                    goto dispatch_record;
                } else if (heading == 0) {
                    goto dispatch_heading_handler;
                } else {
                    event_id = 0x17;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                    __asm__ volatile("" : : : "memory");
#endif
                    if (current_record->event_id == event_id ||
                        current_record->mode < 2 ||
                        (int32_t)g_queue_transition_state != (heading & 1)) {
                        goto dispatch_heading_handler;
                    }
                    func_8001B044();
                }
                goto dispatch_complete;

dispatch_heading_handler:
                {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                    register const GameTransitionHeadingTable *heading_table
                        __asm__("$3") = g_transition_heading_handlers;
                    register uint32_t heading_offset __asm__("$2") =
                        (uint32_t)heading << 3;
                    record_handler = *(GameTransitionRecordHandler *)
                        ((uintptr_t)heading_offset + (uintptr_t)heading_table +
                         0x10);
#else
                    record_handler = (GameTransitionRecordHandler)
                        g_transition_heading_handlers->entries[heading].handler;
#endif
                    if (record_handler == 0) {
                        goto dispatch_complete;
                    }
                    goto dispatch_record;
                }

dispatch_record:
                record_handler((GameTransitionEventRecord *)current_record);
dispatch_complete:
                ;
            }
            record_offset += sizeof(GameTransitionQueueRecord);
        } while (++record_index < snapshot_count);
    }
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile(".set reorder");
#endif

    if (g_queue_mode == 0) {
        uint16_t counter = (uint16_t)g_queue_counter_144;
        func_8006BB84(1);
        if (dispatch_generated_event == 0) {
            func_800250CC();
        }
        if (*((uint8_t *)actor_context + 0x0D) == 0) {
            if ((counter & 1) != 0) {
                g_queue_value_150 = func_800439EC() & 0xFF;
            } else if (g_queue_value_150 == 0) {
                func_80044268();
            }
            func_8001B0A0();
        }
    }
    func_80029164();
    func_80032C8C((g_queue_mode ^ 1) < 1);
    g_queue_transition_state =
        (int16_t)(g_queue_transition_state == 0);
}
