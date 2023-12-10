/* variable 'arr' allocated 4 bytes of storage at offset 0 */
/* variable 'sum' allocated vreg 10 */
/* Function 'main' uses 4 bytes of memory and 11 virtual registers */
/* allocate machine register %rbx to variable 'sum' (vr10) */
/* Function 'main' uses 8 total bytes of memory storage for stack allocation */
/* Function 'main': placing memory storage at offset -8 from %rbp */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -8 from %rbp */
/* Function 'main': 8 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbx                /* enter    $4 */
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $8, %rsp
	leaq     -8(%rbp), %r8       /* localaddr vr12<%r8>, $0 */
	movl     $2, (%r8)           /* mov_l    (vr12)<%r8>, $2 */
	movl     (%r8), %r9d         /* mov_l    vr17<%r9>, (vr12)<%r8> */
	movl     %r9d, %r10d         /* add_l    vr18<%r8>, vr17<%r9>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %eax          /* mov_l    vr0, vr18<%r8> */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $8, %rsp            /* leave    $4 */
	popq     %rbp
	popq     %rbx
	ret                          /* ret       */
