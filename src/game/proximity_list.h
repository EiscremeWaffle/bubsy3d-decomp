#ifndef BUBSY3D_PROXIMITY_LIST_H
#define BUBSY3D_PROXIMITY_LIST_H

#include <stdint.h>

typedef struct ProximityPosition {
    uint8_t reserved_00[0x14];
    int32_t x;
    int32_t y;
    int32_t z;
} ProximityPosition;

typedef struct ProximityObject {
    uint8_t reserved_00[0x14];
    ProximityPosition *position;
    uint8_t reserved_18[6];
    int16_t radius;
} ProximityObject;

typedef struct ProximityNode {
    ProximityObject *object;
    uint8_t reserved_04[4];
    struct ProximityNode *next;
} ProximityNode;

extern ProximityObject *g_reference_object;
extern ProximityNode *g_object_list_head;
extern uint16_t g_proximity_count;
extern ProximityObject *g_proximity_objects[];
extern const char g_proximity_limit_condition[];
extern const char g_proximity_source_path[];

void func_80053270(void *object);
ProximityObject *func_8005327C(void);
int32_t func_800529D0(ProximityNode **output);
void func_80069758(void);

#endif
