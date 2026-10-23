#include <stdint.h>

extern volatile uint8_t g_grounded_case_event;

void func_8002E7D8(void) {
    g_grounded_case_event = 0;
}

void func_8002E7FC(void) {
    g_grounded_case_event = 1;
}