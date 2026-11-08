#include "game_transition.h"
#include "word_copy.h"

#ifndef TRANSITION_SNAPSHOT_FUNCTION
#define TRANSITION_SNAPSHOT_FUNCTION func_8001B8B8
#endif

enum {
    TRANSITION_QUEUE_HEADER_BYTES = sizeof(uint32_t),
    TRANSITION_RECORD_BYTES = sizeof(GameTransitionQueueRecord),
    TRANSITION_RECORD_WORDS = sizeof(GameTransitionQueueRecord) / sizeof(uint32_t),
    TRANSITION_QUEUE_CAPACITY = (int)(sizeof(((GameTransitionQueue *)0)->records) /
                                      sizeof(GameTransitionQueueRecord))
};

void TRANSITION_SNAPSHOT_FUNCTION(GameTransitionQueue *input_arg, GameTransitionQueue *output_arg) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register GameTransitionQueue *input __asm__("$20") = input_arg;
    GameTransitionQueue *output = output_arg;
    register int32_t copied __asm__("$22") = 0;
    register int32_t mode __asm__("$23") = 0;
    register uint32_t remaining __asm__("$18");
    register int32_t slot __asm__("$21");
    register uint8_t *cursor __asm__("$16");
    register int32_t source_offset __asm__("$19");
    register int32_t destination_offset __asm__("$17");
    register int32_t destination_base __asm__("$2");
#else
    GameTransitionQueue *input = input_arg;
    GameTransitionQueue *output = output_arg;
    int32_t copied = 0;
    int32_t mode = 0;
    uint32_t remaining;
    int32_t slot;
    uint8_t *cursor;
    int32_t source_offset;
    int32_t destination_offset;
    int32_t destination_base;
#endif

outer:
    remaining = input->count;
    if (remaining != 0) {
        slot = 0;
        cursor = (uint8_t *)input;
        /* Separate cursors preserve the original scan and copy register allocation. */
        source_offset = 0;
        destination_base = copied * TRANSITION_RECORD_BYTES;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__("" : "=r"(destination_base) : "0"(destination_base));
#endif
        destination_offset = destination_base + TRANSITION_QUEUE_HEADER_BYTES;
    scan:
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(remaining) : "0"(remaining));
#endif
        if (remaining != 0) {
            ++slot;
            if (((GameTransitionEventRecord *)(cursor + TRANSITION_QUEUE_HEADER_BYTES))->event_id != 0) {
                --remaining;
                if (((GameTransitionEventRecord *)(cursor + TRANSITION_QUEUE_HEADER_BYTES))->mode == mode) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                    register int32_t source_position __asm__("$5") = source_offset + TRANSITION_QUEUE_HEADER_BYTES;
                    register uintptr_t destination_position __asm__("$4") =
                        (uintptr_t)output + destination_offset;
                    __asm__("" : "=r"(source_position), "=r"(destination_position)
                            : "0"(source_position), "1"(destination_position));
#else
                    int32_t source_position = source_offset + TRANSITION_QUEUE_HEADER_BYTES;
                    uintptr_t destination_position = (uintptr_t)output + destination_offset;
#endif
                    func_80010220(
                        (uint32_t *)destination_position,
                        (const uint32_t *)((uintptr_t)input + source_position),
                        TRANSITION_RECORD_WORDS
                    );
                    destination_offset += TRANSITION_RECORD_BYTES;
                    ((GameTransitionEventRecord *)(cursor + TRANSITION_QUEUE_HEADER_BYTES))->event_id = 0;
                    --input->count;
                    ++copied;
                }
            }
            cursor += TRANSITION_RECORD_BYTES;
            source_offset += TRANSITION_RECORD_BYTES;
            if (slot < TRANSITION_QUEUE_CAPACITY) {
                goto scan;
            }
        }
        ++mode;
        if (mode < 5) {
            goto outer;
        }
    }
    output->count = copied;
}
