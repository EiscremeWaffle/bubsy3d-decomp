#include "packed_channel_copy.h"
#include "player_model.h"
#include "sequence_playback.h"
#include "word_copy.h"

static __inline__ const uint8_t *sequence_frame_address(
    const uint8_t *base, int32_t count, int32_t key, uint32_t shift
) {
    uint32_t offset = (uint32_t)((long long)count * (long long)key) << shift;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    return (const uint8_t *)((uintptr_t)base + offset);
#else
    return (const uint8_t *)((uintptr_t)base + (uintptr_t)(int64_t)(int32_t)offset);
#endif
}

int32_t func_80052E54(PlayerActorSequenceView *component, int32_t key) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register PlayerActorSequenceData *sequence __asm__("$16") = component->sequence_18;
    register PlayerSequenceResource *resource __asm__("$2");
    register PlayerSequenceChannels *channels __asm__("$3");
    register uint8_t *first_output __asm__("$4");
    register int32_t first_count __asm__("$6");
    register int32_t second_count __asm__("$17");
    register uint32_t flags __asm__("$5");
    register const uint8_t *frames __asm__("$7");
    register int32_t doubled_key __asm__("$2");
    register int32_t saved_key __asm__("$18") = key;
    register uint32_t first_masked __asm__("$2");
    register PlayerSequenceChannels *second_channels __asm__("$2");
    register uint8_t *second_output __asm__("$4");
    register uint32_t second_flags __asm__("$2");
    register uint32_t second_mask __asm__("$3");
    register uint32_t second_masked __asm__("$2");
#else
    PlayerActorSequenceData *sequence = component->sequence_18;
    PlayerSequenceResource *resource;
    PlayerSequenceChannels *channels;
    uint8_t *first_output;
    int32_t first_count;
    int32_t second_count;
    uint32_t flags;
    const uint8_t *frames;
    int32_t doubled_key;
    int32_t saved_key = key;
    uint32_t first_masked;
    PlayerSequenceChannels *second_channels;
    uint8_t *second_output;
    uint32_t second_flags;
    uint32_t second_mask;
    uint32_t second_masked;
#endif

    if (sequence->enabled_2B == 0) {
        return 1;
    }

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(saved_key) : "0"(saved_key));
#endif
    flags = sequence->flags_2C;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(flags) : "0"(flags));
#endif
    if (flags & 4) {
        func_80067DEC(component, saved_key, g_sequence_stream_context);
        return 0;
    }

    resource = sequence->resource_20;
    frames = sequence->first_frames_10;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(resource), "=r"(frames) : "0"(resource), "1"(frames) : "memory");
#endif
    channels = resource->channels_08;
    first_output = channels->first_output;
    first_count = channels->first_count;
    second_count = channels->second_count;
    first_masked = flags & 0x40000000u;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(first_masked) : "0"(first_masked));
#endif
    if (first_masked) {
        func_80010244(first_output, sequence_frame_address(frames, first_count, saved_key, 2),
                      (uint32_t)first_count);
    } else {
        doubled_key = (int32_t)((uint32_t)saved_key << 1);
        func_80010220((uint32_t *)first_output,
                      (const uint32_t *)sequence_frame_address(frames, first_count, doubled_key, 2),
                      (uint32_t)first_count << 1);
    }

    /* The second count is captured above, but these fields are read again. */
    resource = sequence->resource_20;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(resource) : "0"(resource) : "memory");
#endif
    second_channels = resource->channels_08;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(second_channels) : "0"(second_channels));
#endif
    second_mask = 0x60000000u;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(second_mask) : "0"(second_mask));
#endif
    second_output = second_channels->second_output;
    second_flags = sequence->flags_2C;
    frames = sequence->second_frames_14;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(second_output), "=r"(second_flags), "=r"(frames)
                     : "0"(second_output), "1"(second_flags), "2"(frames) : "memory");
#endif
    second_masked = second_flags & second_mask;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__ volatile("" : "=r"(second_masked) : "0"(second_masked));
#endif
    if (second_masked) {
        func_80010278(second_output,
                      sequence_frame_address(frames, second_count, saved_key, 2),
                      (uint32_t)second_count);
    } else {
        doubled_key = (int32_t)((uint32_t)saved_key << 1);
        func_80010220((uint32_t *)second_output,
                      (const uint32_t *)sequence_frame_address(frames, second_count, doubled_key, 2),
                      (uint32_t)second_count << 1);
    }
    sequence->initialized_key_28 = (int16_t)saved_key;
    return 0;
}
