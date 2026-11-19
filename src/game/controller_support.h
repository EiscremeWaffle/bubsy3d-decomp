#ifndef BUBSY3D_CONTROLLER_SUPPORT_H
#define BUBSY3D_CONTROLLER_SUPPORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct ControllerSelectionSource {
    uint8_t unknown_00[0x0C];
    uint32_t selection_0C;
} ControllerSelectionSource;

typedef struct ControllerScreenPosition {
    uint8_t unknown_00[4];
    uint16_t x_04;
    uint16_t y_06;
} ControllerScreenPosition;

typedef struct ControllerScreenOrigin {
    uint16_t x;
    uint16_t y;
} ControllerScreenOrigin;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(ControllerSelectionSource, selection_0C) == 0x0C, "selection offset");
_Static_assert(offsetof(ControllerScreenPosition, x_04) == 0x04, "screen x offset");
_Static_assert(offsetof(ControllerScreenPosition, y_06) == 0x06, "screen y offset");
#endif

extern ControllerSelectionSource *g_controller_selection_source;
extern ControllerScreenOrigin g_controller_screen_origin;

uint32_t func_8004753C(void);
int32_t func_80055D94(ControllerScreenPosition *position, uint32_t x, uint32_t y);

#endif
