/* variable 'p' allocated 8 bytes of storage at offset 0 */
/* Function 'main' uses 8 bytes of memory and 10 virtual registers */
/* Function 'main' uses 8 total bytes of memory storage for stack allocation */
/* Function 'main': placing memory storage at offset -8 from %rbp */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -8 from %rbp */
/* Function 'main': 16 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbp                /* enter    $8 */
	movq     %rsp, %rbp
	subq     $16, %rsp
	leaq     -8(%rbp), %r8       /* localaddr vr11<%r8>, $0 */
	movl     $2, (%r8)           /* mov_l    (vr11)<%r8>, $2 */
	movq     %r8, %r10           /* add_q    vr13<%r9>, vr11<%r8>, $4 */
	addq     $4, %r10
	movq     %r10, %r9
	movl     $3, (%r9)           /* mov_l    (vr13)<%r9>, $3 */
	movl     (%r8), %ecx         /* mov_l    vr16<%rcx>, (vr11)<%r8> */
	movl     (%r9), %r8d         /* mov_l    vr17<%r8>, (vr13)<%r9> */
	movl     %ecx, %r10d         /* add_l    vr18<%r9>, vr16<%rcx>, vr17<%r8> */
	addl     %r8d, %r10d
	movl     %r10d, %r9d
	movl     %r9d, %eax          /* mov_l    vr0, vr18<%r9> */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $16, %rsp           /* leave    $8 */
	popq     %rbp
	ret                          /* ret       */
