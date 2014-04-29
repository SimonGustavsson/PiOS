;@
;@ Various ASM snippets that enables you to do some things C doesn't
;@

;@ 
;@ Gets the Frame Pointer (FP) register
;@ C Signature: int* get_frame_pointer(void)
;@
.align 2
.globl get_fp
get_fp:
    mov r0, fp
    mov pc, lr

;@
;@ Branch to the given memory address
;@ C Signature: void branchTo(unsigned int* addr)
;@              addr: The address to branch to
;@
.globl branchTo
branchTo:
    push {fp, lr}
    add	fp, sp, #4

    mov r2, r0
    blx r2
    pop {fp, lr}
    
    bx lr
