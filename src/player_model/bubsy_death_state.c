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
        if (level_id == 5 || level_id == 7 || level_id == 9 || level_id == 18) {
            death_state = 6;
        } else if (level_id == 4 || level_id == 6 || level_id == 8) {
            death_state = 11;
        }
    }

    if (death_state != 11 && (level_id == 4 || level_id == 6 || level_id == 8)) {
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
    register int32_t current_death_state __asm__("$16");
    register int32_t death_state_one __asm__("$19");
    uint32_t stack_storage[0x128 / sizeof(uint32_t)];

    death_info = func_80026648((void *)actor_bytes, context);
        __asm__ volatile("" : "=r"(death_info) : "0"(death_info));
    {
        __asm__ volatile(
            ".set noreorder\n\t"
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
            "1:\n\t"
            ".set reorder"
            :
            :
            : "$2", "$3", "$4", "$5", "$6", "$7", "$31", "memory"
        );
    }

continue_main_path:
    __asm__ volatile(
        ".set noreorder\n\t"
        "lui $3,0x801d\n\t"
        "lh $3,0x36f0($3)\n\t"
        "ori $2,$zero,0x13\n\t"
        "bne $3,$2,1f\n\t"
        "nop\n\t"
        "jal func_8004733C\n\t"
        "addu $4,$zero,$zero\n\t"
        "j 0x80037ad0\n\t"
        "nop\n\t"
        "1:\n\t"
        ".set reorder"
        :
        :
        : "$2", "$3", "$4", "$31", "memory"
    );
    {
        __asm__ volatile(
            ".set noreorder\n\t"
            "lui $2,0x8018\n\t"
            "lbu $2,0x6462($2)\n\t"
            "nop\n\t"
            "bnez $2,31f\n\t"
            "nop\n\t"
            ".set reorder"
            :
            :
            : "$2", "memory"
        );
    }
    {
        __asm__ volatile(
            ".set noat\n\t"
            ".set noreorder\n\t"
            "lw $2,0x3a4($gp)\n\t"
            "nop\n\t"
            "addiu $2,$2,2\n\t"
            "sltiu $2,$2,2\n\t"
            "beqz $2,21f\n\t"
            "addiu $2,$zero,-0x32\n\t"
            "lui $3,0x8018\n\t"
            "lw $3,0x645c($3)\n\t"
            "sw $2,0x118($sp)\n\t"
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
            "ori $4,$zero,0x1b\n\t"
            "sw $0,0x10($sp)\n\t"
            "sw $0,0x14($sp)\n\t"
            "sw $0,0x18($sp)\n\t"
            "sw $0,0x1C($sp)\n\t"
            "lw $6,0x0c($20)\n\t"
            "lui $7,0x8018\n\t"
            "lw $7,0x645c($7)\n\t"
            "jal func_8001B558\n\t"
            "ori $5,$zero,5\n\t"
            "li $2,-1\n\t"
            "2:\n\t"
            "lui $1,0x8018\n\t"
            "sw $2,0x645c($1)\n\t"
            "1:\n\t"
            "jal func_8003CB14\n\t"
            "nop\n\t"
            "addu $4,$zero,$zero\n\t"
            "jal func_80039158\n\t"
            "addu $5,$zero,$zero\n\t"
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
            "ori $4,$zero,5\n\t"
            "addu $4,$zero,$zero\n\t"
            "lui $6,0x8003\n\t"
            "addiu $6,$6,0x7350\n\t"
            "jal func_8005EF9C\n\t"
            "ori $5,$zero,0x3c\n\t"
            "lbu $3,0x7ec($gp)\n\t"
            "ori $2,$zero,1\n\t"
            "bne $3,$2,4f\n\t"
            "ori $4,$zero,5\n\t"
            "lw $2,0x10($20)\n\t"
            "lui $3,0x10\n\t"
            "and $2,$2,$3\n\t"
            "beqz $2,15f\n\t"
            "addu $5,$zero,$zero\n\t"
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
            "ori $5,$zero,1\n\t"
            "sw $0,0x364($gp)\n\t"
            "ori $4,$zero,5\n\t"
            "15:\n\t"
            "jal func_8001C700\n\t"
            "ori $5,$zero,3\n\t"
            "ori $4,$zero,5\n\t"
            "jal func_8001C700\n\t"
            "ori $5,$zero,5\n\t"
            "ori $4,$zero,0x0f\n\t"
            "jal func_8002CA88\n\t"
            "ori $5,$zero,0xb4\n\t"
            "jal func_8002BAD4\n\t"
            "ori $4,$zero,0x0f\n\t"
            "j 5f\n\t"
            "sb $0,0($20)\n\t"
            "4:\n\t"
            "jal func_8001C700\n\t"
            "ori $5,$zero,5\n\t"
            "ori $4,$zero,5\n\t"
            "jal func_8001C700\n\t"
            "ori $5,$zero,8\n\t"
            "5:\n\t"
            "jal func_8001BC08\n\t"
            "nop\n\t"
            "jal func_8001B9C8\n\t"
            "nop\n\t"
            "ori $4,$zero,0x2c\n\t"
            "addu $5,$zero,$zero\n\t"
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
            "ori $4,$zero,0x1b\n\t"
            "3:\n\t"
            "jal func_800222E0\n\t"
            "nop\n\t"
            "andi $2,$2,0x00ff\n\t"
            "ori $3,$zero,1\n\t"
            "bne $2,$3,6f\n\t"
            "ori $4,$zero,0x1b\n\t"
            "sw $0,0x3a4($gp)\n\t"
            "6:\n\t"
            "sw $0,0x10($sp)\n\t"
            "sw $0,0x14($sp)\n\t"
            "sw $0,0x18($sp)\n\t"
            "sw $0,0x1c($sp)\n\t"
            "lw $6,0x0c($20)\n\t"
            "addu $5,$zero,$zero\n\t"
            "jal func_8001B558\n\t"
            "addu $7,$6,$zero\n\t"
            "j 0x80037ad0\n\t"
            "nop\n\t"
            "21:\n\t"
            ".set reorder\n\t"
            ".set at"
            :
            : "r"(actor_bytes)
            : "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "hi", "lo", "memory"
        );
    }
    __asm__ volatile(
        ".set noreorder\n\t"
        "jal func_80028FC8\n\t"
        "nop\n\t"
        ".set reorder"
        :
        :
        : "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "hi", "lo", "memory"
    );
    __asm__ volatile(
        ".set noat\n\t"
        ".set noreorder\n\t"
        "lui $2,0x8018\n\t"
        "lw $2,0x6458($2)\n\t"
        "nop\n\t"
        "bgez $2,1f\n\t"
        "ori $19,$zero,1\n\t"
        "addu $4,$zero,$zero\n\t"
        "lui $5,0x8001\n\t"
        "addiu $5,$5,0x2a20\n\t"
        "lui $6,0x8001\n\t"
        "addiu $6,$6,0x298c\n\t"
        "jal func_800556D0\n\t"
        "ori $7,$zero,0x912\n\t"
        "lui $2,0x8018\n\t"
        "lw $2,0x6458($2)\n\t"
        "nop\n\t"
        "bgez $2,1f\n\t"
        "ori $2,$zero,1\n\t"
        "lui $1,0x8018\n\t"
        "sw $2,0x6458($1)\n\t"
        "1:\n\t"
        "lui $17,0x8018\n\t"
        "addiu $17,$17,0x6458\n\t"
        "lw $16,0($17)\n\t"
        "nop\n\t"
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
        "beqz $4,13f\n\t"
        "nop\n\t"
        "bne $4,$16,11f\n\t"
        "ori $2,$zero,2\n\t"
        "ori $2,$zero,7\n\t"
        "j 7f\n\t"
        "sw $2,0($17)\n\t"
        "11:\n\t"
        "bne $4,$2,7f\n\t"
        "ori $2,$zero,9\n\t"
        "j 7f\n\t"
        "sw $2,0($17)\n\t"
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
        "13:\n\t"
        "j 7f\n\t"
        "sw $16,0($17)\n\t"
        "4:\n\t"
        "bne $4,$19,7f\n\t"
        "ori $2,$zero,10\n\t"
        "j 7f\n\t"
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
        "beq $3,$16,14f\n\t"
        "ori $2,$zero,6\n\t"
        "beq $3,$2,14f\n\t"
        "ori $2,$zero,8\n\t"
        "bne $3,$2,7f\n\t"
        "nop\n\t"
        "14:\n\t"
        "ori $2,$zero,11\n\t"
        "8:\n\t"
        "lui $1,0x8018\n\t"
        "sw $2,0x6458($1)\n\t"
        "7:\n\t"
        "lui $3,0x8018\n\t"
        "lw $3,0x6458($3)\n\t"
        "ori $2,$zero,11\n\t"
        "beq $3,$2,9f\n\t"
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
        ".set reorder\n\t"
        ".set at"
        : "=r"(death_type), "=r"(current_death_state), "=r"(death_state_one)
        :
        : "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "hi", "lo", "memory"
    );
    __asm__ volatile("" : : "r"(death_type), "r"(current_death_state), "r"(death_state_one));

    __asm__ volatile(
        ".set noat\n\t"
        ".set noreorder\n\t"
        "lui $2,0x8018\n\t"
        "lw $2,0x6458($2)\n\t"
        "nop\n\t"
        "sll $2,$2,2\n\t"
        "lui $1,0x800A\n\t"
        "addu $1,$1,$2\n\t"
        "lw $5,-0x744($1)\n\t"
        "jal func_80082E8C\n\t"
        "addiu $4,$sp,0x20\n\t"
        ".set reorder\n\t"
        ".set at"
        :
        :
        : "$1", "$2", "$4", "$5", "$9", "$10", "$31", "memory"
    );
    __asm__ volatile(
        ".set noat\n\t"
        ".set noreorder\n\t"
        "lui $16,0x8018\n\t"
        "addiu $16,$16,0x64bd\n\t"
        "jal func_8002DD24\n\t"
        "addu $4,$16,$zero\n\t"
        "ori $2,$zero,0x200\n\t"
        "sw $2,0xA0($sp)\n\t"
        "ori $2,$zero,0xF0\n\t"
        "sw $2,0xA4($sp)\n\t"
        "addiu $2,$zero,-1\n\t"
        "sw $2,0xB0($sp)\n\t"
        "ori $2,$zero,0x28\n\t"
        "sw $zero,0xA8($sp)\n\t"
        "sw $zero,0xAC($sp)\n\t"
        "sb $zero,0xB6($sp)\n\t"
        "sb $zero,0xB5($sp)\n\t"
        "sb $zero,0xB4($sp)\n\t"
        "sb $2,0xB8($sp)\n\t"
        "sw $zero,0xBC($sp)\n\t"
        "sw $zero,0xC0($sp)\n\t"
        "lbu $2,0($16)\n\t"
        "ori $3,$zero,1\n\t"
        "beqz $2,1f\n\t"
        "sb $3,0xC5($sp)\n\t"
        "j 2f\n\t"
        "sb $3,0xC6($sp)\n\t"
        "1:\n\t"
        "sb $zero,0xC6($sp)\n\t"
        "2:\n\t"
        "lui $2,0x800C\n\t"
        "lw $2,-0x1968($2)\n\t"
        "nop\n\t"
        "lbu $2,6($2)\n\t"
        "nop\n\t"
        "sll $4,$2,1\n\t"
        "addu $4,$4,$2\n\t"
        "sll $4,$4,2\n\t"
        "jal func_8005F2C0\n\t"
        "addu $5,$4,$zero\n\t"
        ".set reorder\n\t"
        ".set at"
        :
        :
        : "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$16", "$24", "$25", "$31", "hi", "lo", "memory"
    );
    __asm__ volatile(
        ".set noreorder\n\t"
        "ori $4,$zero,4\n\t"
        "ori $5,$zero,5\n\t"
        "ori $6,$zero,0xFFFF\n\t"
        "ori $7,$zero,0xFFFF\n\t"
        "addiu $2,$sp,0x20\n\t"
        "sw $2,0x10($sp)\n\t"
        "sw $zero,0x14($sp)\n\t"
        "sw $zero,0x18($sp)\n\t"
        "jal func_8001B558\n\t"
        "sw $zero,0x1C($sp)\n\t"
        "ori $4,$zero,4\n\t"
        "jal func_8001C700\n\t"
        "ori $5,$zero,3\n\t"
        ".set reorder"
        : "=m"(stack_storage)
        :
        : "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "hi", "lo", "memory"
    );

    __asm__ volatile(
        ".set noat\n\t"
        ".set noreorder\n\t"
        "sw $zero,0x10($20)\n\t"
        "lbu $2,0x0C($22)\n\t"
        "nop\n\t"
        "beqz $2,1f\n\t"
        "ori $2,$zero,1\n\t"
        "lui $1,0x8018\n\t"
        "sb $2,0x6463($1)\n\t"
        "j 2f\n\t"
        "addu $4,$zero,$zero\n\t"
        "1:\n\t"
        "lui $1,0x8018\n\t"
        "sb $zero,0x6463($1)\n\t"
        "addu $4,$zero,$zero\n\t"
        "2:\n\t"
        "lui $17,0x8018\n\t"
        "addiu $17,$17,0x6460\n\t"
        "sb $zero,0($17)\n\t"
        "lui $1,0x8018\n\t"
        "sw $zero,0x64C0($1)\n\t"
        "sw $zero,4($20)\n\t"
        "lui $1,0x8018\n\t"
        "sb $zero,0x64B8($1)\n\t"
        "lui $1,0x8018\n\t"
        "sb $zero,0x6461($1)\n\t"
        "lui $1,0x8018\n\t"
        "sh $zero,0x6452($1)\n\t"
        "lui $1,0x8018\n\t"
        "sb $zero,0x649D($1)\n\t"
        "lui $1,0x8018\n\t"
        "sb $zero,0x6479($1)\n\t"
        "jal func_80037214\n\t"
        "addu $5,$20,$zero\n\t"
        "addu $4,$20,$zero\n\t"
        "jal func_8003539C\n\t"
        "ori $5,$zero,1\n\t"
        "addu $4,$zero,$zero\n\t"
        "addiu $16,$sp,0xE0\n\t"
        "jal func_800289E8\n\t"
        "addu $5,$16,$zero\n\t"
        "addu $4,$21,$zero\n\t"
        "jal func_800549E8\n\t"
        "addu $5,$16,$zero\n\t"
        "addu $4,$21,$zero\n\t"
        "addu $5,$zero,$zero\n\t"
        "addiu $17,$17,8\n\t"
        "jal func_80054C7C\n\t"
        "addu $6,$17,$zero\n\t"
        "jal func_8001B9C8\n\t"
        "nop\n\t"
        "jal func_800516F8\n\t"
        "addiu $4,$sp,0x130\n\t"
        "addu $4,$21,$zero\n\t"
        "addiu $16,$sp,0x120\n\t"
        "jal func_80054AF0\n\t"
        "addu $5,$16,$zero\n\t"
        "addu $4,$17,$zero\n\t"
        "lw $2,0x120($sp)\n\t"
        "lui $5,0x800C\n\t"
        "lw $5,-0x2118($5)\n\t"
        "lw $3,0x128($sp)\n\t"
        "subu $2,$zero,$2\n\t"
        "sw $2,0x120($sp)\n\t"
        "lw $2,0x124($sp)\n\t"
        "subu $3,$zero,$3\n\t"
        "sw $3,0x128($sp)\n\t"
        "subu $2,$zero,$2\n\t"
        "jal func_8002E954\n\t"
        "sw $2,0x124($sp)\n\t"
        "addu $4,$16,$zero\n\t"
        "ori $5,$zero,0x240\n\t"
        "addiu $16,$sp,0xD0\n\t"
        "lui $6,0x8018\n\t"
        "lw $6,0x646C($6)\n\t"
        "addu $7,$16,$zero\n\t"
        "jal func_8002E858\n\t"
        "addiu $6,$6,0x780\n\t"
        "addu $5,$zero,$zero\n\t"
        "lw $4,0x130($sp)\n\t"
        "jal func_80054C7C\n\t"
        "addu $6,$16,$zero\n\t"
        "lw $4,0x130($sp)\n\t"
        "jal func_800550A4\n\t"
        "addu $5,$21,$zero\n\t"
        "jal func_80045094\n\t"
        "nop\n\t"
        "jal func_80044268\n\t"
        "nop\n\t"
        "jal func_80069758\n\t"
        "nop\n\t"
        "jal func_8003AA30\n\t"
        "nop\n\t"
        ".set reorder\n\t"
        ".set at"
        :
        : "r"(actor_bytes), "r"(death_info), "r"(context_value)
        : "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$24", "$25", "$31", "hi", "lo", "memory"
    );

    __asm__ volatile(
        ".set noat\n\t"
        ".set noreorder\n\t"
        "ori $2,$zero,1\n\t"
        "lui $1,0x8018\n\t"
        "sb $2,0x6462($1)\n\t"
        "lw $3,0x1C($20)\n\t"
        "addiu $2,$zero,-2\n\t"
        "sb $zero,0x2C($20)\n\t"
        "sw $2,0x3A4($gp)\n\t"
        "sw $zero,0x20($3)\n\t"
        "sw $zero,0x40($18)\n\t"
        "sw $zero,0x3C($18)\n\t"
        "sw $zero,0x38($18)\n\t"
        "sw $zero,0x80($18)\n\t"
        "sw $zero,0x7C($18)\n\t"
        "sw $zero,0x78($18)\n\t"
        "sw $zero,0xA0($18)\n\t"
        "sw $zero,0x9C($18)\n\t"
        "sw $zero,0x98($18)\n\t"
        "sw $zero,0x30($18)\n\t"
        "sw $zero,0x34($18)\n\t"
        "lui $1,0x8018\n\t"
        "sb $zero,0x6464($1)\n\t"
        "31:\n\t"
        "jal func_8001A670\n\t"
        "nop\n\t"
        ".set reorder\n\t"
        ".set at"
        :
        : "r"(actor_bytes), "r"(nested_state)
        : "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "hi", "lo", "memory"
    );
}

#undef BUBSY_GLOBAL_COUNTER
#undef BUBSY_DEATH_READ_COUNTER
#undef BUBSY_DEATH_REMAINDER_3
#undef BUBSY_DEATH_REMAINDER_2
#endif
