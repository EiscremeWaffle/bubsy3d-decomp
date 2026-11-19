#include "component_rotation.h"
#include "level_records.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(GameLevelRecord) == 0x24, "level-record stride");
_Static_assert(offsetof(GameLevelRecord, selector_04) == 0x04, "selector offset");
_Static_assert(offsetof(GameLevelRecord, rotation_08) == 0x08, "rotation offset");
_Static_assert(offsetof(GameLevelRecord, position_0C) == 0x0C, "position offset");
_Static_assert(offsetof(GameLevelRecord, table_count_10) == 0x10, "count offset");
_Static_assert(offsetof(GameLevelRecord, table_14) == 0x14, "table offset");
_Static_assert(offsetof(GameLevelRecord, list_id_18) == 0x18, "list ID offset");
_Static_assert(offsetof(GameLevelRecord, object_1C) == 0x1C, "object offset");
_Static_assert(offsetof(GameLevelRecord, flag_20) == 0x20, "interaction flag offset");
_Static_assert(sizeof(GameLevelList) == 8, "level-list stride");
_Static_assert(offsetof(GameLevelList, count_02) == 2, "level-list count offset");
_Static_assert(offsetof(GameLevelList, indices_04) == 4, "level-list pointer offset");
_Static_assert(offsetof(GameLevelListInput, record_id_0C) == 0x0C, "input record ID offset");
#endif

int32_t g_level_record_count;
GameLevelRecord *volatile g_level_records;
int32_t g_level_list_count;
GameLevelList *volatile g_level_lists;
const ComponentRotationMatrix g_level_default_rotation = {
    {0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000, 0x9000},
    0xABCD,
    {101, 202, 303}
};

static GameLevelRecord records[4];
static GameLevelPosition positions[4];
static GameLevelList lists[4];
static int32_t list_indices[4][3];
static MovePreparationEntry tables[4];
static int objects[4];
static int fallback_object;
static int32_t fallback_index;
static unsigned fallback_calls;

void *func_80040FC0(int32_t index) {
    ++fallback_calls;
    fallback_index = index;
    return &fallback_object;
}

static void reset(void) {
    unsigned index;

    memset(records, 0, sizeof(records));
    memset(positions, 0, sizeof(positions));
    memset(lists, 0, sizeof(lists));
    memset(tables, 0, sizeof(tables));
    for (index = 0; index < 4; ++index) {
        records[index].table_count_10 = (int32_t)index * 11 - 7;
        records[index].position_0C = &positions[index];
        records[index].table_14 = &tables[index];
        records[index].list_id_18 = (int32_t)index * 19 - 3;
        records[index].object_1C = &objects[index];
        lists[index].count_02 = (int16_t)((int32_t)index - 2);
        lists[index].indices_04 = list_indices[index];
    }
    g_level_record_count = 4;
    g_level_records = records;
    g_level_list_count = 4;
    g_level_lists = lists;
    fallback_calls = 0;
    fallback_index = -1;
}

static void test_positions(void) {
    GameLevelPosition output;
    int32_t index;
    unsigned int word;

    reset();
    for (index = 0; index < 4; index++) {
        for (word = 0; word < 4; word++) {
            positions[index].words[word] = index * 100 - (int32_t)word * 1234567;
        }
        memset(&output, 0xA5, sizeof(output));
        assert(func_80028B5C(index, &output) == 1);
        assert(memcmp(&output, &positions[index], sizeof(output)) == 0);
        assert(func_80028B5C(index, &positions[index]) == 1);
    }
    memset(&output, 0xA5, sizeof(output));
    assert(func_80028B5C(-1, &output) == 0);
    assert(func_80028B5C(4, &output) == 0);
    for (word = 0; word < 4; word++) assert((uint32_t)output.words[word] == 0xA5A5A5A5u);
    assert(func_80028B5C(INT32_MIN, NULL) == 0);
    assert(func_80028B5C(INT32_MAX, NULL) == 0);
    g_level_record_count = 0;
    assert(func_80028B5C(0, NULL) == 0);
    g_level_record_count = -1;
    assert(func_80028B5C(0, NULL) == 0);
}

static void test_rotation_lookup(void) {
    static const uint16_t rotation[9] = {
        0, 1, 0x7FFF, 0x8000, 0xFFFF, 5, 6, 7, 8
    };
    ComponentRotationMatrix output;
    unsigned int index;

    reset();
    positions[1].words[0] = INT32_MIN;
    positions[1].words[1] = -7;
    positions[1].words[2] = INT32_MAX;
    positions[1].words[3] = 123;
    memset(&output, 0xA5, sizeof(output));
    assert(func_800289E8(1, &output) == 1);
    assert(memcmp(output.elements, g_level_default_rotation.elements, sizeof(output.elements)) == 0);
    assert(output.padding == g_level_default_rotation.padding);
    assert(output.translation[0] == INT32_MIN);
    assert(output.translation[1] == -7);
    assert(output.translation[2] == INT32_MAX);

    records[2].rotation_08 = rotation;
    positions[2].words[0] = -1234567;
    positions[2].words[1] = 0;
    positions[2].words[2] = 7654321;
    memset(&output, 0xA5, sizeof(output));
    assert(func_800289E8(2, &output) == 1);
    assert(memcmp(output.elements, rotation, sizeof(rotation)) == 0);
    assert(output.padding == 0xA5A5);
    assert(output.translation[0] == -1234567);
    assert(output.translation[1] == 0);
    assert(output.translation[2] == 7654321);

    memset(&output, 0xA5, sizeof(output));
    assert(func_800289E8(-1, &output) == 0);
    assert(func_800289E8(4, &output) == 0);
    for (index = 0; index < sizeof(output); index++) {
        assert(((const uint8_t *)&output)[index] == 0xA5);
    }
    assert(func_800289E8(INT32_MIN, NULL) == 0);
    assert(func_800289E8(INT32_MAX, NULL) == 0);
    g_level_record_count = -1;
    assert(func_800289E8(0, NULL) == 0);
}

static void test_selector(void) {
    unsigned value;
    uint8_t selector = 0xA5;

    reset();
    assert(func_800286F8(-1, &selector) == 0 && selector == 0xA5);
    assert(func_800286F8(INT32_MIN, NULL) == 0);
    g_level_record_count = 0;
    for (value = 0; value < 256; ++value) {
        records[2].selector_04 = (uint8_t)value;
        assert(func_800286F8(2, &selector) == 1);
        assert(selector == value);
    }
}

static void test_table(void) {
    int32_t count;
    int32_t index;
    const MovePreparationEntry *table;

    reset();
    for (index = 0; index < 4; ++index) {
        count = INT32_MAX;
        table = NULL;
        assert(func_80028C18(index, &count, &table) == 1);
        assert(count == records[index].table_count_10 && table == &tables[index]);
    }
    count = 0x12345678;
    table = &tables[0];
    assert(func_80028C18(-1, &count, &table) == 0);
    assert(count == 0x12345678 && table == &tables[0]);
    assert(func_80028C18(INT32_MIN, NULL, NULL) == 0);
    assert(func_80028C18(4, &count, &table) == 0);
    assert(count == 0x12345678 && table == &tables[0]);
    g_level_record_count = 0;
    assert(func_80028C18(0, NULL, NULL) == 0);
    g_level_record_count = -1;
    assert(func_80028C18(0, NULL, NULL) == 0);
}

static void test_objects(void) {
    reset();
    g_level_record_count = 2;
    assert(func_80028764(INT32_MIN) == NULL);
    assert(func_80028764(-1) == NULL);
    assert(fallback_calls == 0);
    assert(func_80028764(0) == &objects[0]);
    records[1].object_1C = NULL;
    assert(func_80028764(1) == NULL && fallback_calls == 0);
    assert(func_80028764(2) == &fallback_object);
    assert(fallback_calls == 1 && fallback_index == 2);
    assert(func_80028764(INT32_MAX) == &fallback_object);
    assert(fallback_calls == 2 && fallback_index == INT32_MAX);
    g_level_record_count = -1;
    assert(func_80028764(0) == &fallback_object);
    assert(fallback_calls == 3 && fallback_index == 0);
}

static void test_list_id(void) {
    int32_t index;
    int32_t list_id = 0x12345678;

    reset();
    assert(func_80028C84(INT32_MIN, NULL) == 0);
    assert(func_80028C84(-1, &list_id) == 0 && list_id == 0x12345678);
    assert(func_80028C84(4, &list_id) == 0 && list_id == 0x12345678);
    assert(func_80028C84(INT32_MAX, NULL) == 0);
    for (index = 0; index < 4; ++index) {
        assert(func_80028C84(index, &list_id) == 1);
        assert(list_id == records[index].list_id_18);
    }
    g_level_record_count = -1;
    assert(func_80028C84(0, NULL) == 0);
}

static void test_list_lookup(void) {
    static const int32_t counts[] = {INT32_MIN, -1, 0, 1, 2, 4};
    static const int32_t ids[] = {INT32_MIN, -1, 0, 1, 2, 4, 5, INT32_MAX};
    unsigned int count_index;
    unsigned int id_index;
    uint32_t value;
    int32_t count;

    reset();
    for (count_index = 0; count_index < sizeof(counts) / sizeof(counts[0]); ++count_index) {
        g_level_list_count = counts[count_index];
        for (id_index = 0; id_index < sizeof(ids) / sizeof(ids[0]); ++id_index) {
            int32_t id = ids[id_index];
            int valid = id > 0 && id <= g_level_list_count;
            int32_t *result;

            count = 0x12345678;
            result = func_80028CD4(id, &count);
            if (valid) {
                assert(result == list_indices[id - 1]);
                assert(count == lists[id - 1].count_02);
            } else {
                assert(result == NULL && count == 0);
            }
        }
    }

    g_level_list_count = 4;
    for (value = 0; value <= UINT16_MAX; ++value) {
        int32_t signed_value = value <= INT16_MAX ? (int32_t)value : (int32_t)value - 0x10000;

        lists[2].count_02 = (int16_t)signed_value;
        count = 0;
        assert(func_80028CD4(3, &count) == list_indices[2]);
        assert(count == signed_value);
    }
}

int main(void) {
    test_selector();
    test_positions();
    test_rotation_lookup();
    test_table();
    test_objects();
    test_list_id();
    test_list_lookup();
    puts("Level records: position, selector, list bounds, signed counts and alternate object lookup passed.");
    return 0;
}
