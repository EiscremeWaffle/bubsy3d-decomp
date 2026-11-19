#ifndef BUBSY3D_SEQUENCE_PLAYBACK_H
#define BUBSY3D_SEQUENCE_PLAYBACK_H

#include <stdint.h>

typedef struct SequencePlaybackNode {
    void *owner_00;
    struct SequencePlaybackNode *previous_04;
    struct SequencePlaybackNode *next_08;
} SequencePlaybackNode;

struct SequenceStreamOwner;

typedef struct SequencePlaybackSlot {
    uint32_t unknown_00;
#if UINTPTR_MAX == 0xFFFFFFFFu
    struct SequenceStreamOwner *stream_owner_04;
#else
    uint32_t unknown_04;
#endif
    uint8_t active_08;
    uint8_t unknown_09_to_0B[3];
    int32_t timestamp_0C;
    uint8_t key_10;
    uint8_t unknown_11_to_13[3];
    uint32_t value_14;
#if UINTPTR_MAX != 0xFFFFFFFFu
    /* The target pointer is widened without moving the original fields. */
    struct SequenceStreamOwner *stream_owner_04;
#endif
} SequencePlaybackSlot;

struct PlayerActorSequenceView;

extern SequencePlaybackNode *g_sequence_playback_active;
extern SequencePlaybackNode *g_sequence_playback_free;
extern uint16_t g_sequence_playback_count;
extern uint8_t g_sequence_stream_context;
extern const char g_sequence_playback_free_condition[];
extern const char g_sequence_playback_slot_condition[];
extern const char g_sequence_playback_source_path[];

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
#define sequence_playback_address(node) (node)
#else
/* Host representation of the PS1 RAM words addressed through a null node. */
extern SequencePlaybackNode g_sequence_playback_zero_address;

static inline SequencePlaybackNode *sequence_playback_address(SequencePlaybackNode *node) {
    return node != 0 ? node : &g_sequence_playback_zero_address;
}
#endif

int32_t func_80053180(SequencePlaybackNode *node);
SequencePlaybackNode *func_800530FC(void *owner);
int32_t func_80052BDC(void *component, int32_t mode);
int32_t func_80052E54(struct PlayerActorSequenceView *component, int32_t key);
void func_80067DEC(struct PlayerActorSequenceView *component, int32_t key, uint32_t context);

#endif
