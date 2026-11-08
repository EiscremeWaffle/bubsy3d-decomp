#include "game_transition.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

_Static_assert(sizeof(GameTransitionEventRecord) == 28, "event record size");
_Static_assert(sizeof(GameTransitionQueueRecord) == 28, "queue record size");
_Static_assert(offsetof(GameTransitionQueue, records) == 4, "queue record offset");
_Static_assert(sizeof(GameTransitionQueue) == 2804, "queue size");

static uint32_t random_state = 0x9E3779B9u;

static uint32_t next_random(void) {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

static uint32_t fill_queue(GameTransitionQueue *queue) {
    uint32_t occupied = 0;
    unsigned int slot, word;

    queue->count = next_random();
    for (slot = 0; slot < 100; ++slot) {
        for (word = 0; word < 7; ++word) {
            queue->records[slot].words[word] = next_random();
        }
        if (next_random() % 3 == 0) {
            queue->records[slot].event.event_id = 0;
        } else {
            queue->records[slot].event.event_id = (uint16_t)((next_random() % 65535) + 1);
            ++occupied;
        }
        queue->records[slot].event.mode = (uint16_t)(next_random() % 8);
    }
    return occupied;
}

static void snapshot_reference(GameTransitionQueue *input, GameTransitionQueue *output) {
    uint32_t copied = 0;
    unsigned int mode;

    for (mode = 0; mode < 5; ++mode) {
        uint32_t remaining = input->count;
        unsigned int slot;

        if (remaining == 0) {
            break;
        }
        for (slot = 0; slot < 100 && remaining != 0; ++slot) {
            GameTransitionQueueRecord *record = &input->records[slot];
            unsigned int word;

            if (record->event.event_id == 0) {
                continue;
            }
            --remaining;
            if (record->event.mode != mode) {
                continue;
            }
            for (word = 0; word < 7; ++word) {
                output->records[copied].words[word] = record->words[word];
            }
            record->event.event_id = 0;
            --input->count;
            ++copied;
        }
    }
    output->count = copied;
}

static void test_priority_and_preservation(void) {
    GameTransitionQueue input, output;
    unsigned int slot;

    memset(&input, 0, sizeof(input));
    memset(&output, 0xA5, sizeof(output));
    input.count = 5;
    input.records[0].event.event_id = 11;
    input.records[0].event.mode = 4;
    input.records[4].event.event_id = 22;
    input.records[4].event.mode = 0;
    input.records[28].event.event_id = 33;
    input.records[28].event.mode = 5;
    input.records[50].event.event_id = 44;
    input.records[50].event.mode = 0;
    input.records[99].event.event_id = 55;
    input.records[99].event.mode = 2;
    for (slot = 0; slot < 100; ++slot) {
        input.records[slot].event.payload.opaque[3] = 0xABCD0000u + slot;
    }

    func_8001B8B8(&input, &output);
    assert(output.count == 4 && input.count == 1);
    assert(output.records[0].event.event_id == 22);
    assert(output.records[1].event.event_id == 44);
    assert(output.records[2].event.event_id == 55);
    assert(output.records[3].event.event_id == 11);
    assert(output.records[0].event.payload.opaque[3] == 0xABCD0004u);
    assert(output.records[1].event.payload.opaque[3] == 0xABCD0032u);
    assert(output.records[2].event.payload.opaque[3] == 0xABCD0063u);
    assert(output.records[3].event.payload.opaque[3] == 0xABCD0000u);
    assert(output.records[4].words[0] == 0xA5A5A5A5u);
    assert(input.records[0].event.event_id == 0);
    assert(input.records[4].event.event_id == 0);
    assert(input.records[28].event.event_id == 33);
    assert(input.records[50].event.event_id == 0);
    assert(input.records[99].event.event_id == 0);
}

static void test_count_and_capacity_edges(void) {
    GameTransitionQueue input, expected_input, output, expected_output;
    unsigned int slot;

    memset(&input, 0, sizeof(input));
    memset(&output, 0xA5, sizeof(output));
    for (slot = 0; slot < 100; ++slot) {
        input.records[slot].event.event_id = (uint16_t)(slot + 1);
        input.records[slot].event.mode = (uint16_t)(slot % 5);
    }

    expected_input = input;
    func_8001B8B8(&input, &output);
    assert(output.count == 0);
    assert(memcmp(&input, &expected_input, sizeof(input)) == 0);
    assert(output.records[0].words[0] == 0xA5A5A5A5u);

    input.count = 100;
    expected_input = input;
    expected_output = output;
    snapshot_reference(&expected_input, &expected_output);
    func_8001B8B8(&input, &output);
    assert(output.count == 100 && input.count == 0);
    assert(memcmp(&input, &expected_input, sizeof(input)) == 0);
    assert(memcmp(&output, &expected_output, sizeof(output)) == 0);

    input = expected_input;
    for (slot = 0; slot < 100; ++slot) {
        input.records[slot].event.event_id = (uint16_t)(slot + 1);
    }
    input.count = UINT32_MAX;
    expected_input = input;
    expected_output = output;
    snapshot_reference(&expected_input, &expected_output);
    func_8001B8B8(&input, &output);
    assert(output.count == 100 && input.count == UINT32_MAX - 100);
    assert(memcmp(&input, &expected_input, sizeof(input)) == 0);
    assert(memcmp(&output, &expected_output, sizeof(output)) == 0);
}

int main(void) {
    unsigned int iteration;
    GameTransitionQueue actual_input, expected_input, actual_output, expected_output;
    GameTransitionQueue alias_actual, alias_expected;

    test_priority_and_preservation();
    test_count_and_capacity_edges();
    for (iteration = 0; iteration < 4096; ++iteration) {
        uint32_t occupied = fill_queue(&actual_input);
        fill_queue(&actual_output);
        switch (iteration % 8) {
            case 0: actual_input.count = 0; break;
            case 1: actual_input.count = 1; break;
            case 2: actual_input.count = occupied; break;
            case 3: actual_input.count = occupied == 0 ? 0 : occupied - 1; break;
            case 4: actual_input.count = occupied < 100 ? occupied + 1 : occupied; break;
            case 5: actual_input.count = 100; break;
            case 6: actual_input.count = next_random() % 101; break;
            default: actual_input.count = 3; break;
        }
        alias_actual = actual_input;
        alias_expected = actual_input;
        expected_input = actual_input;
        expected_output = actual_output;
        snapshot_reference(&expected_input, &expected_output);
        func_8001B8B8(&actual_input, &actual_output);
        assert(memcmp(&actual_input, &expected_input, sizeof(actual_input)) == 0);
        assert(memcmp(&actual_output, &expected_output, sizeof(actual_output)) == 0);

        snapshot_reference(&alias_expected, &alias_expected);
        func_8001B8B8(&alias_actual, &alias_actual);
        assert(memcmp(&alias_actual, &alias_expected, sizeof(alias_actual)) == 0);
    }
    printf("Transition snapshot: priority, preserved records and 8192 randomized cases passed.\n");
    return 0;
}
