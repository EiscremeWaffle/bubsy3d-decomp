#include "bubsy_death_state.h"

int32_t bubsy_death_state_normalize_initial(
    int32_t death_state,
    BubsyDeathStateAssertFailure assert_failure
) {
    if (death_state >= 0) {
        return death_state;
    }

    if (assert_failure != 0) {
        assert_failure(
            0,
            "gBubsyInfo.deathType >= 0",
            "../f/bubsy.c",
            0x912
        );
    }
    return 1;
}

int32_t bubsy_advance_death_state(int32_t death_state, int16_t level_id, int32_t counter_value) {
    death_state = bubsy_death_state_normalize_initial(death_state, 0);

    if (death_state == 1) {
        const int32_t remainder = counter_value % 3;
        if (remainder == 1) {
            death_state = 7;
        } else if (remainder == 2) {
            death_state = 9;
        }
    } else if (death_state == 2) {
        if (counter_value % 2 == 1) {
            death_state = 10;
        }
    } else if (death_state == 4) {
        if (level_id == 4 || level_id == 7 || level_id == 9 || level_id == 18) {
            death_state = 6;
        } else if (level_id == 6) {
            death_state = 8;
        } else if (level_id == 8) {
            death_state = 11;
        }
    } else if (level_id == 5 || level_id == 8) {
        death_state = 11;
    }

    if (death_state == 11 && (level_id == 4 || level_id == 6 || level_id == 8)) {
        death_state = counter_value % 2 == 0 ? 12 : 13;
    }

    return death_state;
}

int32_t bubsy_death_state_should_dispatch_entry_event(
    int32_t global_gate_3A4,
    uint8_t helper_result_low_byte
) {
    return global_gate_3A4 == -2 && helper_result_low_byte == 0;
}

int32_t bubsy_death_state_should_run_level13_cleanup(int16_t level_id) {
    return level_id == 0x13;
}

int32_t bubsy_death_state_should_run_main_loop(uint8_t state_6462) {
    return state_6462 == 0;
}

int32_t bubsy_death_state_uses_counter_entry_list(int32_t global_counter_3A4) {
    return global_counter_3A4 == -2 || global_counter_3A4 == -1;
}

int32_t bubsy_death_state_next_global_counter(
    int32_t global_counter_3A4,
    uint8_t helper_result_low_byte
) {
    if (global_counter_3A4 == -2) {
        return -1;
    }

    return helper_result_low_byte == 1 ? 0 : global_counter_3A4;
}

void bubsy_death_state_reset_runtime(
    BubsyDeathStateRuntimeView *runtime,
    uint8_t actor_mode_0C
) {
    runtime->actor_flags_04 = 0;
    runtime->actor_state_10 = 0;
    runtime->actor_event_guard_6460 = 0;
    runtime->state_6461 = 0;
    runtime->state_6463 = actor_mode_0C != 0;
    runtime->state_6479 = 0;
    runtime->state_64B8 = 0;
    runtime->state_649D = 0;
    runtime->state_6452 = 0;
    runtime->state_64C0 = 0;
}

void bubsy_death_state_finalize_runtime(BubsyDeathStateFinalizationView *runtime) {
    unsigned int index;

    *runtime->actor_state_2C = 0;
    *runtime->actor_component_state_20 = 0;
    for (index = 0; index < 11; index++) {
        *runtime->nested_state_words[index] = 0;
    }
    *runtime->global_counter_3A4 = -2;
    *runtime->state_6462 = 1;
    *runtime->state_6464 = 0;
}

int32_t bubsy_death_state_should_dispatch_counter_entry_event(
    int32_t global_counter_3A4,
    uint8_t state_6462,
    int32_t entry_count,
    uint8_t entry_flag_20
) {
    return (global_counter_3A4 == -2 || global_counter_3A4 == -1) &&
        state_6462 == 0 &&
        entry_count > 0 &&
        entry_flag_20 == 0;
}

#ifdef BUBSY3D_MATCH_ORIGINAL_L0
#define BUBSY_GLOBAL_COUNTER (*(volatile int32_t *)0x800BE100u)

extern void *func_80026648(void *actor, void *context);
extern void func_80028FC8(void);
extern int32_t func_800222E0(void);
extern int32_t func_80082ECC(void);
extern void func_8002DD24(volatile uint8_t *state);
extern void func_8005F2C0(int32_t value, int32_t value_copy);
extern void func_80037214(int32_t mode, void *actor);
extern void func_8003539C(void *actor, int32_t mode);
extern void func_800289E8(int32_t mode, void *output);
extern void func_800549E8(void *context, void *input);
extern void func_80054C7C(void *object, int32_t mode, void *data);
extern void func_8001B9C8(void);
extern void func_8001BC08(void);
extern void func_8002C488(void);
extern void func_8002CB2C(int32_t value);
extern void func_8002CA88(int32_t value, int32_t duration);
extern void func_8002BAD4(int32_t value);
extern void func_8003CB14(void);
extern void func_80039158(int32_t mode, int32_t flags);
extern void func_80048488(void);
extern void func_80040634(void);
extern void func_8005FBE4(void);
extern void func_80040994(void);
extern void func_8005EF9C(int32_t mode, int32_t duration, void *callback);
extern void func_800516F8(void *output);
extern void func_80054AF0(void *context, void *output);
extern void func_80052BDC(void *context, int32_t mode);
extern void func_8002E954(void *state, void *value);
extern void func_8002E858(void *input, int32_t size, void *source, void *output);
extern void func_800550A4(void *object, void *context);
extern void func_80045094(void);
extern void func_80044268(void);
extern void func_80069758(void);
extern void func_8003AA30(void);
extern void func_8001B558(
    int32_t event_id,
    int32_t property_id,
    int32_t actor_value,
    int32_t actor_value_copy,
    int32_t argument_10,
    int32_t argument_14,
    int32_t argument_18,
    int32_t argument_1C
);
extern void func_8001C700(int32_t event_id, int32_t property_id);
extern void func_8004733C(int32_t value);
extern void func_8001A670(void);
extern void func_800556D0(
    int32_t failed,
    const char *condition,
    const char *source_path,
    uint32_t source_line
);

void bubsy_handle_death_state(void *actor, void *context) {
    register volatile uint8_t *actor_bytes __asm__("$20") =
        (volatile uint8_t *)actor;
    register void *context_value __asm__("$21") = context;
    register volatile uint8_t *nested_state __asm__("$18") =
        *(volatile uint8_t **)(actor_bytes + 0x28);
    register void *death_info __asm__("$22");
    register volatile int32_t *death_type __asm__("$17");
    volatile uint8_t *actor_component;
    register int32_t current_death_state __asm__("$16");
    register int32_t death_state_one __asm__("$19");
    uint32_t stack_storage[0x118 / sizeof(uint32_t)];

    death_info = func_80026648((void *)actor_bytes, context);
        __asm__ volatile("" : "=r"(death_info) : "0"(death_info));
    {
        __asm__ volatile(
            "lw $3,0x3A4($gp)\n\t"
            "li $2,-2\n\t"
            "sh $0,0x362($gp)\n\t"
            "bne $3,$2,1f\n\t"
            "nop\n\t"
            "jal func_800222E0\n\t"
            "nop\n\t"
            "andi $2,$2,0x00ff\n\t"
            "bnez $2,1f\n\t"
            "ori $4,$zero,0x1b\n\t"
            "sw $0,0x10($sp)\n\t"
            "sw $0,0x14($sp)\n\t"
            "sw $0,0x18($sp)\n\t"
            "sw $0,0x1C($sp)\n\t"
            "lw $6,0x0c($20)\n\t"
            "j 0x8003764c\n\t"
            "ori $5,$zero,0x04\n"
            "1:"
            :
            :
            : "$2", "$3", "$4", "$5", "$6", "$7", "$31", "memory"
        );
    }

continue_main_path:
    __asm__ volatile(
        "lui $3,0x801d\n\t"
        "lh $3,0x36f0($3)\n\t"
        "li $2,0x13\n\t"
        "bne $3,$2,1f\n\t"
        "nop\n\t"
        "jal func_8004733C\n\t"
        "move $4,$0\n\t"
        "j 0x80037ad0\n\t"
        "nop\n\t"
        "1:"
        :
        :
        : "$2", "$3", "$4", "$31", "memory"
    );
    if (*(volatile const uint8_t *)0x80186462u != 0) {
        return;
    }
    {
        int32_t counter_list_gate;
        __asm__ volatile(
            "lw $2,0x3a4($gp)\n\t"
            "nop\n\t"
            "addiu $2,$2,2\n\t"
            "sltiu $2,$2,2"
            : "=r"(counter_list_gate)
            :
            : "memory"
        );
        if (counter_list_gate != 0) {
        register int32_t list_initial_value __asm__("$2");
        __asm__ volatile(
            ".set noat\n\t"
            "sw $2,0x118($sp)\n\t"
            "lui $3,0x8018\n\t"
            "lw $3,0x645c($3)\n\t"
            "sb $0,0x102($sp)\n\t"
            "sb $0,0x101($sp)\n\t"
            "blez $3,1f\n\t"
            "sb $0,0x100($sp)\n\t"
            "sll $2,$3,3\n\t"
            "addu $2,$2,$3\n\t"
            "lui $3,0x801e\n\t"
            "lw $3,-0x765c($3)\n\t"
            "sll $2,$2,2\n\t"
            "addu $2,$2,$3\n\t"
            "lbu $2,0x20($2)\n\t"
            "nop\n\t"
            "bnez $2,2f\n\t"
            "li $2,-1\n\t"
            "li $4,0x1b\n\t"
            "sw $0,0x10($sp)\n\t"
            "sw $0,0x14($sp)\n\t"
            "sw $0,0x18($sp)\n\t"
            "sw $0,0x1C($sp)\n\t"
            "lw $6,0x0c($20)\n\t"
            "lui $7,0x8018\n\t"
            "lw $7,0x645c($7)\n\t"
            "jal func_8001B558\n\t"
            "li $5,5\n\t"
            "2:\n\t"
            "li $2,-1\n\t"
            "lui $1,0x8018\n\t"
            "sw $2,0x645c($1)\n\t"
            "1:\n\t"
            "jal func_8003CB14\n\t"
            "nop\n\t"
            "move $4,$0\n\t"
            "jal func_80039158\n\t"
            "move $5,$0\n\t"
            "jal func_8002C488\n\t"
            "nop\n\t"
            "jal func_80048488\n\t"
            "nop\n\t"
            "jal func_80040634\n\t"
            "nop\n\t"
            "jal func_8005FBE4\n\t"
            "nop\n\t"
            "jal func_80040994\n\t"
            "nop\n\t"
            "lw $3,0x3a4($gp)\n\t"
            "li $2,-2\n\t"
            "bne $3,$2,3f\n\t"
            "nop\n\t"
            "jal func_8002CB2C\n\t"
            "li $4,5\n\t"
            "move $4,$0\n\t"
            "lui $6,0x8003\n\t"
            "addiu $6,$6,0x7350\n\t"
            "jal func_8005EF9C\n\t"
            "li $5,0x3c\n\t"
            "lbu $3,0x7ec($gp)\n\t"
            "li $2,1\n\t"
            "bne $3,$2,4f\n\t"
            "li $4,5\n\t"
            "lw $2,0x10($20)\n\t"
            "lui $3,0x10\n\t"
            "and $2,$2,$3\n\t"
            "beqz $2,4f\n\t"
            "move $5,$0\n\t"
            "lw $2,0x7fc($gp)\n\t"
            "nop\n\t"
            "lw $4,0x14($2)\n\t"
            "addiu $6,$sp,0x108\n\t"
            "sw $0,0x10c($sp)\n\t"
            "sw $0,0x110($sp)\n\t"
            "jal func_80054C7C\n\t"
            "sw $0,0x108($sp)\n\t"
            "lw $4,0x7fc($gp)\n\t"
            "jal func_80052BDC\n\t"
            "li $5,1\n\t"
            "sw $0,0x364($gp)\n\t"
            "li $4,5\n\t"
            "jal func_8001C700\n\t"
            "li $5,3\n\t"
            "li $4,5\n\t"
            "jal func_8001C700\n\t"
            "li $5,5\n\t"
            "li $4,0x0f\n\t"
            "jal func_8002CA88\n\t"
            "li $5,0xb4\n\t"
            "jal func_8002BAD4\n\t"
            "li $4,0x0f\n\t"
            "j 5f\n\t"
            "sb $0,0($20)\n\t"
            "4:\n\t"
            "jal func_8001C700\n\t"
            "li $5,5\n\t"
            "li $4,5\n\t"
            "jal func_8001C700\n\t"
            "li $5,8\n\t"
            "5:\n\t"
            "jal func_8001BC08\n\t"
            "nop\n\t"
            "jal func_8001B9C8\n\t"
            "nop\n\t"
            "li $4,0x2c\n\t"
            "move $5,$0\n\t"
            "li $6,0xfffe\n\t"
            "li $7,0xfffe\n\t"
            "addiu $2,$sp,0x118\n\t"
            "sw $2,0x10($sp)\n\t"
            "addiu $2,$sp,0x100\n\t"
            "sw $2,0x14($sp)\n\t"
            "addiu $2,$sp,0x101\n\t"
            "sw $2,0x18($sp)\n\t"
            "addiu $2,$sp,0x102\n\t"
            "jal func_8001B558\n\t"
            "sw $2,0x1c($sp)\n\t"
            "li $2,-1\n\t"
            "sw $2,0x3a4($gp)\n\t"
            "j 6f\n\t"
            "li $4,0x1b\n\t"
            "3:\n\t"
            "jal func_800222E0\n\t"
            "nop\n\t"
            "andi $2,$2,0x00ff\n\t"
            "li $3,1\n\t"
            "bne $2,$3,6f\n\t"
            "li $4,0x1b\n\t"
            "sw $0,0x3a4($gp)\n\t"
            "6:\n\t"
            "sw $0,0x10($sp)\n\t"
            "sw $0,0x14($sp)\n\t"
            "sw $0,0x18($sp)\n\t"
            "sw $0,0x1c($sp)\n\t"
            "lw $6,0x0c($20)\n\t"
            "move $5,$0\n\t"
            "jal func_8001B558\n\t"
            "move $7,$6\n\t"
            ".set at"
            : "=r"(list_initial_value)
            : "0"(-0x32)
            : "$1", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "hi", "lo", "memory"
        );
        return;
        }
    }
    func_80028FC8();
    if (*(volatile int32_t *)0x80186458u < 0) {
        func_800556D0(
            0,
            (const char *)0x80012A20u,
            (const char *)0x8001298Cu,
            0x912
        );
        if (*(volatile int32_t *)0x80186458u < 0) {
            *(volatile int32_t *)0x80186458u = 1;
        }
    }
    __asm__ volatile(
        ".set noat\n\t"
        "lui $17,0x8018\n\t"
        "addiu $17,$17,0x6458\n\t"
        "lw $16,0($17)\n\t"
        "nop\n\t"
        "ori $19,$zero,1\n\t"
        "bne $16,$19,2f\n\t"
        "ori $2,$zero,2\n\t"
        "jal func_80082ECC\n\t"
        "nop\n\t"
        "lui $3,0x5555\n\t"
        "ori $3,$3,0x5556\n\t"
        "mult $2,$3\n\t"
        "sra $4,$2,31\n\t"
        "mfhi $3\n\t"
        "subu $4,$3,$4\n\t"
        "sll $3,$4,1\n\t"
        "addu $3,$3,$4\n\t"
        "subu $4,$2,$3\n\t"
        "beqz $4,1f\n\t"
        "nop\n\t"
        "bne $4,$19,11f\n\t"
        "ori $2,$zero,2\n\t"
        "ori $2,$zero,7\n\t"
        "j 8f\n\t"
        "sw $2,0($17)\n\t"
        "11:\n\t"
        "bne $4,$2,7f\n\t"
        "ori $2,$zero,9\n\t"
        "j 8f\n\t"
        "sw $2,0($17)\n\t"
        "1:\n\t"
        "j 8f\n\t"
        "sw $16,0($17)\n\t"
        "2:\n\t"
        "bne $16,$2,3f\n\t"
        "ori $2,$zero,4\n\t"
        "jal func_80082ECC\n\t"
        "nop\n\t"
        "srl $3,$2,31\n\t"
        "addu $3,$2,$3\n\t"
        "sra $4,$3,1\n\t"
        "sll $3,$4,1\n\t"
        "subu $4,$2,$3\n\t"
        "bnez $4,4f\n\t"
        "nop\n\t"
        "j 8f\n\t"
        "sw $16,0($17)\n\t"
        "4:\n\t"
        "bne $4,$19,7f\n\t"
        "ori $2,$zero,10\n\t"
        "j 8f\n\t"
        "sw $2,0($17)\n\t"
        "3:\n\t"
        "bne $16,$2,7f\n\t"
        "ori $2,$zero,5\n\t"
        "lui $3,0x801d\n\t"
        "lh $3,0x36f0($3)\n\t"
        "nop\n\t"
        "beq $3,$2,6f\n\t"
        "ori $2,$zero,7\n\t"
        "beq $3,$2,6f\n\t"
        "ori $2,$zero,9\n\t"
        "beq $3,$2,6f\n\t"
        "ori $2,$zero,18\n\t"
        "bne $3,$2,5f\n\t"
        "nop\n\t"
        "6:\n\t"
        "j 8f\n\t"
        "ori $2,$zero,6\n\t"
        "5:\n\t"
        "beq $3,$16,8f\n\t"
        "ori $2,$zero,6\n\t"
        "beq $3,$2,8f\n\t"
        "ori $2,$zero,8\n\t"
        "bne $3,$2,7f\n\t"
        "nop\n\t"
        "ori $2,$zero,11\n\t"
        "j 8f\n\t"
        "nop\n\t"
        "8:\n\t"
        "lui $1,0x8018\n\t"
        "sw $2,0x6458($1)\n\t"
        "7:\n\t"
        "lui $3,0x8018\n\t"
        "lw $3,0x6458($3)\n\t"
        "ori $2,$zero,11\n\t"
        "bne $3,$2,9f\n\t"
        "ori $2,$zero,4\n\t"
        "lui $3,0x801d\n\t"
        "lh $3,0x36f0($3)\n\t"
        "nop\n\t"
        "beq $3,$2,10f\n\t"
        "ori $2,$zero,6\n\t"
        "beq $3,$2,10f\n\t"
        "ori $2,$zero,8\n\t"
        "bne $3,$2,9f\n\t"
        "nop\n\t"
        "10:\n\t"
        "jal func_80082ECC\n\t"
        "nop\n\t"
        "srl $3,$2,31\n\t"
        "addu $3,$2,$3\n\t"
        "sra $4,$3,1\n\t"
        "sll $3,$4,1\n\t"
        "bne $2,$3,12f\n\t"
        "ori $2,$zero,13\n\t"
        "ori $2,$zero,12\n\t"
        "12:\n\t"
        "lui $1,0x8018\n\t"
        "sw $2,0x6458($1)\n\t"
        "9:\n\t"
        ".set at"
        : "=r"(death_type), "=r"(current_death_state), "=r"(death_state_one)
        :
        : "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$9", "$10", "$31", "hi", "lo", "memory"
    );
    __asm__ volatile("" : : "r"(death_type), "r"(current_death_state), "r"(death_state_one));

    __asm__ volatile(
        ".set noat\n\t"
        "lui $2,0x8018\n\t"
        "lw $2,0x6458($2)\n\t"
        "nop\n\t"
        "sll $2,$2,2\n\t"
        "lui $1,0x800A\n\t"
        "addu $1,$1,$2\n\t"
        "lw $5,-0x744($1)\n\t"
        "jal func_80082E8C\n\t"
        "addiu $4,$sp,0x20\n\t"
        ".set at"
        :
        :
        : "$1", "$2", "$4", "$5", "$9", "$10", "$31", "memory"
    );
    __asm__ volatile(
        ".set noat\n\t"
        "lui $16,0x8018\n\t"
        "addiu $16,$16,0x64bd\n\t"
        "jal func_8002DD24\n\t"
        "move $4,$16\n\t"
        ".set at"
        :
        :
        : "$4", "$16", "$31", "memory"
    );
    *(volatile uint32_t *)((volatile uint8_t *)stack_storage + 0x80) = 0x200;
    *(volatile uint32_t *)((volatile uint8_t *)stack_storage + 0x84) = 0xF0;
    *(volatile uint32_t *)((volatile uint8_t *)stack_storage + 0x90) = 0xFFFFFFFFu;
    *(volatile uint32_t *)((volatile uint8_t *)stack_storage + 0x88) = 0;
    *(volatile uint32_t *)((volatile uint8_t *)stack_storage + 0x8C) = 0;
    ((volatile uint8_t *)stack_storage)[0x96] = 0;
    ((volatile uint8_t *)stack_storage)[0x95] = 0;
    ((volatile uint8_t *)stack_storage)[0x94] = 0;
    ((volatile uint8_t *)stack_storage)[0x98] = 0x28;
    *(volatile uint32_t *)((volatile uint8_t *)stack_storage + 0x9C) = 0;
    *(volatile uint32_t *)((volatile uint8_t *)stack_storage + 0xA0) = 0;
    ((volatile uint8_t *)stack_storage)[0xA5] = 1;
    ((volatile uint8_t *)stack_storage)[0xA6] = *(volatile uint8_t *)0x801864BDu != 0;
    {
        volatile const uint8_t *config = *(volatile const uint8_t * volatile *)0x800BE698u;
        const int32_t value = config[6] * 12;
        func_8005F2C0(value, value);
    }
    func_8001B558(4, 5, 0xFFFF, 0xFFFF, (int32_t)stack_storage, 0, 0, 0);
    func_8001C700(4, 3);

    *(volatile uint32_t *)(actor_bytes + 0x10) = 0;
    *(volatile uint8_t *)0x80186463u =
        *((volatile const uint8_t *)death_info + 0x0C) != 0;
    *(volatile uint8_t *)0x80186460u = 0;
    *(volatile uint32_t *)0x801864C0u = 0;
    *(volatile uint32_t *)(actor_bytes + 0x04) = 0;
    *(volatile uint8_t *)0x801864B8u = 0;
    *(volatile uint8_t *)0x80186461u = 0;
    *(volatile uint16_t *)0x80186452u = 0;
    *(volatile uint8_t *)0x8018649Du = 0;
    *(volatile uint8_t *)0x80186479u = 0;

    func_80037214(0, (void *)actor_bytes);
    func_8003539C((void *)actor_bytes, 1);
    func_800289E8(0, (void *)((uint8_t *)stack_storage + 0xC0));
    func_800549E8(context_value, (void *)((uint8_t *)stack_storage + 0xC0));
    func_80054C7C(context_value, 0, (void *)0x80186468u);
    func_8001B9C8();
    func_800516F8((void *)((uint8_t *)stack_storage + 0x110));
    func_80054AF0(context_value, (void *)((uint8_t *)stack_storage + 0x100));
    ((uint32_t *)((uint8_t *)stack_storage + 0x100))[0] =
        (uint32_t)-(int32_t)((uint32_t *)((uint8_t *)stack_storage + 0x100))[0];
    ((uint32_t *)((uint8_t *)stack_storage + 0x100))[1] =
        (uint32_t)-(int32_t)((uint32_t *)((uint8_t *)stack_storage + 0x100))[1];
    ((uint32_t *)((uint8_t *)stack_storage + 0x100))[2] =
        (uint32_t)-(int32_t)((uint32_t *)((uint8_t *)stack_storage + 0x100))[2];
    func_8002E954((void *)0x80186468u, *(void * volatile *)0x800BDEE8u);
    func_8002E858(
        (void *)((uint8_t *)stack_storage + 0x100),
        0x240,
        (void *)(*(volatile uint32_t *)0x8018646Cu + 0x780u),
        (void *)((uint8_t *)stack_storage + 0xB0)
    );
    func_80054C7C(
        (void *)(uintptr_t)((uint32_t *)((uint8_t *)stack_storage + 0x110))[0],
        0,
        (void *)((uint8_t *)stack_storage + 0xB0)
    );
    func_800550A4(
        (void *)(uintptr_t)((uint32_t *)((uint8_t *)stack_storage + 0x110))[0],
        context_value
    );
    func_80045094();
    func_80044268();
    func_80069758();
    func_8003AA30();

    *(volatile uint8_t *)0x80186462u = 1;
    actor_component = *(volatile uint8_t **)(actor_bytes + 0x1C);
    actor_bytes[0x2C] = 0;
    BUBSY_GLOBAL_COUNTER = -2;
    *(volatile uint32_t *)(actor_component + 0x20) = 0;
    *(volatile uint32_t *)(nested_state + 0x40) = 0;
    *(volatile uint32_t *)(nested_state + 0x3C) = 0;
    *(volatile uint32_t *)(nested_state + 0x38) = 0;
    *(volatile uint32_t *)(nested_state + 0x80) = 0;
    *(volatile uint32_t *)(nested_state + 0x7C) = 0;
    *(volatile uint32_t *)(nested_state + 0x78) = 0;
    *(volatile uint32_t *)(nested_state + 0xA0) = 0;
    *(volatile uint32_t *)(nested_state + 0x9C) = 0;
    *(volatile uint32_t *)(nested_state + 0x98) = 0;
    *(volatile uint32_t *)(nested_state + 0x30) = 0;
    *(volatile uint32_t *)(nested_state + 0x34) = 0;
    *(volatile uint8_t *)0x80186464u = 0;
    func_8001A670();
}

#undef BUBSY_GLOBAL_COUNTER
#undef BUBSY_DEATH_READ_COUNTER
#undef BUBSY_DEATH_REMAINDER_3
#undef BUBSY_DEATH_REMAINDER_2
#endif
