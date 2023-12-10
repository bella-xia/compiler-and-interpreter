/* variable 'x' allocated vreg 10 */
/* variable 'y' allocated vreg 11 */
/* Function 'main' uses 0 bytes of memory and 12 virtual registers */
/* allocate machine register %rbx to variable 'x' (vr10) */
/* allocate machine register %r12 to variable 'y' (vr11) */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset 0 from %rbp */
/* Function 'main': 0 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbx                /* enter    $0 */
	pushq    %r12
	pushq    %rbp
	movq     %rsp, %rbp
	call     read_i32            /* call     read_i32 */
	movl     %eax, %ebx          /* mov_l    vr10<%rbx>, vr0 */
	call     read_i32            /* call     read_i32 */
	movl     %ebx, %r10d         /* add_l    vr12<%r8>, vr10<%rbx>, vr0 */
	addl     %eax, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %eax          /* mov_l    vr0, vr12<%r8> */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	popq     %rbp                /* leave    $0 */
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
