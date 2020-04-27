.section .text.UTVMInit, "ax", %progbits
.globl UTVMInit
UTVMInit:
	la	sp, _utvm_stack_pointer_init
	jal	UTVMMain

garbage:
	/* we should not reach here */
	wfi
	j garbage

.size UTVMInit, .-UTVMInit
