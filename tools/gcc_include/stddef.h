#ifndef BUBSY_GCC_STDDEF_H
#define BUBSY_GCC_STDDEF_H

typedef unsigned int size_t;

#define offsetof(type, member) ((size_t)&(((type *)0)->member))
#define BUBSY_STATIC_ASSERT_NAME_INNER(line) bubsy_static_assert_##line
#define BUBSY_STATIC_ASSERT_NAME(line) BUBSY_STATIC_ASSERT_NAME_INNER(line)
#define _Static_assert(condition, message) typedef char BUBSY_STATIC_ASSERT_NAME(__LINE__)[(condition) ? 1 : -1]

#endif