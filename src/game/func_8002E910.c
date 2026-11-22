#include <stdint.h>

typedef struct ComponentTransformView ComponentTransformView;
typedef struct ComponentBasisView ComponentBasisView;

extern void *g_unknown_800BE528;
extern int32_t func_80054C7C(
    ComponentTransformView *component, const ComponentBasisView *basis, const int32_t *position
);

void func_8002E910(const int32_t position[3]) {
    func_80054C7C(g_unknown_800BE528, 0, position);
}
