#include "bubsy_actor_events.h"

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
register volatile uint8_t *bubsy_runtime_globals __asm__("$28");
extern int32_t func_80052ABC(void *component, int32_t *event_out);
extern int32_t func_80052AEC(void *component, int32_t property_id);
extern int32_t func_80052E18(void *component);
extern int32_t func_800532CC(void *component, int32_t active, int32_t argument_2, int32_t argument_3, uint32_t flags);

int32_t bubsy_process_actor_event(int32_t update_mode, void *actor) {
    register void *actor_value __asm__("$16") = actor;
    register int32_t property_id __asm__("$5");
    register uintptr_t state_base __asm__("$3");
    register int32_t event_value __asm__("$3");
    int32_t event_data[2];
    register void *component_value __asm__("$4");

    switch (update_mode) {
        case 0: break;
        case 1: goto mode_one;
        default: return 1;
    }
    func_80052ABC(*(void * volatile *)((uint8_t *)actor_value + 0x1C), event_data);
    {
        register uint32_t runtime_mode __asm__("$2") = bubsy_runtime_globals[0x7EC];
        __asm__ volatile("" : "=r"(runtime_mode) : "0"(runtime_mode));
        if (runtime_mode != 0) {
            goto mode_zero_swim;
        }
    }
    {
        register int32_t expected_event __asm__("$2") = BUBSY_EVENT_PROPERTY_KIND_3E0;
        event_value = event_data[0];
        if (event_value != expected_event) {
            goto mode_zero_swim;
        }
        __asm__ volatile("" : "=r"(expected_event) : "0"(expected_event));
        expected_event = 3;
        __asm__ volatile("" : : : "memory");
        state_base = 0x80180000u;
        __asm__ volatile("" : "=r"(state_base) : "0"(state_base));
        event_value = *(volatile uint8_t *)(state_base + 0x6463);
        __asm__ volatile("" : "=r"(event_value) : "0"(event_value));
        if (event_value == expected_event) {
            goto mode_zero_swim;
        }
    }
    func_80052AEC(*(void * volatile *)((uint8_t *)actor_value + 0x1C), BUBSY_EVENT_PROPERTY_NORMAL);
    func_80052E18(*(void * volatile *)((uint8_t *)actor_value + 0x1C));
    return func_800532CC(*(void * volatile *)((uint8_t *)actor_value + 0x1C), 1, 0, 0, 0);

mode_zero_swim:
    {
        register uint32_t runtime_mode __asm__("$3") = bubsy_runtime_globals[0x7EC];
        register int32_t mode_one_value __asm__("$2") = 1;
        __asm__ volatile("" : "=r"(runtime_mode) : "0"(runtime_mode));
        if (runtime_mode != mode_one_value) {
            return BUBSY_EVENT_PROPERTY_KIND_157;
        }
    }
    {
        register int32_t expected_event __asm__("$2") = BUBSY_EVENT_PROPERTY_KIND_157;
        event_value = event_data[0];
        if (event_value != expected_event) {
            return expected_event;
        }
    }
    func_80052AEC(*(void * volatile *)((uint8_t *)actor_value + 0x1C), BUBSY_EVENT_PROPERTY_SWIM_MODE);
    func_80052E18(*(void * volatile *)((uint8_t *)actor_value + 0x1C));
    __asm__ volatile("" : : : "$8", "memory");
    return func_800532CC(*(void * volatile *)((uint8_t *)actor_value + 0x1C), 1, 0, 0, 0);
mode_one:
    if (bubsy_runtime_globals[0x7EC] == 0) {
        component_value = *(void * volatile *)((uint8_t *)actor_value + 0x1C);
        property_id = BUBSY_EVENT_PROPERTY_KIND_3E0;
    } else {
        component_value = *(void * volatile *)((uint8_t *)actor_value + 0x1C);
        property_id = BUBSY_EVENT_PROPERTY_KIND_157;
    }
mode_one_select:
    func_80052AEC(component_value, property_id);
    func_80052E18(*(void * volatile *)((uint8_t *)actor_value + 0x1C));
    return func_800532CC(*(void * volatile *)((uint8_t *)actor_value + 0x1C), 1, 0, 0, BUBSY_EVENT_SPECIAL_FLAG);
}
#else
int32_t bubsy_process_actor_event(uint8_t update_mode, const BubsyActorEventOps *ops) {
    if (update_mode > 1) {
        return 1;
    }

    if (update_mode == 0 && ops->runtime_mode == 0 &&
        ops->event_id == BUBSY_EVENT_PROPERTY_KIND_3E0 && ops->state_6463 != 3) {
        ops->select_property(ops->context, ops->actor_component, BUBSY_EVENT_PROPERTY_NORMAL);
        ops->set_active_flag(ops->context, ops->actor_component);
        return ops->dispatch(ops->context, ops->actor_component, 0);
    }

    if (update_mode == 0 && ops->runtime_mode == 1 &&
        ops->event_id == BUBSY_EVENT_PROPERTY_KIND_157) {
        ops->select_property(ops->context, ops->actor_component, BUBSY_EVENT_PROPERTY_SWIM_MODE);
        ops->set_active_flag(ops->context, ops->actor_component);
        return ops->dispatch(ops->context, ops->actor_component, 0);
    }

    if (update_mode == 0) {
        return BUBSY_EVENT_PROPERTY_KIND_157;
    }

    ops->select_property(
        ops->context,
        ops->actor_component,
        ops->runtime_mode != 0 ? BUBSY_EVENT_PROPERTY_KIND_157 : BUBSY_EVENT_PROPERTY_KIND_3E0
    );
    ops->set_active_flag(ops->context, ops->actor_component);
    return ops->dispatch(ops->context, ops->actor_component, BUBSY_EVENT_SPECIAL_FLAG);
}
#endif
