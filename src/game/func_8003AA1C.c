#include <stdint.h>

int32_t func_8003AA1C(void **output) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uintptr_t pointer __asm__("$2") = 0x80180000;
    __asm__ volatile("" : "=r"(pointer) : "0"(pointer));
    *output = (void *)(pointer + 0x6450);
#else
    *output = (void *)0x80186450;
#endif
    return 1;
}