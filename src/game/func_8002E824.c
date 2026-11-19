#include "heading_callback_gates.h"

uint32_t func_8002E824(void) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    /* Avoid GCC 2.6.3's redundant volatile-byte mask in this unoptimized getter. */
    return *(const uint8_t *)&g_grounded_case_event;
#else
    return g_grounded_case_event;
#endif
}
