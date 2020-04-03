.section .text.UTVMInit, "ax", %progbits
.globl UTVMInit
UTVMInit:
	la	s0, _trap
	csrw	mtvec, s0
	/* enable XS in mstatus */
	li	s0, 0x8000
	csrs	mstatus, s0
	la	sp, _utvm_stack_pointer_init
	jal	UTVMMain

garbage:
	/* we should not reach here */
	wfi
	j garbage

.size UTVMInit, .-UTVMInit

.align 3
.section .text.UTVMInit, "ax", %progbits
.globl _trap
_trap:
	wfi
	j	_trap
