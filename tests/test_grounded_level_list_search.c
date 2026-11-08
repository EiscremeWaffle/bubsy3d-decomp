#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bubsy_grounded_state.h"
#include "level_records.h"

const char g_level_list_condition[] = "EGSBoolFalse";
const char g_game_source_path[] = "../f/game.c";

enum Mutation {
    NONE,
    SHORTEN_LIST,
    EXTEND_LIST,
    CHANGE_INPUT_ID
};

static GameLevelListInput input;
static BubsyGroundedActor actor_one;
static BubsyGroundedActor actor_two;
static int32_t indices[] = {0, 1, 2};
static int32_t *live_count;
static int32_t initial_count;
static int32_t flag_result;
static enum Mutation mutation;
static unsigned int selected_count;
static unsigned int flag_calls;
static unsigned int assertion_calls;

uint8_t func_80028C84(int32_t index, int32_t *list_id) {
    assert(index == 0);
    *list_id = 1;
    return 1;
}

int32_t *func_80028CD4(int32_t list_id, int32_t *count) {
    assert(list_id == 1);
    live_count = count;
    *count = initial_count;
    return indices;
}

void *func_80028764(int32_t index) {
    ++selected_count;
    if (index == 1) {
        if (mutation == SHORTEN_LIST) {
            *live_count = 2;
        } else if (mutation == EXTEND_LIST) {
            *live_count = 3;
        } else if (mutation == CHANGE_INPUT_ID) {
            input.record_id_0C = 2;
        }
        return &actor_one;
    }
    assert(index == 2);
    return &actor_two;
}

int32_t func_8004B264(BubsyGroundedActor *actor, void *context) {
    assert(actor == &actor_two);
    assert(context == NULL);
    ++flag_calls;
    return flag_result;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0 && condition == g_level_list_condition);
    assert(source == g_game_source_path && line == 0x12CA);
    ++assertion_calls;
}

static void reset(int32_t count, enum Mutation change, int32_t flag) {
    memset(&input, 0, sizeof(input));
    memset(&actor_one, 0, sizeof(actor_one));
    memset(&actor_two, 0, sizeof(actor_two));
    actor_one.state_00 = 0x12;
    actor_two.state_00 = 0xF5;
    initial_count = count;
    mutation = change;
    flag_result = flag;
    live_count = NULL;
    selected_count = 0;
    flag_calls = 0;
    assertion_calls = 0;
}

int main(void) {
    static const int32_t results[] = {0, 1, 0x100, 0x101, -256, -255};
    unsigned int i;

    for (i = 0; i < sizeof(results) / sizeof(results[0]); ++i) {
        reset(3, NONE, results[i]);
        assert(func_80027E24(&input) == ((uint8_t)results[i] != 0));
        assert(selected_count == 2 && flag_calls == 1 && assertion_calls == 0);
    }
    reset(3, SHORTEN_LIST, 1);
    assert(func_80027E24(&input) == 1);
    assert(selected_count == 1 && flag_calls == 0 && assertion_calls == 1);
    reset(2, EXTEND_LIST, 0);
    assert(func_80027E24(&input) == 0);
    assert(selected_count == 2 && flag_calls == 1 && assertion_calls == 0);
    reset(3, CHANGE_INPUT_ID, 1);
    assert(func_80027E24(&input) == 1);
    assert(input.record_id_0C == 2);
    assert(selected_count == 1 && flag_calls == 0 && assertion_calls == 1);
    reset(-1, NONE, 1);
    assert(func_80027E24(&input) == 1);
    assert(selected_count == 0 && flag_calls == 0 && assertion_calls == 1);

    puts("Grounded level-list search: callback mutations and byte return gates passed.");
    return 0;
}
