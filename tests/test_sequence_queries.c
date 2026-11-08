#include "sequence_queries.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stdio.h>

const int16_t g_sequence_values[32768] = {
    [0] = -32768,
    [1] = 1234,
    [32767] = 32767
};
const char g_sequence_query_condition[] = "query";
const char g_transition_source_path[] = "source";

static int32_t expected_sequence;
static int32_t expected_state;
static int32_t expected_value;
static int32_t backend_status;
static unsigned backend_calls;
static unsigned assertion_calls;
static uint8_t query_value;
static int write_query;
static int mutate_on_assertion;
static uint8_t *query_output;

int32_t func_8005E5C0(int32_t sequence, int32_t state, int32_t value) {
    assert(sequence == expected_sequence && state == expected_state && value == expected_value);
    ++backend_calls;
    return backend_status;
}

int32_t func_8005E7B8(int32_t sequence, int32_t state) {
    assert(sequence == expected_sequence && state == expected_state);
    ++backend_calls;
    return backend_status;
}

int32_t func_8005E9B8(int32_t sequence, int32_t state, uint8_t *output) {
    assert(sequence == expected_sequence && state == expected_state && *output == 0);
    ++backend_calls;
    query_output = output;
    if (write_query) {
        *output = query_value;
    }
    return backend_status;
}

void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line) {
    assert(failed == 0 && condition == g_sequence_query_condition);
    assert(source == g_transition_source_path && line == 0x5CB);
    ++assertion_calls;
    if (mutate_on_assertion) {
        *query_output = 0xE7;
    }
}

static void test_setter_and_related_wrapper(void) {
    static const int32_t indices[] = {0, 1, 32767, 65536, 65537, 98303};
    static const int32_t values[] = {INT32_MIN, -32769, -1, 0, 32767, 32768, 65535, INT32_MAX};
    static const int32_t statuses[] = {INT32_MIN, -1, 0, 1, INT32_MAX};
    unsigned index, state, value, status;

    for (index = 0; index < sizeof(indices) / sizeof(indices[0]); ++index) {
        expected_sequence = g_sequence_values[(int16_t)indices[index]];
        for (state = 0; state < sizeof(values) / sizeof(values[0]); ++state) {
            expected_state = (int16_t)values[state];
            for (status = 0; status < sizeof(statuses) / sizeof(statuses[0]); ++status) {
                backend_status = statuses[status];
                backend_calls = 0;
                assert(func_8001C700(indices[index], values[state]) == (backend_status == 0));
                assert(backend_calls == 1);
                for (value = 0; value < sizeof(values) / sizeof(values[0]); ++value) {
                    expected_value = (int16_t)values[value];
                    backend_calls = 0;
                    assert(func_8001C6BC(indices[index], values[state], values[value]) ==
                        (backend_status == 0));
                    assert(backend_calls == 1);
                }
            }
        }
    }
}

static void test_status_query(void) {
    unsigned value;
    int status;

    expected_sequence = 1234;
    expected_state = -1;
    for (status = -1; status <= 1; ++status) {
        backend_status = status;
        for (value = 0; value < 256; ++value) {
            query_value = (uint8_t)value;
            write_query = 1;
            mutate_on_assertion = 0;
            backend_calls = assertion_calls = 0;
            assert(func_8001C7C4(65537, 65535) == value);
            assert(backend_calls == 1 && assertion_calls == (unsigned)(status != 0));
        }
    }
    write_query = 0;
    backend_status = 0;
    backend_calls = assertion_calls = 0;
    assert(func_8001C7C4(1, -1) == 0);
    assert(backend_calls == 1 && assertion_calls == 0);

    backend_status = 1;
    mutate_on_assertion = 1;
    backend_calls = assertion_calls = 0;
    assert(func_8001C7C4(1, -1) == 0xE7);
    assert(backend_calls == 1 && assertion_calls == 1);
}

int main(void) {
    test_setter_and_related_wrapper();
    test_status_query();
    puts("Sequence queries: signed-halfword inputs, status returns, 256 output values and assertion mutation passed.");
    return 0;
}
