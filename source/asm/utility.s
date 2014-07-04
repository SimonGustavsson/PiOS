;@
;@ Various ASM snippets that enables you to do some things C doesn't
;@

;@ This function isn't actually used, it's only used to
;@ Show how prologue/epilogue works
.globl fp_showoff
fp_showoff:
    
    ;@ Save FP and LR from previous function
    push	{fp, lr}

    ;@ Save the location of the values we just pushed
    add	fp, sp, #4

    ;@ Because we only store 2 values in this frame, increase the stack
    ;@ By two more words to make sure we have a clean, readable stack frame
    sub	sp, sp, #8

    ;@
    ;@ SP is now usuable, function body here...
    ;@
    
    ;@ Restore the stack to the location where we stored FP and LR in the proloque 
    sub	sp, fp, #4

    ;@ Pop the old frame pointer off the stack, and return to the caller
    ;@ By restoring LR into PC
    pop	{fp, pc}

;@ 
;@ Gets the Frame Pointer (FP) register
;@ C Signature: int* get_frame_pointer(void)
;@
.type get_fp %function
.align 2
.globl get_fp
get_fp:
    mov r0, fp
    mov pc, lr
    
;@
;@ Gets the current address of the Stack Pointer
;@ C Signature: unsigned int get_sp(void)
;@
.type get_sp %function
.globl get_sp
get_sp:
    mov r0, sp

    bx lr

;@ 
;@ Branch to the give function (WARNING: this trashes FP and does not set up LR!)
;@ C Signature: void branch(unsigned int* addr)
;@              addr: The address to branch to 
;@
.type branch %function
.globl branch
branch:
    bx r0
    
;@
;@ Call the function at the given memory address
;@ C Signature: void call(unsigned int* addr)
;@              addr: The address to branch to
;@
.type call %function
.globl call
call:
    push {fp, lr}
    add	fp, sp, #4

    blx r0
    pop {fp, lr}
    
    bx lr
