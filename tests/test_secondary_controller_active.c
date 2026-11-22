#include "secondary_controller_active.h"
#include "level_records.h"
#include "secondary_controller_entry.h"
#include "secondary_controller_helpers.h"
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
    uint32_t buttons, response, gate, level_count, flag506, flag507, mutations;
} ProbeInput;

typedef struct ProbeOutput {
    uint32_t outcome, object, component, gate, level_count, flag506, flag507, count, calls[3];
} ProbeOutput;

uint32_t g_unknown_8009F58C;
int32_t g_level_record_count;
GameLevelRecord *volatile g_level_records;
uint8_t g_secondary_controller_flag_506;
volatile uint8_t g_secondary_controller_flag_507;
static GameLevelRecord level;
static GroundedCase37ComponentView objects[2], owners[2];
static int components[2];
static ProbeInput current;
static ProbeOutput output;

uint32_t func_80056678(uint32_t value, uint32_t selector) {
    assert(value == current.buttons && selector == 7);
    output.calls[output.count++] = 1;
    if (current.mutations & 1) g_unknown_8009F58C ^= 1;
    if (current.mutations & 2) g_level_record_count = 1;
    return current.response;
}

void *func_80040FC0(int32_t index) {
    assert(index == 0);
    output.calls[output.count++] = 2;
    if (current.mutations & 4) {
        g_unknown_8009F58C ^= 1;
        objects[0].owner_1C = &owners[1];
    }
    return &objects[0];
}

int32_t func_8001C6BC(int32_t index, int32_t state, int32_t value) {
    assert(index == 4 && state == 5 && value == 1);
    output.calls[output.count++] = 3;
    if (current.mutations & 8) {
        g_secondary_controller_flag_507 = 1;
        owners[0].transform_14 = &components[1];
        owners[1].transform_14 = &components[1];
    }
    return -1;
}

static ProbeOutput run_probe(const ProbeInput *input) {
    GroundedCase37ComponentView *object = NULL;
    void *component = NULL;
    memset(&output, 0, sizeof(output));
    memset(objects, 0, sizeof(objects));
    memset(owners, 0, sizeof(owners));
    memset(&level, 0, sizeof(level));
    current = *input;
    g_unknown_8009F58C = input->gate;
    memcpy(&g_level_record_count, &input->level_count, sizeof(g_level_record_count));
    g_secondary_controller_flag_506 = (uint8_t)input->flag506;
    g_secondary_controller_flag_507 = (uint8_t)input->flag507;
    g_level_records = &level;
    level.object_1C = &objects[1];
    objects[0].owner_1C = &owners[0];
    objects[1].owner_1C = &owners[1];
    owners[0].transform_14 = &components[0];
    owners[1].transform_14 = &components[1];
    output.outcome = secondary_controller_active_gate(input->buttons, &object, &component);
    output.object = object == &objects[0] ? 1 : object == &objects[1] ? 2 : 0;
    output.component = component == &components[0] ? 1 : component == &components[1] ? 2 : 0;
    output.gate = g_unknown_8009F58C;
    memcpy(&output.level_count, &g_level_record_count, sizeof(output.level_count));
    output.flag506 = g_secondary_controller_flag_506;
    output.flag507 = g_secondary_controller_flag_507;
    return output;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--binary") == 0) {
        ProbeInput input;
        size_t bytes_read;
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
            perror("read probe input");
            return 2;
        }
        return 0;
    }
    if (argc != 1) return 2;
    {
        ProbeInput input = {0x80, 1, 1, 0, 3, 0, 0};
        ProbeOutput result = run_probe(&input);
        assert(result.outcome == SECONDARY_ACTIVE_EXIT && result.count == 1 && result.object == 0);
        input.response = 0;
        input.gate = 0;
        input.mutations = 8;
        result = run_probe(&input);
        assert(result.outcome == SECONDARY_ACTIVE_CONTINUE && result.flag506 == 1);
        assert(result.object == 1 && result.component == 1 && result.count == 3);
    }
    puts("secondary controller active gate tests passed");
    return 0;
}
