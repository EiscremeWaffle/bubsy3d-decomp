#include "actor_context.h"
#include "heading_timed_records.h"

typedef struct ResultLookupSlot {
    int32_t word_00, word_04;
    int8_t byte_08;
    uint8_t padding[3];
    int32_t word_0C, word_10;
} ResultLookupSlot;

typedef struct ResultLookupContext {
    uint8_t unknown[0x14];
    int16_t selection;
    uint8_t padding[2];
    ResultLookupSlot slots[2];
} ResultLookupContext;

extern int32_t func_8001AD28(int32_t value);
extern int32_t func_80037AFC(void);

int32_t func_8002BD0C(int32_t value) {
    ResultLookupContext *context;
    ResultLookupSlot *slot;
    HeadingTimedRecord *record;
    int32_t index;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    register int32_t saved_value __asm__("$16");
    register void *incoming_context __asm__("$5");
    __asm__("" : "=r"(incoming_context));
    saved_value = value;
    context = func_80026648((void *)(uintptr_t)(uint32_t)value, incoming_context);
    __asm__("" : : "r"(saved_value), "r"(context));
#else
    int32_t saved_value = value;
    context = func_80026648((void *)(uintptr_t)(uint32_t)value, 0);
#endif
    index = (int16_t)saved_value;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    slot = (ResultLookupSlot *)(
        (uintptr_t)context + 0x18u +
        (uintptr_t)((int32_t)context->selection * (int32_t)sizeof(*slot)));
#else
    slot = &context->slots[context->selection != 0];
#endif
    record = (HeadingTimedRecord *)(
        (uintptr_t)g_heading_timed_records +
        (uintptr_t)(index * (int32_t)sizeof(*record)));
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    __asm__("" : : "r"(context), "r"(slot), "r"(record));
#endif
    switch (index) {
    case 12: return func_8001AD28(15);
    case 1: return slot->word_04;
    case 0: return slot->byte_08;
    case 2: return slot->word_0C;
    case 4: return slot->word_00;
    case 5:
    case 15:
    case 17:
        return record->duration_30 < 1000 ? record->duration_30 : 999;
    case 16: return func_80037AFC();
    case 13: return context->slots[1].word_04;
    case 14: return context->slots[1].byte_08;
    case 6: return context->slots[0].word_00;
    case 7: return context->slots[1].word_00;
    case 8: return context->slots[0].word_0C;
    case 9: return context->slots[1].word_0C;
    case 10: return context->slots[0].word_10;
    case 11: return context->slots[1].word_10;
    default: return 99;
    }
}
