#include "sequence_stream.h"

#ifndef BUBSY3D_MATCH_ORIGINAL_L0
#include <stdlib.h>
#endif

extern void func_800556D0(
    int32_t failed, const char *condition, const char *source, uint32_t line
);

void func_80067DEC(PlayerActorSequenceView *actor, int32_t key, uint32_t context) {
    PlayerActorSequenceData *sequence = actor->sequence_18;
    uint32_t width;
    int32_t quotient, remainder;
    SequenceStreamDescriptor *descriptor;
    uint8_t *output;
    void *metadata;
    const uint8_t *frames;
    SequenceStreamArguments arguments;

    if (actor->playback_slot_06 < 0) {
        return;
    }

    width = context & 0xFFu;
    if (width == 0) {
        func_800556D0(0, g_sequence_stream_width_condition, g_sequence_stream_source_path, 0xED);
#ifndef BUBSY3D_MATCH_ORIGINAL_L0
        abort();
#endif
    }
    quotient = key / (int32_t)width;
    remainder = key % (int32_t)width;
    descriptor = sequence->playback_slots[actor->playback_slot_06].stream_owner_04->descriptor_08;
    metadata = descriptor->metadata_10;
    output = descriptor->output_08;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    frames = (const uint8_t *)((uintptr_t)sequence->first_frames_10 + ((uint32_t)key << 9));
#else
    frames = (const uint8_t *)((uintptr_t)sequence->first_frames_10
             + (uintptr_t)(int64_t)(int32_t)((uint32_t)key << 9));
#endif
    arguments.quotient_00 = quotient;
    arguments.remainder_04 = remainder;
    arguments.remainder_scaled_08 = (uint32_t)remainder << 11;
    arguments.quotient_scaled_0C = (uint32_t)quotient << 11;
    arguments.descriptor_10 = descriptor;
    arguments.table_14 = sequence->stream_table_18;
    arguments.stride_18 = (width << 3) + 1;
    arguments.offset_1C = ((uint32_t)remainder + (uint32_t)quotient * arguments.stride_18) << 3;
    arguments.lookup_20 = sequence->second_frames_14;
    func_80010368(&arguments);
    func_800102BC(output, frames, metadata, (const uint32_t *)sequence->second_frames_14);
    sequence->playback_slots[actor->playback_slot_06].key_10 = (uint8_t)key;
}
