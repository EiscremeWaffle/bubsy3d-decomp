	.file	1 "/tmp/bubsy-gcc.TbtH0GgaJF/func_800289E8.c"

 # GNU C 2.6.3 [AL 1.1, MM 40] Sony Playstation compiled by GNU C

 # Cc1 defaults:
 # -mgas -msoft-float

 # Cc1 arguments (-G value = 8, Cpu = 3000, ISA = 1):
 # -G8 -quiet -dumpbase -msoft-float -O2 -fno-pic -fomit-frame-pointer -o

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
func_800289E8:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	move	$10,$4
	move	$11,$5
	lw	$2,g_level_record_count
	#nop
	slt	$2,$10,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L3
	move	$6,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bgez	$10,$L2
	sll	$2,$10,3
	.set	macro
	.set	reorder

$L3:
	.set	noreorder
	.set	nomacro
	j	$L14
	move	$2,$0
	.set	macro
	.set	reorder

$L2:
	addu	$2,$2,$10
	#.set	volatile
	lw	$3,g_level_records
	#.set	novolatile
	sll	$4,$2,2
	addu	$3,$4,$3
	lw	$2,8($3)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L4
	move	$8,$0
	.set	macro
	.set	reorder

	la	$5,g_level_default_rotation
	lw	$2,0($5)
	lw	$3,4($5)
	lw	$4,8($5)
	sw	$2,0($11)
	sw	$3,4($11)
	sw	$4,8($11)
	lw	$2,12($5)
	lw	$3,16($5)
	lw	$4,20($5)
	sw	$2,12($11)
	sw	$3,16($11)
	sw	$4,20($11)
	lw	$2,24($5)
	lw	$3,28($5)
	sw	$2,24($11)
	sw	$3,28($11)
	.set	noreorder
	.set	nomacro
	j	$L15
	sll	$3,$10,3
	.set	macro
	.set	reorder

$L4:
	move	$7,$11
	move	$9,$4
$L6:
	move	$5,$0
	move	$4,$7
$L9:
	#.set	volatile
	lw	$2,g_level_records
	#.set	novolatile
	sll	$3,$6,1
	addu	$2,$9,$2
	lw	$2,8($2)
	addu	$6,$6,1
	addu	$3,$3,$2
	lhu	$2,0($3)
	addu	$5,$5,1
	sh	$2,0($4)
	slt	$2,$5,3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L9
	addu	$4,$4,2
	.set	macro
	.set	reorder

	addu	$8,$8,1
	slt	$2,$8,3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L6
	addu	$7,$7,6
	.set	macro
	.set	reorder

	sll	$3,$10,3
$L15:
	addu	$3,$3,$10
	#.set	volatile
	lw	$2,g_level_records
	#.set	novolatile
	sll	$3,$3,2
	addu	$2,$3,$2
	lw	$2,12($2)
	#nop
	lw	$2,0($2)
	#nop
	#.set	volatile
	sw	$2,20($11)
	#.set	novolatile
	#.set	volatile
	lw	$2,g_level_records
	#.set	novolatile
	#nop
	addu	$2,$3,$2
	lw	$2,12($2)
	#nop
	lw	$2,4($2)
	#nop
	#.set	volatile
	sw	$2,24($11)
	#.set	novolatile
	#.set	volatile
	lw	$2,g_level_records
	#.set	novolatile
	#nop
	addu	$3,$3,$2
	lw	$2,12($3)
	#nop
	lw	$3,8($2)
	li	$2,0x00000001		# 1
	#.set	volatile
	sw	$3,28($11)
	#.set	novolatile
$L14:
	j	$31
	.end	func_800289E8
