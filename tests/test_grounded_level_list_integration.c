#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "actor_context.h"
#include "bubsy_grounded_state.h"
#include "level_records.h"

int32_t g_level_record_count;
GameLevelRecord *volatile g_level_records;
int32_t g_level_list_count;
GameLevelList *volatile g_level_lists;
uint8_t g_actor_context_table[0x40 + 256];
const char g_level_list_condition[] = "EGSBoolFalse";
const char g_game_source_path[] = "../f/game.c";

static GameLevelRecord records[3];
static GameLevelList list;
static int32_t indices[3];
static BubsyGroundedActor actor_one;
static BubsyGroundedActor actor_two;
static BubsyGroundedActor fallback_actor;
static unsigned int assertion_calls;
static unsigned int fallback_calls;

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0);
    assert(condition == g_level_list_condition);
    assert(source == g_game_source_path);
    assert(line == 0x12CA);
    ++assertion_calls;
}

void *func_80040FC0(int32_t index) {
    assert(index == 99);
    ++fallback_calls;
    return &fallback_actor;
}

int main(void) {
    static const uint8_t flags[] = {0, 1, 0x80, 0xFF};
    GameLevelListInput input;
    unsigned int i;

    memset(&input, 0, sizeof(input));
    memset(records, 0, sizeof(records));
    memset(&list, 0, sizeof(list));
    memset(&actor_one, 0, sizeof(actor_one));
    memset(&actor_two, 0, sizeof(actor_two));
    memset(&fallback_actor, 0, sizeof(fallback_actor));
    memset(g_actor_context_table, 0xA5, sizeof(g_actor_context_table));
    g_level_record_count = 3;
    g_level_records = records;
    g_level_list_count = 1;
    g_level_lists = &list;
    records[0].list_id_18 = 1;
    records[1].object_1C = &actor_one;
    records[2].object_1C = &actor_two;
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    list.count_02 = 3;
    list.indices_04 = indices;
    actor_one.state_00 = 0x12;
    actor_one.state_01 = 1;
    actor_two.state_00 = 0xF5;
    actor_two.state_01 = 0xFF;

    assert(func_80026648(NULL, NULL) == g_actor_context_table);
    assert(func_80026648(&actor_one, &actor_two) == g_actor_context_table);
    assert(func_80026648(&actor_two, &input) == g_actor_context_table);
    for (i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i) {
        g_actor_context_table[0x40 + 0xFF] = flags[i];
        assertion_calls = 0;
        fallback_calls = 0;
        assert(func_80027E24(&input) == (flags[i] == 0));
        assert(assertion_calls == 0 && fallback_calls == 0);
    }

    actor_one.state_00 = 0xF5;
    g_actor_context_table[0x40 + 1] = 0;
    g_actor_context_table[0x40 + 0xFF] = 1;
    assert(func_80027E24(&input) == 1);
    g_actor_context_table[0x40 + 1] = 1;
    g_actor_context_table[0x40 + 0xFF] = 0;
    assert(func_80027E24(&input) == 0);

    actor_one.state_00 = 0x12;
    actor_two.state_00 = 0x12;
    assertion_calls = 0;
    assert(func_80027E24(&input) == 1 && assertion_calls == 1);
    list.count_02 = INT16_MIN;
    assertion_calls = 0;
    assert(func_80027E24(&input) == 1 && assertion_calls == 1);
    records[0].list_id_18 = 0;
    assertion_calls = 0;
    assert(func_80027E24(&input) == 1 && assertion_calls == 1);

    records[0].list_id_18 = 1;
    list.count_02 = 2;
    indices[1] = 99;
    fallback_actor.state_00 = 0xF5;
    fallback_actor.state_01 = 7;
    g_actor_context_table[0x40 + 7] = 0;
    assertion_calls = 0;
    fallback_calls = 0;
    assert(func_80027E24(&input) == 1);
    assert(assertion_calls == 0 && fallback_calls == 1);

    puts("Grounded level-list chain: real getters, flags, assertions and fallback passed.");
    return 0;
}
