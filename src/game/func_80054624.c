#include "component_rotation.h"

extern ComponentRotationMatrix g_component_allocation_template;
extern void *func_80082EEC(uint32_t size);

int32_t func_80054624(void **output) {
    ComponentRotationMatrix *allocated = func_80082EEC(sizeof(*allocated));

    *output = allocated;
    if (allocated == 0) {
        return 3;
    }
    *allocated = g_component_allocation_template;
    return 0;
}