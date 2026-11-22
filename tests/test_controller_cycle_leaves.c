#include <stdint.h>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

typedef struct ProbeInput {
    uint32_t flags, selector, source[8], index, value;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t flags, result, readback, parameters[8];
} ProbeOutput;

uint32_t g_event_parameters[8];
static uint32_t source[8];
const uint32_t *g_level_event_parameters = source;

extern int32_t func_800226C0(void *actor, uint8_t selector);
extern void func_80028E80(void);
extern void func_80044C04(int32_t index, uint32_t value);
extern uint32_t func_80044C1C(int32_t index);

static ProbeOutput run_probe(const ProbeInput *input) {
    uint32_t actor[2] = {0, input->flags};
    ProbeOutput output;
    assert(input->index < 8);
    memcpy(source, input->source, sizeof(source));
    memset(g_event_parameters, 0xA5, sizeof(g_event_parameters));
    output.result = (uint32_t)func_800226C0(actor, (uint8_t)input->selector);
    output.flags = actor[1];
    func_80028E80();
    func_80044C04((int32_t)input->index, input->value);
    output.readback = func_80044C1C((int32_t)input->index);
    memcpy(output.parameters, g_event_parameters, sizeof(output.parameters));
    return output;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--binary") == 0) {
        ProbeInput input;
        size_t bytes;
#ifdef _WIN32
        if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
            perror("set binary mode");
            return 2;
        }
#endif
        while ((bytes = fread(&input, 1, sizeof(input), stdin)) != 0) {
            ProbeOutput result;
            if (bytes != sizeof(input)) {
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
            perror("read probe input");
            return 2;
        }
        return 0;
    }
    if (argc != 1) return 2;
    {
        ProbeInput input = {0, 16, {0, 1, 2, 3, 4, 5, 6, 7}, 1, 0x12345678};
        ProbeOutput result = run_probe(&input);
        assert(result.flags == 0x10000 && result.result == 0x10000);
        assert(result.readback == 0x12345678 && result.parameters[1] == 0x12345678);
    }
    puts("controller cycle leaf tests passed");
    return 0;
}
