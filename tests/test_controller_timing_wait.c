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
    uint32_t position, count, current;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t calls, arguments[3];
} ProbeOutput;

_Static_assert(sizeof(ProbeInput) == 3 * sizeof(uint32_t), "probe input layout");
_Static_assert(sizeof(ProbeOutput) == 4 * sizeof(uint32_t), "probe output layout");

extern void func_8006BCCC(uint32_t position, int32_t count);
volatile uint32_t g_unknown_800A5438;
const char g_unknown_80013EA8[] = "timing failure";
static ProbeOutput output;

void func_80082E4C(const char *message) {
    assert(message == g_unknown_80013EA8);
    assert(output.calls++ == 0);
    output.arguments[0] = 1;
}

void func_8006BB74(int32_t value) {
    assert(output.calls++ == 1);
    output.arguments[1] = (uint32_t)value;
}

void func_8006BD68(int32_t value, int32_t argument) {
    assert(output.calls++ == 2);
    output.arguments[2] = (uint32_t)value;
    assert(argument == 0);
}

static ProbeOutput run_probe(const ProbeInput *input) {
    int32_t count;
    memset(&output, 0, sizeof(output));
    g_unknown_800A5438 = input->current;
    memcpy(&count, &input->count, sizeof(count));
    func_8006BCCC(input->position, count);
    return output;
}

int main(int argc, char **argv) {
    ProbeInput input;
    ProbeOutput result;
    size_t read_count;
    if (argc == 1) {
        input.position = 2;
        input.count = 0;
        input.current = 1;
        result = run_probe(&input);
        assert(result.calls == 3 && result.arguments[0] == 1);
        assert(result.arguments[1] == 0 && result.arguments[2] == 3);
        input.position = 1;
        result = run_probe(&input);
        assert(result.calls == 0);
        puts("controller timing wait tests passed");
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
