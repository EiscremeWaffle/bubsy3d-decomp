#ifndef BUBSY3D_GAME_TRANSITION_H
#define BUBSY3D_GAME_TRANSITION_H

#include <stdint.h>

typedef struct GameTransitionInputPayload {
    uint32_t buttons;
    uint8_t digital_pad_1;
    uint8_t digital_pad_2;
    uint8_t reserved[10];
} GameTransitionInputPayload;

typedef struct GameTransitionEventRecord {
    uint16_t event_id;
    uint16_t mode;
    int32_t flags;
    int32_t heading;
    union {
        uint32_t opaque[4];
        GameTransitionInputPayload input;
    } payload;
} GameTransitionEventRecord;

typedef union GameTransitionQueueRecord {
    GameTransitionEventRecord event;
    uint32_t words[7];
} GameTransitionQueueRecord;

typedef struct GameTransitionQueue {
    uint32_t count;
    GameTransitionQueueRecord records[100];
} GameTransitionQueue;

typedef struct GameTransitionPayload {
    uint16_t event_id;
    uint16_t mode;
    int32_t flags;
    int32_t heading;
    uint32_t opaque[4];
} GameTransitionPayload;

typedef void (*GameTransitionArgumentHandler)(
    GameTransitionEventRecord *record,
    void *argument_4,
    void *argument_5,
    void *argument_6,
    void *argument_7
);
typedef void (*GameTransitionRecordHandler)(GameTransitionEventRecord *record);
typedef void (*GameTransitionPayloadHandler)(GameTransitionPayload *payload);

typedef struct GameTransitionHeadingEntry {
    GameTransitionPayloadHandler handler;
    uintptr_t reserved;
} GameTransitionHeadingEntry;

typedef struct GameTransitionHeadingTable {
    uint32_t reserved[4];
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    /* GCC 2.6.3 predates C99 flexible array members. */
    GameTransitionHeadingEntry entries[0];
#else
    GameTransitionHeadingEntry entries[];
#endif
} GameTransitionHeadingTable;

typedef struct GameTransitionStaticHandlers {
    GameTransitionRecordHandler mode_5_heading_ffff;
    GameTransitionArgumentHandler flags_ffff;
    uint32_t reserved;
} GameTransitionStaticHandlers;

typedef union GameTransitionHeadingHandlers {
    GameTransitionRecordHandler heading_fffe;
    GameTransitionPayloadHandler heading_fffd;
} GameTransitionHeadingHandlers;

typedef struct GameTransitionModeHandlers {
    uint32_t reserved;
    GameTransitionHeadingHandlers heading;
    GameTransitionArgumentHandler flags_fffe;
} GameTransitionModeHandlers;

/* The third argument is a target-sized word; some callers pass a pointer. */
void func_8001B558(
    int32_t event_id,
    int32_t mode,
    uintptr_t flags,
    int32_t heading,
    void *argument_4,
    void *argument_5,
    void *argument_6,
    void *argument_7
);

/* Snapshot occupied modes 0..4 in priority and slot order; count tracks occupied slots. */
void func_8001B8B8(GameTransitionQueue *input, GameTransitionQueue *output);

void func_80023A0C(
    GameTransitionEventRecord *record,
    void *buttons,
    void *digital_pad_1,
    void *digital_pad_2,
    void *unused
);

void func_8002278C(void);
void func_80027F08(void);
void func_80044C34(const uint32_t *parameters);

#endif
