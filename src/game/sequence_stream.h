#ifndef BUBSY3D_SEQUENCE_STREAM_H
#define BUBSY3D_SEQUENCE_STREAM_H

#include "player_model.h"
#include "sequence_playback.h"

typedef struct SequenceStreamDescriptor {
    uint32_t *surface_00;
    uint32_t unknown_04;
    uint8_t *output_08;
    uint32_t unknown_0C;
    void *metadata_10;
} SequenceStreamDescriptor;

typedef struct SequenceStreamOwner {
    uint8_t unknown_00_to_07[8];
    SequenceStreamDescriptor *descriptor_08;
} SequenceStreamOwner;

typedef struct SequenceStreamArguments {
    int32_t quotient_00;
    int32_t remainder_04;
    uint32_t remainder_scaled_08;
    uint32_t quotient_scaled_0C;
    SequenceStreamDescriptor *descriptor_10;
    const uint8_t *table_14;
    uint32_t stride_18;
    uint32_t offset_1C;
    const uint8_t *lookup_20;
} SequenceStreamArguments;

typedef struct SequenceStreamLookupRecord {
    uint8_t unknown_00_to_0F[16];
    uint32_t value_10;
    uint8_t unknown_14_to_33[32];
} SequenceStreamLookupRecord;

_Static_assert(sizeof(SequenceStreamLookupRecord) == 0x34, "stream lookup record size");
#if UINTPTR_MAX == 0xFFFFFFFFu
_Static_assert(offsetof(SequencePlaybackSlot, stream_owner_04) == 0x04, "slot owner offset");
_Static_assert(sizeof(SequencePlaybackSlot) == 0x18, "target slot size");
_Static_assert(offsetof(SequenceStreamOwner, descriptor_08) == 0x08, "stream descriptor offset");
_Static_assert(offsetof(SequenceStreamDescriptor, output_08) == 0x08, "stream output offset");
_Static_assert(offsetof(SequenceStreamDescriptor, metadata_10) == 0x10, "stream metadata offset");
_Static_assert(sizeof(SequenceStreamArguments) == 0x24, "stream argument block size");
#endif

extern const char g_sequence_stream_width_condition[];
extern const char g_sequence_stream_source_path[];
extern int16_t g_sequence_stream_lookup_count;
extern SequenceStreamLookupRecord g_sequence_stream_lookup_records[];

int32_t func_80010368(SequenceStreamArguments *arguments);
int32_t func_800102BC(
    uint8_t *output, const uint8_t *frames, void *metadata, const uint32_t *lookup
);

#endif
