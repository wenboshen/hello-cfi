#include <linux/linkage.h>
#include <asm/assembler.h>

good_msg:
	.asciz "Jump to function entry, but wrong sig. CFI should protect transfer to here\n"

bad_msg:
	.asciz "Jump to function middle. CFI should protect transfer to here\n"

ENTRY(not_entry_point)
    	stp	x29, x30, [sp,#-16]!
    	mov	x29, sp
	ldr	x0, =good_msg
   	bl	printk
	ldp	x29, x30, [sp],#16
	ret
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
    	stp	x29, x30, [sp,#-16]!
    	mov	x29, sp
	ldr	x0, =bad_msg
   	bl	printk
	ldp	x29, x30, [sp],#16
	ret
ENDPROC(not_entry_point)

.section .text
