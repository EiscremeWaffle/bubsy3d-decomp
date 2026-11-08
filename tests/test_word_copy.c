#include "word_copy.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint32_t actual[96];
    uint32_t expected[96];
    unsigned int source, destination, count, index;
    unsigned int cases = 0;

    for (source = 16; source <= 40; ++source) {
        for (destination = 16; destination <= 40; ++destination) {
            for (count = 1; count <= 24; ++count) {
                for (index = 0; index < 96; ++index) {
                    actual[index] = expected[index] = 0x9E3779B9u * (index + 1u);
                }
                for (index = 0; index < count; ++index) {
                    expected[destination + index] = expected[source + index];
                }
                assert(func_80010220(actual + destination, actual + source, count) == 0);
                assert(memcmp(actual, expected, sizeof(actual)) == 0);
                ++cases;
            }
        }
    }
    printf("Forward word copy: %u lengths, offsets and overlapping cases passed.\n", cases);
    return 0;
}
