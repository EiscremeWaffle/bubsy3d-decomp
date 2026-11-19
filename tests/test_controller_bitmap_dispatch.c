#include "controller_bitmap_dispatch.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct BitmapFixture {
    uint32_t bitmap;
    int16_t selector;
    int32_t rows[4];
    uint32_t pressed[4][16];
    uint32_t released[4][16];
    int32_t events[32];
    uint32_t snapshots[32];
    unsigned int count;
    int mutate;
} BitmapFixture;

static void emit_event(
    void *context, int32_t event_id, int32_t mode, uintptr_t flags, int32_t heading,
    void *argument_4, void *argument_5, void *argument_6, void *argument_7
) {
    BitmapFixture *fixture = context;
    assert(mode == 4 && flags == 0xFFFE && heading == 0);
    assert(argument_4 == 0 && argument_5 == 0 && argument_6 == 0 && argument_7 == 0);
    assert(fixture->count < sizeof(fixture->events) / sizeof(fixture->events[0]));
    fixture->events[fixture->count] = event_id;
    fixture->snapshots[fixture->count] = fixture->bitmap;
    fixture->count++;
    if (fixture->mutate && fixture->count == 1) {
        fixture->bitmap |= 1u << 5;
        fixture->selector = 1;
        fixture->rows[1] = 2;
        fixture->pressed[2][4] = 0x1F1u;
        fixture->released[2][5] = 0x222u;
    }
}

static ControllerBitmapDispatch dispatch_for(BitmapFixture *fixture) {
    ControllerBitmapDispatch dispatch = {
        &fixture->bitmap, &fixture->selector, fixture->rows,
        fixture->pressed, fixture->released,
        emit_event, fixture,
    };
    return dispatch;
}

int main(void) {
    BitmapFixture fixture;
    ControllerBitmapDispatch dispatch;
    unsigned int bit, active, previous;

    for (bit = 0; bit < 16; bit++) {
        for (active = 0; active < 2; active++) {
            for (previous = 0; previous < 2; previous++) {
                uint32_t mask = 1u << bit;
                memset(&fixture, 0, sizeof(fixture));
                fixture.bitmap = 0xA5A50000u | (previous ? mask : 0);
                fixture.pressed[0][bit] = 100u + bit;
                fixture.released[0][bit] = 150u + bit;
                dispatch = dispatch_for(&fixture);
                controller_bitmap_dispatch(&dispatch, active ? mask : 0);
                assert(fixture.bitmap == (0xA5A50000u | (active ? mask : 0)));
                assert(fixture.count == (active || previous ? 1u : 0u));
                if (fixture.count) {
                    assert(fixture.events[0] == (int32_t)((active ? 100u : 150u) + bit));
                    assert(fixture.snapshots[0] == fixture.bitmap);
                }
            }
        }
    }

    memset(&fixture, 0, sizeof(fixture));
    fixture.bitmap = 0x5A5A0000u;
    fixture.pressed[0][0] = 0x100u;
    fixture.mutate = 1;
    dispatch = dispatch_for(&fixture);
    controller_bitmap_dispatch(&dispatch, (1u << 0) | (1u << 4));
    assert(fixture.count == 3);
    assert(fixture.events[0] == 0 && fixture.events[1] == 0xF1 && fixture.events[2] == 0x22);
    assert(fixture.snapshots[0] == 0x5A5A0001u);
    assert(fixture.snapshots[1] == 0x5A5A0031u);
    assert(fixture.snapshots[2] == 0x5A5A0011u);
    assert(fixture.bitmap == 0x5A5A0011u);
    assert(fixture.selector == 1 && fixture.rows[1] == 2);

    memset(&fixture, 0, sizeof(fixture));
    fixture.bitmap = 1u << 8;
    dispatch = dispatch_for(&fixture);
    controller_bitmap_dispatch(&dispatch, 1u << 7);
    assert(fixture.count == 0);
    assert(fixture.bitmap == 1u << 7);
    puts("Controller bitmap dispatch: held, released, zero mappings and callback mutations passed");
    return 0;
}
