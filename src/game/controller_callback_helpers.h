#ifndef BUBSY3D_CONTROLLER_CALLBACK_HELPERS_H
#define BUBSY3D_CONTROLLER_CALLBACK_HELPERS_H

#include <stdint.h>

#ifdef BUBSY3D_TEST_REAL_TIMED_RECORDS
#include "heading_timed_records.h"
/* The PSX gate aliases record 5's duration; native pointer layout differs. */
#define g_unknown_8009F58C (*(uint32_t *)&g_heading_timed_records[5].duration_30)
#else
extern uint32_t g_unknown_8009F58C;
#endif

extern uint8_t g_unknown_800BE4F8[];
extern void *g_player_model_asset;
extern uint8_t g_unknown_800BDFD2;

void func_80027F40(void);
void func_8002C9D4(int32_t value);

#endif
