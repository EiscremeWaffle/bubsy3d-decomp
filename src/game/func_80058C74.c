#include "component_bounds.h"

extern int32_t func_800529F8(ComponentRecordOwner *object, int32_t *index);

int32_t func_80058C74(ComponentRecordOwner *object, ComponentBounds *bounds) {
    int32_t index;
    uint32_t saved_y;
    int32_t status = 0;

    status = func_800529F8(object, &index);
    if (status != 0) {
        return status;
    }
    status = func_80052094(object, (uint32_t)index, bounds->words);
    if (status != 0) {
        return status;
    }
    saved_y = bounds->vectors.lower.y;
    bounds->vectors.lower.y = 0U - bounds->vectors.upper.y;
    bounds->vectors.upper.y = 0U - saved_y;
    return 0;
}
