#include "game_transition.h"
#include "input_events.h"
#include "reset_resources.h"

extern void *func_80028764(int32_t index);
extern int32_t func_8003AA1C(void **output);
extern void func_80069758(void);

void func_8001BC28(void) {
    void *actor = func_80028764(0);
    void *context;
    g_event_runtime_byte = 0;
    func_8003AA1C(&context);
    if (((uint8_t *)context)[0x6D] == 0 && ((uint8_t *)context)[0x2B] == 0) {
        if (((uint32_t)func_80045278() << 16) != 0) {
            global_0x800BDE98 += 1u;
            func_80045094();
            func_80069758();
            func_8001B558(4, 1, 0xFFFE, 0xFFFE, (void *)((uintptr_t)actor + 0x0C), 0, 0, 0);
        }
    }
}
