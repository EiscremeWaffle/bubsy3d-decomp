#include "sequence_limit.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

typedef struct ProbeInput {
    uint32_t first, second, selector_result, backend_result, which;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t result, selector_calls, selector_first, selector_second;
    uint32_t backend_calls, backend_first, backend_second, backend_kind;
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 5 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 8 * sizeof(uint32_t), "probe output layout");

static int32_t selector_result, backend_result;
static ProbeOutput output;

static int32_t signed_bits(uint32_t value) {
    int32_t result;
    memcpy(&result, &value, sizeof(result));
    return result;
}

int32_t func_8005D5A8(int32_t first, int32_t second) {
    ++output.selector_calls;
    output.selector_first = (uint32_t)first;
    output.selector_second = (uint32_t)second;
    return selector_result;
}

static int32_t backend(int32_t first, int32_t second, uint32_t kind) {
    ++output.backend_calls;
    output.backend_first = (uint32_t)first;
    output.backend_second = (uint32_t)second;
    output.backend_kind = kind;
    return backend_result;
}

int32_t func_8008DBF8(int32_t first, int32_t second) {
    return backend(first, second, 1);
}

int32_t func_8008DF28(int32_t first, int32_t second) {
    return backend(first, second, 2);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t first = signed_bits(input->first);
    int32_t second = signed_bits(input->second);

    if (input->which > 1) {
        fputs("invalid sequence-wrapper selection\n", stderr);
        exit(EXIT_FAILURE);
    }
    memset(&output, 0, sizeof(output));
    selector_result = signed_bits(input->selector_result);
    backend_result = signed_bits(input->backend_result);
    if (input->which == 0) {
        output.result = (uint32_t)func_8005E708(first, second);
    } else {
        output.result = (uint32_t)func_8005E760(first, second);
    }
    return output;
}

static int32_t signed_halfword(uint32_t value) {
    uint32_t halfword = value & 0xFFFFU;
    return halfword < 0x8000U ? (int32_t)halfword : (int32_t)halfword - 0x10000;
}

static void self_test(void) {
    static const uint32_t values[] = {0, 1, 0x7FFF, 0x8000, 0xFFFF, 0x1234FFFF};
    static const uint32_t statuses[] = {0, 1, 0x1B, 0x80000000U, 0xFFFFFFFFU};
    size_t first, second, status;
    uint32_t which;

    for (which = 0; which < 2; ++which) {
        for (first = 0; first < sizeof(values) / sizeof(values[0]); ++first) {
            for (second = 0; second < sizeof(values) / sizeof(values[0]); ++second) {
                for (status = 0; status < sizeof(statuses) / sizeof(statuses[0]); ++status) {
                    ProbeInput input = {values[first], values[second], statuses[status], 0xABCDEF12U, which};
                    ProbeOutput actual = run_probe(&input);
                    assert(actual.result == input.selector_result);
                    assert(actual.selector_calls == 1);
                    assert(actual.selector_first == (uint32_t)signed_halfword(input.first));
                    assert(actual.selector_second == (uint32_t)signed_halfword(input.second));
                    assert(actual.backend_calls == (input.selector_result == 0));
                    if (actual.backend_calls != 0) {
                        assert(actual.backend_first == actual.selector_first);
                        assert(actual.backend_second == actual.selector_second);
                        assert(actual.backend_kind == which + 1);
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    ProbeInput input;
    size_t bytes_read;

    if (argc == 1) {
        self_test();
        puts("sequence-effect wrapper tests passed");
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--binary") != 0) {
        fputs("expected --binary or no arguments\n", stderr);
        return 2;
    }
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
        perror("set binary mode");
        return 2;
    }
#endif
    while ((bytes_read = fread(&input, 1, sizeof(input), stdin)) != 0) {
        ProbeOutput result;
        if (bytes_read != sizeof(input)) {
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
