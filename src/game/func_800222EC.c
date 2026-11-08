#include "move_preparation.h"

extern void *func_80082EEC(uint32_t size);

uint8_t func_800222EC(MovePreparationInfo **output, const MovePreparationVector *origin) {
    MovePreparationInfo *allocated = func_80082EEC(0xA8);
    int32_t sample;

    *output = allocated;
    if (allocated == 0) {
        return 0;
    }
    allocated->index = -1;
    (*output)->separation_18 = 0;
    (*output)->counter = 0;
    (*output)->target.x = 0;
    (*output)->target.y = 0;
    (*output)->target.z = 0;
    (*output)->auxiliary_78.x = 0;
    (*output)->auxiliary_78.y = 0;
    (*output)->auxiliary_78.z = 0;
    allocated = *output;
    sample = origin->x;
    allocated->origin_58.x = sample;
    allocated->origin_copy_68.x = sample;
    allocated = *output;
    sample = origin->y;
    allocated->origin_58.y = sample;
    allocated->origin_copy_68.y = sample;
    allocated = *output;
    sample = origin->z;
    allocated->origin_58.z = sample;
    allocated->origin_copy_68.z = sample;
    (*output)->pending_38.x = 0;
    (*output)->pending_38.y = 0;
    (*output)->pending_38.z = 0;
    (*output)->auxiliary_98[0] = 0;
    (*output)->auxiliary_98[1] = 0;
    (*output)->auxiliary_98[2] = 0;
    (*output)->rotation_48.x = 0;
    (*output)->rotation_48.y = 0;
    (*output)->rotation_48.z = 0;
    (*output)->stride_30 = 0;
    (*output)->unknown_34 = 0;
    (*output)->transform_88.x = 0;
    (*output)->transform_88.y = 0;
    (*output)->transform_88.z = 0;
    return 1;
}
