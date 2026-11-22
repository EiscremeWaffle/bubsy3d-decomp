#include "cyclic_entries.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

extern int32_t func_80022B5C(const int32_t position[3], const int32_t other[3]);

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--binary") == 0) {
        GameCyclicEntry values[2];
        size_t bytes;
#ifdef _WIN32
        if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) {
            perror("set binary mode");
            return 2;
        }
#endif
        while ((bytes = fread(values, 1, sizeof(values), stdin)) != 0) {
            int32_t result;
            if (bytes != sizeof(values)) {
                fputs("incomplete probe input\n", stderr);
                return 2;
            }
            result = func_80022B5C((const int32_t *)values[0].words, (const int32_t *)values[1].words);
            if (fwrite(&result, sizeof(result), 1, stdout) != 1) {
                perror("write probe output");
                return 2;
            }
        }
        if (ferror(stdin)) {
            perror("read probe input");
            return 2;
        }
        return 0;
    }
    if (argc != 1) return 2;
    {
        GameCyclicEntry first = {{100, 9999, 100, 0}}, second = {{0, 0, 0, 0}};
        int32_t three_words[3] = {100, 9999, 100}, origin[3] = {0, 0, 0};
        assert(func_80022B5C((const int32_t *)first.words, (const int32_t *)second.words) == 150);
        assert(func_80022B5C(three_words, origin) == 150);
        first.words[2] = 0;
        assert(func_80022B5C((const int32_t *)first.words, (const int32_t *)second.words) == 100);
    }
    puts("controller cycle distance tests passed");
    return 0;
}
