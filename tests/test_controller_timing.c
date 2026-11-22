#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

typedef struct ProbeInput {
    uint32_t count, status, tick, previous, position, current, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, previous, position, calls, arguments[4], status, tick;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 7 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 10 * sizeof(uint32_t), "probe output layout");

extern uint32_t func_8006BB84(int32_t count);
volatile uint32_t *volatile g_unknown_800A436C;
volatile uint32_t *volatile g_unknown_800A4370;
volatile uint32_t g_unknown_800A4374;
volatile uint32_t g_unknown_800A4378;
volatile uint32_t g_unknown_800A5438;

static volatile uint32_t status_word;
static volatile uint32_t tick_word;
static ProbeInput current;
static ProbeOutput output;

void func_8006BCCC(uint32_t position, int32_t count) {
    assert(output.calls < 2);
    output.arguments[output.calls * 2] = position;
    output.arguments[output.calls * 2 + 1] = (uint32_t)count;
    output.calls++;
    if (output.calls == 1) {
        if (current.mutations & 1U) {
            g_unknown_800A5438 += 7U;
            tick_word += 0x12345U;
        }
        if (current.mutations & 2U) {
            status_word ^= 0x80000000U;
        }
    } else if (current.mutations & 4U) {
        status_word ^= 0x80000000U;
    }
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t count;
    current = *input;
    memset(&output, 0, sizeof(output));
    status_word = input->status;
    tick_word = input->tick;
    g_unknown_800A436C = &status_word;
    g_unknown_800A4370 = &tick_word;
    g_unknown_800A4374 = input->previous;
    g_unknown_800A4378 = input->position;
    g_unknown_800A5438 = input->current;
    memcpy(&count, &input->count, sizeof(count));
    output.result = func_8006BB84(count);
    output.previous = g_unknown_800A4374;
    output.position = g_unknown_800A4378;
    output.status = status_word;
    output.tick = tick_word;
    return output;
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t read_count;

    if (argc == 1) {
        memset(&input, 0, sizeof(input));
        input.count = 3;
        input.tick = 2;
        input.previous = 0xFFFFU;
        input.position = 10;
        input.current = 20;
        input.mutations = 1;
        result = run_probe(&input);
        assert(result.result == 3 && result.calls == 2);
        assert(result.arguments[0] == 12 && result.arguments[1] == 2);
        assert(result.arguments[2] == 28 && result.arguments[3] == 1);
        assert(result.position == 27 && result.previous == 0x12347U);
        puts("controller timing tests passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) return 2;
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
        perror("set binary mode");
        return 2;
    }
#endif
    while ((read_count = fread(&input, 1, sizeof(input), stdin)) != 0) {
        if (read_count != sizeof(input)) {
            fputs("incomplete probe input\n", stderr);
            return 2;
        }
        result = run_probe(&input);
        if (fwrite(&result, sizeof(result), 1, stdout) != 1) {
            perror("write probe output");
            return 2;
        }
    }
    if (ferror(stdin)) {
        fputs("failed to read probe input\n", stderr);
        return 2;
    }
    return 0;
}
