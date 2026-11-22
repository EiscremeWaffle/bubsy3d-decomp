
.file	1 "/tmp/bubsy-gcc.TbtH0GgaJF/func_800289E8.c"




gcc2_compiled.:
__gnu_compiled_c:
.text
.align	2
.globl	func_800289E8

.extern	g_level_default_rotation, 32
.extern	g_level_records, 4
.extern	g_level_record_count, 4

.text
.ent	func_800289E8
.set	noreorder
func_800289E8:
.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
.mask	0x00000000,0
.fmask	0x00000000,0
addu	$10,$4,$zero
addu	$11,$5,$zero
lw	$2,g_level_record_count
nop # DEBUG: Reuse of '$2'. 'slt	$2,$10,$2' does not use $at
slt	$2,$10,$2
beq	$2,$0,$L3
addu	$6,$0,$zero

bgez	$10,$L2
sll	$2,$10,3

$L3:
j	$L14
addu	$2,$0,$zero

$L2:
addu	$2,$2,$10
lw	$3,g_level_records
#nop # DEBUG: 'sll	$4,$2,2' does not load from $3
sll	$4,$2,2
addu	$3,$4,$3
lw	$2,8($3)
nop # DEBUG: Reuse of '$2'. 'bne	$2,$0,$L4' does not use $at
bne	$2,$0,$L4
addu	$8,$0,$zero

la	$5,g_level_default_rotation
lw	$2,0($5)
#nop # DEBUG: 'lw	$3,4($5)' does not load from $2
lw	$3,4($5)
#nop # DEBUG: 'lw	$4,8($5)' does not load from $3
lw	$4,8($5)
#nop # DEBUG: 'sw	$2,0($11)' does not load from $4
sw	$2,0($11)
sw	$3,4($11)
sw	$4,8($11)
lw	$2,12($5)
#nop # DEBUG: 'lw	$3,16($5)' does not load from $2
lw	$3,16($5)
#nop # DEBUG: 'lw	$4,20($5)' does not load from $3
lw	$4,20($5)
#nop # DEBUG: 'sw	$2,12($11)' does not load from $4
sw	$2,12($11)
sw	$3,16($11)
sw	$4,20($11)
lw	$2,24($5)
#nop # DEBUG: 'lw	$3,28($5)' does not load from $2
lw	$3,28($5)
#nop # DEBUG: 'sw	$2,24($11)' does not load from $3
sw	$2,24($11)
sw	$3,28($11)
j	$L15
sll	$3,$10,3

$L4:
addu	$7,$11,$zero
addu	$9,$4,$zero
$L6:
addu	$5,$0,$zero
addu	$4,$7,$zero
$L9:
lw	$2,g_level_records
#nop # DEBUG: 'sll	$3,$6,1' does not load from $2
sll	$3,$6,1
addu	$2,$9,$2
lw	$2,8($2)
#nop # DEBUG: 'addu	$6,$6,1' does not load from $2
addu	$6,$6,1
addu	$3,$3,$2
lhu	$2,0($3)
#nop # DEBUG: 'addu	$5,$5,1' does not load from $2
addu	$5,$5,1
sh	$2,0($4)
slt	$2,$5,3
bne	$2,$0,$L9
addu	$4,$4,2

addu	$8,$8,1
slt	$2,$8,3
bne	$2,$0,$L6
addu	$7,$7,6

sll	$3,$10,3
$L15:
addu	$3,$3,$10
lw	$2,g_level_records
#nop # DEBUG: 'sll	$3,$3,2' does not load from $2
sll	$3,$3,2
addu	$2,$3,$2
lw	$2,12($2)
nop # DEBUG: Reuse of '$2'. 'lw	$2,0($2)' does not use $at
lw	$2,0($2)
nop # DEBUG: Reuse of '$2'. 'sw	$2,20($11)' does not use $at
sw	$2,20($11)
lw	$2,g_level_records
nop # DEBUG: Reuse of '$2'. 'addu	$2,$3,$2' does not use $at
addu	$2,$3,$2
lw	$2,12($2)
nop # DEBUG: Reuse of '$2'. 'lw	$2,4($2)' does not use $at
lw	$2,4($2)
nop # DEBUG: Reuse of '$2'. 'sw	$2,24($11)' does not use $at
sw	$2,24($11)
lw	$2,g_level_records
nop # DEBUG: Reuse of '$2'. 'addu	$3,$3,$2' does not use $at
addu	$3,$3,$2
lw	$2,12($3)
nop # DEBUG: Reuse of '$2'. 'lw	$3,8($2)' does not use $at
lw	$3,8($2)
#nop # DEBUG: 'li	$2,0x00000001		# 1' does not load from $3
ori	$2,$zero,1
sw	$3,28($11)
$L14:
j	$31
nop  # DEBUG: branch/jump
.end	func_800289E8
