#include "component_records.h"

int32_t func_80052094(ComponentRecordOwner *owner, uint32_t index, uint32_t *output) {
    ComponentRecordTable *table = owner->table_18;

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    ComponentRecord *destination = (ComponentRecord *)output;
    destination->first = table->records_00[index].first;
    destination->second = table->records_00[index].second;
#else
    component_record_copy_half(output, table->records_00[index].first.words);
    component_record_copy_half(output + 4, table->records_00[index].second.words);
#endif
    return 0;
}
