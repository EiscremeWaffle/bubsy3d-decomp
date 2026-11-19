#include "component_rotation.h"
#include "level_records.h"
#include "movement_reset.h"

struct ComponentTransformView;
struct ComponentBasisView;

extern int32_t func_80054C7C(
    struct ComponentTransformView *component,
    const struct ComponentBasisView *basis,
    const int32_t input[3]
);

void func_80028600(MovementResetObject *object, MovementResetComponent *component) {
    GameLevelPosition position;
    ComponentRotationMatrix rotation;
    void *transform;

    func_80028B5C(object->record_id_0C, &position);
    transform = component->transform_14;
    func_80054C7C(transform, 0, position.words);
    func_800289E8(object->record_id_0C, &rotation);
    func_800549E8(transform, &rotation);
    func_800288D8(object->record_id_0C);
    component->field_10 = 0;
    component->flags_04 &= 0xDFu;
}
