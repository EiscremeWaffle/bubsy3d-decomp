#include <stdint.h>

extern void func_8002BAD4(int32_t index);

void func_8002C488(void) {
    int32_t index;

    for (index = 0; index < 19; ++index) {
        func_8002BAD4((int16_t)index);
    }
}
