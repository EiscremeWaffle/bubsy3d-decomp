#include "proximity_list.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

ProximityObject *g_reference_object;
ProximityNode *g_object_list_head;
uint16_t g_proximity_count;
ProximityObject *g_proximity_objects[40];
const char g_proximity_limit_condition[] = "proximity condition";
const char g_proximity_source_path[] = "proximity source";

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(0 && "unexpected assertion callback");
}

int main(void) {
    ProximityPosition reference_position = {0}, near_position = {0}, far_position = {0};
    ProximityObject reference = {0}, near_object = {0}, far_object = {0};
    ProximityNode near_node = {0}, far_node = {0};
    ProximityNode *output = &far_node;

    reference.position = &reference_position;
    near_position.x = 8;
    near_position.z = -5;
    reference_position.x = 7;
    reference_position.z = -5;
    near_object.position = &near_position;
    near_object.radius = 2;
    far_position.x = 9;
    far_position.z = -5;
    far_object.position = &far_position;
    far_object.radius = 2;
    near_node.object = &near_object;
    near_node.next = &far_node;
    far_node.object = &far_object;

    g_reference_object = 0;
    g_object_list_head = 0;
    assert(func_8005327C() == 0);
    assert(func_800529D0(&output) == 0 && output == 0);
    g_proximity_count = 0xFFFF;
    func_80069758();
    assert(g_proximity_count == 0);

    g_reference_object = &reference;
    g_object_list_head = &near_node;
    assert(func_8005327C() == &reference);
    assert(func_800529D0(&output) == 0 && output == &near_node);
    assert(func_800529D0(&g_object_list_head) == 0 && g_object_list_head == &near_node);
    func_80069758();
    assert(g_proximity_count == 1 && g_proximity_objects[0] == &near_object);

    g_reference_object = 0;
    near_node.object = 0;
    far_node.object = 0;
    func_80069758();
    assert(g_proximity_count == 0);
    puts("proximity getter integration tests passed");
    return 0;
}
