#include "movement_reset.h"

void func_80028698(MovementResetObject *object) {
    MovementResetComponent *first = object->component_20;
    MovementResetComponent *selected = object->component_20;
    MovementResetComponent *second;

    object->field_2C = 0;
    object->field_10 = 0;
    object->field_04 = 0;
    object->field_08 = 0;
    object->field_14 = 0;
    object->component_1C = selected;
    func_80028600(object, first);
    second = object->component_24;
    if (second != 0) {
        func_80028600(object, second);
    }
}
