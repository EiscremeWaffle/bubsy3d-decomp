#include "heading_timed_records.h"

void func_8002CA4C(int32_t index, int32_t duration) {
    if (index == 5 || index == 15 || index == 17) {
        g_heading_timed_records[index].duration_30 = duration;
    }
}
