#ifndef BUBSY3D_ACTOR_EVENTS_H
#define BUBSY3D_ACTOR_EVENTS_H

#include <stdint.h>

enum {
    BUBSY_EVENT_PROPERTY_NORMAL = 0x35,
    BUBSY_EVENT_PROPERTY_SWIM_MODE = 0x54,
    BUBSY_EVENT_PROPERTY_KIND_157 = 0x157,
    BUBSY_EVENT_PROPERTY_KIND_3E0 = 0x3E0,
    BUBSY_EVENT_SPECIAL_FLAG = 0x40000000u
};

typedef struct BubsyActorEventOps {
    void *context;
    void *actor_component;
    uint8_t runtime_mode;
    uint8_t state_6463;
    uint16_t event_id;
    void (*select_property)(void *context, void *actor_component, uint16_t property_id);
    void (*set_active_flag)(void *context, void *actor_component);
    int32_t (*dispatch)(void *context, void *actor_component, uint32_t flags);
} BubsyActorEventOps;

int32_t bubsy_process_actor_event(uint8_t update_mode, const BubsyActorEventOps *ops);

#endif
