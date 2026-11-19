#ifndef BUBSY3D_GTE_BACKGROUND_COLOR_H
#define BUBSY3D_GTE_BACKGROUND_COLOR_H

#include <stdint.h>

void func_80077874(int32_t first, int32_t second, int32_t third);

#ifndef BUBSY3D_MATCH_ORIGINAL_L0
/* Host-side representation of GTE background-color control registers 13..15. */
extern uint32_t g_gte_background_color[3];
#endif

#endif
