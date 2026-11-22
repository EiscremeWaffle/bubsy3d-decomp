#include "mode_camera_helpers.h"

extern int32_t func_8005F374(void);
extern int32_t func_8005F398(void);
extern int32_t func_8004EFD4(void);
extern void func_80028E50(void);
extern int32_t func_8004EE60(uint32_t camera, uint8_t *rotation);
extern int32_t func_80050824(const uint8_t *rotation);
extern void func_80028EE0(void);
extern void func_80028EA8(void);
extern int32_t func_8004EF98(void);
extern void func_80028E20(void);
extern void func_800280D0(uint8_t *rotation);

void func_80027114(int32_t first, int32_t second) {
    uint8_t rotation[3];

    if ((uint8_t)first != 0) {
        if ((uint8_t)second == 1) func_8005F374();
        func_8004EF10(0, 0, 0);
        func_8004EFD4();
        func_80028E50();
        func_8004EE60(0, rotation);
        rotation[0] -= rotation[0] >> 3;
        rotation[1] -= rotation[1] >> 3;
        rotation[2] -= rotation[2] >> 3;
        func_8004ED78(0, rotation);
        func_8004ED78(1, rotation);
        rotation[0] = rotation[1] = rotation[2] = 0;
        func_80050824(rotation);
    } else {
        if ((uint8_t)second == 1) func_8005F398();
        func_80028EE0();
        func_80028EA8();
        func_8004EF98();
        func_80028E20();
        func_800280D0(rotation);
        func_8004ED78(0, rotation);
        func_8004ED78(1, rotation);
    }
}
