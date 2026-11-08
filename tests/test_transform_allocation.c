#include "component_rotation.h"
#include "level_records.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct MoveAllocation {
    MovePreparationInfo info;
    uint32_t tail;
} MoveAllocation;

_Static_assert(sizeof(ComponentRotationMatrix) == 0x20, "component allocation size");
_Static_assert(offsetof(ComponentRotationMatrix, translation) == 0x14, "translation offset");
_Static_assert(sizeof(MoveAllocation) == 0xA8, "move-info allocation size");

ComponentRotationMatrix g_component_allocation_template;
void *g_move_preparation_transform;
int32_t g_level_record_count;
GameLevelRecord *volatile g_level_records;
const char g_move_table_condition[] = "table";
const char g_game_source_path[] = "../f/game.c";

static ComponentRotationMatrix allocated_transform;
static ComponentRotationMatrix cached_transform;
static MoveAllocation allocated_move;
static MovePreparationActor actor;
static MovePreparationEntry entry;
static GameLevelRecord records[6];
static MovePreparationVector origin = {7, -13, 29, 0};
static int scale_marker;
static unsigned calls[8];
static unsigned call_count;
static int fail_move;
static int fail_transform;
static int mutate_template;
static int mutate_output;
static void **observed_output;
static void *expected_output_before_allocation;
static void *expected_initialized_transform;

enum {
    MOVE_ALLOCATION = 1,
    TRANSFORM_ALLOCATION,
    INITIALIZER_CALL
};

static void record(unsigned call) {
    assert(call_count < sizeof(calls) / sizeof(calls[0]));
    calls[call_count++] = call;
}

void *func_80082EEC(uint32_t size) {
    if (size == 0xA8) {
        record(MOVE_ALLOCATION);
        assert(actor.move_info_28 == NULL);
        return fail_move ? NULL : &allocated_move.info;
    }

    assert(size == 0x20);
    record(TRANSFORM_ALLOCATION);
    if (observed_output != NULL) {
        assert(*observed_output == expected_output_before_allocation);
        if (mutate_output) {
            *observed_output = &cached_transform;
        }
    }
    if (mutate_template) {
        g_component_allocation_template.elements[4] ^= 0x55AAu;
        g_component_allocation_template.translation[2] = -1234567;
    }
    return fail_transform ? NULL : &allocated_transform;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    (void)failed;
    (void)condition;
    (void)source;
    (void)line;
    assert(0 && "unexpected table assertion");
}

int32_t func_8002611C(MovePreparationActor *input, const void *scale, uint32_t flags) {
    record(INITIALIZER_CALL);
    assert(input == &actor && scale == &scale_marker && flags == 0xFEDCBA98u);
    assert(actor.move_info_28 == &allocated_move.info);
    assert(allocated_move.info.origin_58.x == origin.x);
    assert(allocated_move.info.origin_58.y == origin.y);
    assert(allocated_move.info.origin_58.z == origin.z);
    assert(g_move_preparation_transform == expected_initialized_transform);
    if (expected_initialized_transform == &allocated_transform) {
        assert(memcmp(&allocated_transform, &g_component_allocation_template,
            sizeof(allocated_transform)) == 0);
    }
    return 1;
}

static void fill_template(unsigned seed) {
    unsigned char *bytes = (unsigned char *)&g_component_allocation_template;
    unsigned i;
    for (i = 0; i < sizeof(g_component_allocation_template); ++i) {
        bytes[i] = (unsigned char)(seed + i * 37u);
    }
}

static void reset(void) {
    memset(&actor, 0, sizeof(actor));
    memset(&entry, 0, sizeof(entry));
    memset(records, 0, sizeof(records));
    memset(&allocated_move, 0xA5, sizeof(allocated_move));
    memset(&allocated_transform, 0x5A, sizeof(allocated_transform));
    actor.context_0C = 5;
    entry.limit = 1;
    records[0].table_count_10 = records[5].table_count_10 = 1;
    records[0].table_14 = records[5].table_14 = &entry;
    g_level_record_count = 6;
    g_level_records = records;
    fill_template(0x31);
    g_move_preparation_transform = NULL;
    call_count = 0;
    fail_move = 0;
    fail_transform = 0;
    mutate_template = 0;
    mutate_output = 0;
    observed_output = NULL;
    expected_output_before_allocation = NULL;
    expected_initialized_transform = &allocated_transform;
}

static void test_template_copy_and_publication(void) {
    unsigned seed;
    for (seed = 0; seed < 256; ++seed) {
        void *output;
        reset();
        fill_template(seed);
        output = &cached_transform;
        observed_output = &output;
        expected_output_before_allocation = &cached_transform;
        assert(func_80054624(&output) == 0);
        assert(call_count == 1 && calls[0] == TRANSFORM_ALLOCATION);
        assert(output == &allocated_transform);
        assert(memcmp(&allocated_transform, &g_component_allocation_template,
            sizeof(allocated_transform)) == 0);
    }

    reset();
    {
        void *output = NULL;
        observed_output = &output;
        mutate_output = 1;
        mutate_template = 1;
        assert(func_80054624(&output) == 0);
        assert(output == &allocated_transform);
        assert(allocated_transform.translation[2] == -1234567);
        assert(memcmp(&allocated_transform, &g_component_allocation_template,
            sizeof(allocated_transform)) == 0);
    }
}

static void test_allocation_failure(void) {
    ComponentRotationMatrix before;
    void *output = &cached_transform;
    reset();
    before = allocated_transform;
    observed_output = &output;
    expected_output_before_allocation = &cached_transform;
    mutate_output = 1;
    fail_transform = 1;
    assert(func_80054624(&output) == 3);
    assert(output == NULL);
    assert(memcmp(&allocated_transform, &before, sizeof(before)) == 0);
    assert(call_count == 1 && calls[0] == TRANSFORM_ALLOCATION);
}

static void test_movement_setup_integration(void) {
    reset();
    observed_output = &g_move_preparation_transform;
    mutate_output = 1;
    mutate_template = 1;
    assert(func_80025FFC(&actor, &origin, &scale_marker, 0xFEDCBA98u) == 1);
    assert(call_count == 3 && calls[0] == MOVE_ALLOCATION &&
        calls[1] == TRANSFORM_ALLOCATION && calls[2] == INITIALIZER_CALL);
    assert(g_move_preparation_transform == &allocated_transform);

    reset();
    g_move_preparation_transform = &cached_transform;
    expected_initialized_transform = &cached_transform;
    assert(func_80025FFC(&actor, &origin, &scale_marker, 0xFEDCBA98u) == 1);
    assert(call_count == 2 && calls[0] == MOVE_ALLOCATION && calls[1] == INITIALIZER_CALL);

    reset();
    fail_transform = 1;
    observed_output = &g_move_preparation_transform;
    mutate_output = 1;
    assert(func_80025FFC(&actor, &origin, &scale_marker, 0xFEDCBA98u) == 0);
    assert(call_count == 2 && calls[1] == TRANSFORM_ALLOCATION);
    assert(actor.move_info_28 == &allocated_move.info);
    assert(g_move_preparation_transform == NULL);
    call_count = 0;
    assert(func_80025FFC(&actor, &origin, &scale_marker, 0xFEDCBA98u) == 1);
    assert(call_count == 0);

    reset();
    fail_move = 1;
    assert(func_80025FFC(&actor, &origin, &scale_marker, 0xFEDCBA98u) == 0);
    assert(call_count == 1 && calls[0] == MOVE_ALLOCATION);
    assert(actor.move_info_28 == NULL && g_move_preparation_transform == NULL);

    reset();
    actor.context_0C = 0;
    assert(func_80025FFC(&actor, &origin, &scale_marker, 0xFEDCBA98u) == 1);
    assert(call_count == 1 && calls[0] == MOVE_ALLOCATION);
    assert(g_move_preparation_transform == NULL);

    reset();
    records[5].table_count_10 = 0;
    assert(func_80025FFC(&actor, &origin, &scale_marker, 0xFEDCBA98u) == 1);
    assert(call_count == 0 && actor.move_info_28 == NULL);

    reset();
    entry.limit = 0;
    assert(func_80025FFC(&actor, &origin, &scale_marker, 0xFEDCBA98u) == 1);
    assert(call_count == 0 && actor.move_info_28 == NULL);
}

int main(void) {
    test_template_copy_and_publication();
    test_allocation_failure();
    test_movement_setup_integration();
    puts("Transform allocation: 256 template copies, publication, failure and movement setup passed.");
    return 0;
}