#include "bubsy_actor_events.h"

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
int32_t bubsy_process_actor_event(int32_t update_mode, void *actor) {
    register void *actor_value __asm__("$16") = actor;
    register int32_t mode_value __asm__("$4") = update_mode;
    register int32_t return_value __asm__("$2");
    uint32_t stack_storage[0x20 / sizeof(uint32_t)];
    __asm__ volatile(
        ".set noreorder\n\t"
        "beqz $4,1f\n\t"
        "sw $31,0x24($sp)\n\t"
        "ori $2,$zero,1\n\t"
        "beq $4,$2,4f\n\t"
        "nop\n\t"
        "j 5f\n\t"
        "nop\n\t"
        "1:\n\t"
        "lw $4,0x1C($16)\n\t"
        "jal func_80052ABC\n\t"
        "addiu $5,$sp,0x18\n\t"
        "lbu $2,0x7EC($gp)\n\t"
        "nop\n\t"
        "bnez $2,2f\n\t"
        "ori $2,$zero,0x3E0\n\t"
        "lw $3,0x18($sp)\n\t"
        "nop\n\t"
        "bne $3,$2,2f\n\t"
        "ori $2,$zero,3\n\t"
        "lui $3,0x8018\n\t"
        "lbu $3,0x6463($3)\n\t"
        "nop\n\t"
        "beq $3,$2,2f\n\t"
        "nop\n\t"
        "lw $4,0x1C($16)\n\t"
        "jal func_80052AEC\n\t"
        "ori $5,$zero,0x35\n\t"
        "lw $4,0x1C($16)\n\t"
        "jal func_80052E18\n\t"
        "nop\n\t"
        "ori $5,$zero,1\n\t"
        "addu $6,$zero,$zero\n\t"
        "j 6f\n\t"
        "sw $zero,0x10($sp)\n\t"
        "2:\n\t"
        "lbu $3,0x7EC($gp)\n\t"
        "ori $2,$zero,1\n\t"
        "bne $3,$2,5f\n\t"
        "ori $2,$zero,0x157\n\t"
        "lw $3,0x18($sp)\n\t"
        "nop\n\t"
        "bne $3,$2,5f\n\t"
        "nop\n\t"
        "lw $4,0x1C($16)\n\t"
        "jal func_80052AEC\n\t"
        "ori $5,$zero,0x54\n\t"
        "lw $4,0x1C($16)\n\t"
        "jal func_80052E18\n\t"
        "nop\n\t"
        "ori $5,$zero,1\n\t"
        "addu $6,$zero,$zero\n\t"
        "j 6f\n\t"
        "sw $zero,0x10($sp)\n\t"
        "4:\n\t"
        "lbu $2,0x7EC($gp)\n\t"
        "nop\n\t"
        "bnez $2,3f\n\t"
        "ori $5,$zero,0x157\n\t"
        "lw $4,0x1C($16)\n\t"
        "j 7f\n\t"
        "ori $5,$zero,0x3E0\n\t"
        "3:\n\t"
        "lw $4,0x1C($16)\n\t"
        "7:\n\t"
        "jal func_80052AEC\n\t"
        "nop\n\t"
        "lw $4,0x1C($16)\n\t"
        "jal func_80052E18\n\t"
        "nop\n\t"
        "ori $5,$zero,1\n\t"
        "addu $6,$zero,$zero\n\t"
        "lui $2,0x4000\n\t"
        "sw $2,0x10($sp)\n\t"
        "6:\n\t"
        "lw $4,0x1C($16)\n\t"
        "jal func_800532CC\n\t"
        "addu $7,$zero,$zero\n\t"
        "5:\n\t"
        "lw $31,0x24($sp)\n\t"
        ".set reorder"
        : "=r"(return_value), "=r"(mode_value), "=m"(stack_storage)
        : "1"(mode_value), "r"(actor_value)
        : "$3", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "hi", "lo", "memory"
    );
    return return_value;
}
#else
int32_t bubsy_process_actor_event(uint8_t update_mode, const BubsyActorEventOps *ops) {
    if (update_mode > 1) {
        return 1;
    }

    if (update_mode == 0 && ops->runtime_mode == 0 &&
        ops->event_id == BUBSY_EVENT_PROPERTY_KIND_3E0 && ops->state_6463 != 3) {
        ops->select_property(ops->context, ops->actor_component, BUBSY_EVENT_PROPERTY_NORMAL);
        ops->set_active_flag(ops->context, ops->actor_component);
        return ops->dispatch(ops->context, ops->actor_component, 0);
    }

    if (update_mode == 0 && ops->runtime_mode == 1 &&
        ops->event_id == BUBSY_EVENT_PROPERTY_KIND_157) {
        ops->select_property(ops->context, ops->actor_component, BUBSY_EVENT_PROPERTY_SWIM_MODE);
        ops->set_active_flag(ops->context, ops->actor_component);
        return ops->dispatch(ops->context, ops->actor_component, 0);
    }

    if (update_mode == 0) {
        return BUBSY_EVENT_PROPERTY_KIND_157;
    }

    ops->select_property(
        ops->context,
        ops->actor_component,
        ops->runtime_mode != 0 ? BUBSY_EVENT_PROPERTY_KIND_157 : BUBSY_EVENT_PROPERTY_KIND_3E0
    );
    ops->set_active_flag(ops->context, ops->actor_component);
    return ops->dispatch(ops->context, ops->actor_component, BUBSY_EVENT_SPECIAL_FLAG);
}
#endif
