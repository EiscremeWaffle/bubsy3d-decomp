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

static GameLevelRecord records[4];
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
    memset(lists, 0, sizeof(lists));
    memset(tables, 0, sizeof(tables));
    for (index = 0; index < 4; ++index) {
        records[index].table_count_10 = (int32_t)index * 11 - 7;
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
    test_table();
    test_objects();
    test_list_id();
    test_list_lookup();
    puts("Level records: selector, list bounds, 65536 signed counts and alternate object lookup passed.");
    return 0;
}
