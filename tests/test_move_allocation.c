#include "move_preparation.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct Allocation {
    MovePreparationInfo info;
    uint32_t tail;
} Allocation;

_Static_assert(sizeof(Allocation) == 0xA8, "allocation size");
_Static_assert(offsetof(MovePreparationInfo, origin_copy_68) == 0x68, "origin copy offset");
_Static_assert(offsetof(MovePreparationInfo, auxiliary_78) == 0x78, "auxiliary offset");
_Static_assert(offsetof(MovePreparationInfo, auxiliary_98) == 0x98, "tail vector offset");

static Allocation allocation;
static MovePreparationInfo *output;
static int fail;
static unsigned calls;

void *func_80082EEC(uint32_t size) {
    assert(size == 0xA8);
    assert(output == (MovePreparationInfo *)(void *)&calls);
    ++calls;
    return fail ? NULL : &allocation.info;
}

static void store(unsigned char *bytes, unsigned offset, int32_t value) {
    memcpy(bytes + offset, &value, sizeof(value));
}

static void expected_allocation(unsigned char bytes[0xA8], const MovePreparationVector *external,
    unsigned origin_offset) {
    static const unsigned initial_zero[] = {0x18, 0, 8, 12, 16, 0x78, 0x7C, 0x80};
    static const unsigned final_zero[] = {
        0x38, 0x3C, 0x40, 0x98, 0x9C, 0xA0, 0x48, 0x4C, 0x50,
        0x30, 0x34, 0x88, 0x8C, 0x90
    };
    unsigned i;
    store(bytes, 4, -1);
    for (i = 0; i < sizeof(initial_zero) / sizeof(initial_zero[0]); ++i) {
        store(bytes, initial_zero[i], 0);
    }
    for (i = 0; i < 3; ++i) {
        int32_t sample;
        if (external != NULL) {
            memcpy(&sample, (const unsigned char *)external + i * 4, sizeof(sample));
        } else {
            memcpy(&sample, bytes + origin_offset + i * 4, sizeof(sample));
        }
        store(bytes, 0x58 + i * 4, sample);
        store(bytes, 0x68 + i * 4, sample);
    }
    for (i = 0; i < sizeof(final_zero) / sizeof(final_zero[0]); ++i) {
        store(bytes, final_zero[i], 0);
    }
}

static void run_case(unsigned fill, const MovePreparationVector *external, unsigned origin_offset) {
    Allocation expected;
    const MovePreparationVector *origin = external;
    memset(&allocation, (int)fill, sizeof(allocation));
    expected = allocation;
    expected_allocation((unsigned char *)&expected, external, origin_offset);
    if (external == NULL) {
        switch (origin_offset) {
        case 8: origin = &allocation.info.target; break;
        case 0x38: origin = &allocation.info.pending_38; break;
        case 0x48: origin = &allocation.info.rotation_48; break;
        case 0x58: origin = &allocation.info.origin_58; break;
        case 0x68: origin = &allocation.info.origin_copy_68; break;
        case 0x78: origin = &allocation.info.auxiliary_78; break;
        case 0x88: origin = &allocation.info.transform_88; break;
        default: assert(0);
        }
    }
    output = (MovePreparationInfo *)(void *)&calls;
    calls = 0;
    fail = 0;
    assert(func_800222EC(&output, origin) == 1);
    assert(calls == 1 && output == &allocation.info);
    assert(memcmp(&allocation, &expected, sizeof(allocation)) == 0);
}

int main(void) {
    static const int32_t values[] = {INT32_MIN, -65537, -1, 0, 1, 65537, INT32_MAX};
    static const unsigned offsets[] = {8, 0x38, 0x48, 0x58, 0x68, 0x78, 0x88};
    unsigned x, y, z, fill, offset;
    for (x = 0; x < sizeof(values) / sizeof(values[0]); ++x) {
        for (y = 0; y < sizeof(values) / sizeof(values[0]); ++y) {
            for (z = 0; z < sizeof(values) / sizeof(values[0]); ++z) {
                MovePreparationVector origin = {values[x], values[y], values[z], 123};
                run_case((x * 37 + y * 13 + z) & 255u, &origin, 0);
            }
        }
    }
    for (fill = 0; fill <= 255; ++fill) {
        for (offset = 0; offset < sizeof(offsets) / sizeof(offsets[0]); ++offset) {
            run_case(fill, NULL, offsets[offset]);
        }
    }
    memset(&allocation, 0xA5, sizeof(allocation));
    {
        Allocation before = allocation;
        output = (MovePreparationInfo *)(void *)&calls;
        calls = 0;
        fail = 1;
        assert(func_800222EC(&output, NULL) == 0);
        assert(output == NULL && calls == 1);
        assert(memcmp(&allocation, &before, sizeof(before)) == 0);
    }
    puts("Move-info allocation: 343 origin boundaries, 1792 overlap/fill cases and null allocation passed.");
    return 0;
}
