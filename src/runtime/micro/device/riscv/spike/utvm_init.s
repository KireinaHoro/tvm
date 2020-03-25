.section .text.UTVMInit, "ax", %progbits
.globl UTVMInit
UTVMInit:
	la	s0, _trap
	csrw	mtvec, s0
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
	/* read out CSRs and hang
	 * somehow gdb cannot correctly read out CSRs
	 */
	csrr	a0, mcause
	csrr	a1, mepc
	csrr	a2, mtval
	csrr	a3, mtvec
	wfi
	j	_trap
