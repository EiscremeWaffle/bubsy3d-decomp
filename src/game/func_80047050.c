#include "game_transition.h"

typedef struct ControllerTeardownInitial {
    uint8_t bytes[4];
} ControllerTeardownInitial;

typedef struct ControllerTeardownEvent {
    uint32_t first;
    int16_t second;
    int8_t third;
    uint8_t reserved_07[0x79];
    int32_t width;
    int32_t height;
    int32_t unknown_88;
    int32_t unknown_8c;
    int32_t unknown_90;
    uint8_t unknown_94;
    uint8_t unknown_95;
    uint8_t unknown_96;
    uint8_t reserved_97;
    uint8_t unknown_98;
    uint8_t reserved_99[0x0c];
    uint8_t unknown_a5;
    uint8_t unknown_a6;
    uint8_t reserved_a7[9];
} ControllerTeardownEvent;

typedef struct ControllerTeardownScratch {
    ControllerTeardownInitial initial;
    uint8_t reserved_04[4];
    ControllerTeardownEvent event;
    uint8_t complete;
} ControllerTeardownScratch;

extern ControllerTeardownInitial g_unknown_800BE210;
extern uint8_t g_unknown_800BE214;
extern uint32_t g_unknown_800BE218;
extern int16_t g_unknown_800BE21C;
extern int8_t g_unknown_800BE21E;
extern uint8_t g_unknown_800BE680;
extern void func_80045398(void);
extern void func_80027F40(void);
extern void func_80040634(void);
extern void func_8002C488(void);
extern void func_8005070C(void);
extern int32_t func_80050824(const uint8_t *);
extern int32_t func_8004F428(const uint8_t *);
extern int32_t func_8004F454(int32_t, int32_t, int32_t);
extern void func_80028E50(void);
extern int32_t func_8004EFD4(void);
extern void func_8004F474(int32_t, void *, uint8_t *);
extern int32_t func_800509A4(int32_t, void *, int32_t);
extern void func_8001AE28(void);
extern void func_8004FA18(int32_t, int32_t, void *);
extern int32_t func_80061A0C(uint8_t);
extern int32_t func_80061A64(int32_t);
extern uint32_t func_8006BB84(int32_t);
extern void func_8004F688(void);

void func_80047050(int32_t argument) {
    ControllerTeardownScratch scratch;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register uint32_t first __asm__("$2");
    register int32_t second __asm__("$3");
    register int32_t third __asm__("$4");
#else
    uint32_t first;
    int32_t second;
    int32_t third;
#endif

    scratch.initial = g_unknown_800BE210;
    if (g_unknown_800BE214 != 0) {
        g_unknown_800BE214 = 0;
        func_80045398();
        func_80027F40();
        func_80040634();
        func_8002C488();
        func_8005070C();
        func_80050824(scratch.initial.bytes);
        func_8004F428(scratch.initial.bytes);
        func_8004F454(0, 0, 0);
        func_80028E50();
        func_8004EFD4();
        if ((uint8_t)argument != 0) {
            do {
                func_8004F474(-10, &scratch.initial, &scratch.complete);
                func_800509A4(1, 0, 1);
            } while (scratch.complete == 0);
        }
        func_8001AE28();
        func_8004FA18(0x200, 0x1e0, &scratch.initial);
        first = g_unknown_800BE218;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(first) : "0"(first));
#endif
        second = g_unknown_800BE21C;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(second) : "0"(second));
#endif
        third = g_unknown_800BE21E;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        __asm__ volatile("" : "=r"(third) : "0"(third));
#endif
        scratch.event.first = first;
        scratch.event.second = second;
        scratch.event.third = third;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        /* Preserve the original signed loads and stores before the next call. */
        __asm__ volatile("" : : "m"(scratch.event.third));
#endif
        func_80061A0C(0);
        func_80061A64(30);
        scratch.event.width = 0x200;
        scratch.event.height = 0xf0;
        scratch.event.unknown_98 = 0xfe;
        scratch.event.unknown_90 = -1;
        scratch.event.unknown_a5 = 1;
        g_unknown_800BE680 = 0;
        scratch.event.unknown_8c = 0;
        scratch.event.unknown_88 = 0;
        scratch.event.unknown_96 = 0;
        scratch.event.unknown_95 = 0;
        scratch.event.unknown_94 = 0;
        scratch.event.unknown_a6 = 0;
        func_8001B558(4, 5, 0xffff, 0xffff, &scratch.event, 0, 0, 0);
        func_8006BB84(0);
        func_8004F688();
        func_8006BB84(0);
        func_8004F688();
    }
}
