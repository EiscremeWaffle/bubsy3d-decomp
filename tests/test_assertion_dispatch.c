#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

const char g_assertion_format[] = "assert format";
const char g_assertion_halted[] = "program halted";
uint8_t g_assertion_print_enabled;
void (*g_assertion_handler)(void);
extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

static unsigned int log_calls;
static uintptr_t last_format;
static const char *last_condition;
static const char *last_source;
static uint32_t last_line;
static int32_t halt_argument;
static unsigned int handler_calls;
static uint8_t enable_logging_in_handler;

void func_80082F0C(const char *format, ...) {
    ++log_calls;
    last_format = (uintptr_t)format;
    if (last_format == (uintptr_t)g_assertion_format && log_calls > 2) {
        va_list args;
        va_start(args, format);
        last_condition = va_arg(args, const char *);
        last_source = va_arg(args, const char *);
        last_line = va_arg(args, uint32_t);
        va_end(args);
    }
}

void func_80082E3C(int32_t value) {
    halt_argument = value;
}

static void assertion_handler(void) {
    ++handler_calls;
    g_assertion_print_enabled = enable_logging_in_handler;
}

static void reset_state(void) {
    log_calls = 0;
    last_format = 0;
    last_condition = 0;
    last_source = 0;
    last_line = 0;
    halt_argument = 0;
    handler_calls = 0;
    enable_logging_in_handler = 0;
    g_assertion_print_enabled = 0;
    g_assertion_handler = 0;
}

int main(void) {
    static const char condition[] = "condition";
    static const char source[] = "source.c";

    reset_state();
    func_800556D0(0, condition, source, 123);
    assert(log_calls == 0);
    assert(handler_calls == 0);

    reset_state();
    g_assertion_print_enabled = 2;
    func_800556D0(0x100, condition, source, 123);
    assert(log_calls == 0);
    assert(halt_argument == 0);

    reset_state();
    enable_logging_in_handler = 1;
    g_assertion_handler = assertion_handler;
    func_800556D0(-1, condition, source, 0x12345678u);
    assert(log_calls == 3);
    assert(last_format == (uintptr_t)g_assertion_format);
    assert(last_condition == condition);
    assert(last_source == source);
    assert(last_line == 0x12345678u);
    assert(handler_calls == 1);
    assert(halt_argument == 0);

    reset_state();
    func_800556D0(1, condition, source, 456);
    assert(log_calls == 2);
    assert(last_format == (uintptr_t)g_assertion_halted);
    assert(halt_argument == -1);

    reset_state();
    g_assertion_print_enabled = 1;
    func_800556D0(0, condition, source, 789);
    assert(log_calls == 1);
    assert(last_format == (uintptr_t)g_assertion_format);

    return 0;
}
