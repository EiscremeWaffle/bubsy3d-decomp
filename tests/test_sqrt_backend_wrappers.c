#include <stdint.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

int32_t func_8005359C(int32_t value, int32_t *output);
int32_t func_800535CC(int32_t value, int32_t *output);
int32_t move_vector_length_from_wrapped_squares(
    const int32_t input[3], int32_t *output) __asm__("func_80053904");
int32_t func_80062D4C(const int32_t *left, const int32_t *right, int32_t *output);

static int expected_backend;
static int calls;
static int subtractions;
static int32_t expected_value;
static int32_t backend_result;
static int32_t *mutation_target;

static int32_t invoke_backend(int backend, int32_t value) {
    assert(backend == expected_backend);
    assert(value == expected_value);
    calls++;
    *mutation_target = 0x12345678;
    return backend_result;
}

int32_t func_80076554(int32_t value) {
    return invoke_backend(0, value);
}

int32_t func_800764D0(int32_t value) {
    return invoke_backend(1, value);
}

void func_80010000(const int32_t *left, const int32_t *right, int32_t *output) {
    unsigned index;
    subtractions++;
    for (index = 0; index < 3; index++) {
        output[index] = left[index] - right[index];
    }
}

int main(void) {
    static const int32_t values[] = {
        INT32_MIN, -65536, -4097, -1, 0, 1, 4096, 65536, INT32_MAX,
    };
    int32_t (*const wrappers[])(int32_t, int32_t *) = {func_8005359C, func_800535CC};
    unsigned backend;
    unsigned input;
    unsigned result;
    unsigned x, y, z;

    for (backend = 0; backend < 2; backend++) {
        for (input = 0; input < sizeof(values) / sizeof(values[0]); input++) {
            for (result = 0; result < sizeof(values) / sizeof(values[0]); result++) {
                int32_t output[3] = {0x11223344, 0, 0x55667788};
                expected_backend = (int)backend;
                expected_value = values[input];
                backend_result = values[result];
                mutation_target = &output[1];
                calls = 0;
                assert(wrappers[backend](values[input], &output[1]) == 0);
                assert(calls == 1);
                assert(output[0] == 0x11223344);
                assert(output[1] == values[result]);
                assert(output[2] == 0x55667788);
            }
        }
    }
    for (x = 0; x < sizeof(values) / sizeof(values[0]); x++) {
        for (y = 0; y < sizeof(values) / sizeof(values[0]); y++) {
            for (z = 0; z < sizeof(values) / sizeof(values[0]); z++) {
                int32_t vector[3] = {values[x], values[y], values[z]};
                int32_t output[3] = {0x11223344, 0, 0x55667788};
                uint32_t squares = (uint32_t)vector[0] * (uint32_t)vector[0] +
                                   (uint32_t)vector[1] * (uint32_t)vector[1] +
                                   (uint32_t)vector[2] * (uint32_t)vector[2];
                expected_backend = 1;
                expected_value = (int32_t)squares;
                backend_result = values[(x + y + z) % (sizeof(values) / sizeof(values[0]))];
                mutation_target = &output[1];
                calls = 0;
                assert(move_vector_length_from_wrapped_squares(vector, &output[1]) == 0);
                assert(calls == 1);
                assert(output[0] == 0x11223344);
                assert(output[1] == backend_result);
                assert(output[2] == 0x55667788);
            }
        }
    }
    {
        const int32_t left[3] = {10, 20, 30};
        const int32_t right[3] = {7, 16, 30};
        int32_t output[3] = {0x11223344, 0, 0x55667788};
        expected_backend = 1;
        expected_value = 25;
        backend_result = 5;
        mutation_target = &output[1];
        calls = 0;
        subtractions = 0;
        assert(func_80062D4C(left, right, &output[1]) == 0);
        assert(subtractions == 1);
        assert(calls == 1);
        assert(output[0] == 0x11223344 && output[1] == 5 && output[2] == 0x55667788);
    }
    puts("Square-root wrappers and wrapped vector-length forwarding passed");
    return 0;
}
