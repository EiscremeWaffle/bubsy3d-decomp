#include "controller_input.h"

extern void func_8006BA84(
    volatile uint8_t *first, int32_t first_size,
    volatile uint8_t *second, int32_t second_size
);
extern void func_8006BA94(void);

int32_t func_8004FCBC(void) {
    func_8006BA84(g_controller_packets[0], 8, g_controller_packets[1], 8);
    func_8006BA94();
    return 0;
}
