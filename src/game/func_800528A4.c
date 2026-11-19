#include "component_records.h"

int32_t func_800528A4(ComponentRecordOwner *owner, uint32_t index, const uint32_t *input) {
    ComponentRecordTable *table = owner->table_18;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    const ComponentRecord *source = (const ComponentRecord *)input;
    table->records_00[index].first = source->first;
    table->records_00[index].second = source->second;
#else
    component_record_copy_half(table->records_00[index].first.words, input);
    component_record_copy_half(table->records_00[index].second.words, input + 4);
#endif
    return 0;
}
