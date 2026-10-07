#include "bubsy_actor_events.h"

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

    ops->select_property(
        ops->context,
        ops->actor_component,
        ops->runtime_mode != 0 ? BUBSY_EVENT_PROPERTY_KIND_157 : BUBSY_EVENT_PROPERTY_KIND_3E0
    );
    ops->set_active_flag(ops->context, ops->actor_component);
    return ops->dispatch(ops->context, ops->actor_component, BUBSY_EVENT_SPECIAL_FLAG);
}
