#include "component_rotation.h"
#include "level_records.h"

extern const ComponentRotationMatrix g_level_default_rotation;

int32_t func_800289E8(int32_t index, ComponentRotationMatrix *output) {
    int32_t row;
    int32_t column;
    int32_t next = 0;
    uint16_t *element;
    uint16_t *destination;

    if (index >= g_level_record_count || index < 0) {
        return 0;
    }

    if (g_level_records[index].rotation_08 == 0) {
        *output = g_level_default_rotation;
    } else {
        uintptr_t record_offset;

        row = 0;
        record_offset = (uintptr_t)index * sizeof(GameLevelRecord);
        destination = output->elements;
        do {
            column = 0;
            element = destination;
            do {
                /* Preserve the original byte-offset calculation and live table read. */
                *element++ = ((const GameLevelRecord *)
                    (record_offset + (uintptr_t)g_level_records))->rotation_08[next++];
                column++;
            } while (column < 3);
            destination += 3;
            row++;
        } while (row < 3);
    }

    ((volatile ComponentRotationMatrix *)output)->translation[0] =
        g_level_records[index].position_0C->words[0];
    ((volatile ComponentRotationMatrix *)output)->translation[1] =
        g_level_records[index].position_0C->words[1];
    ((volatile ComponentRotationMatrix *)output)->translation[2] =
        g_level_records[index].position_0C->words[2];
    return 1;
}
