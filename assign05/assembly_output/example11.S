/* variable 'a' allocated 4 bytes of storage at offset 0 */
/* variable 'p' allocated 8 bytes of storage at offset 8 */
/* variable 'pp' allocated vreg 10 */
/* Function 'main' uses 16 bytes of memory and 11 virtual registers */
/* allocate machine register %rbx to variable 'pp' (vr10) */
/* Function 'main' uses 16 total bytes of memory storage for stack allocation */
/* Function 'main': placing memory storage at offset -16 from %rbp */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -16 from %rbp */
/* Function 'main': 24 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbx                /* enter    $16 */
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $24, %rsp
	leaq     -16(%rbp), %r8      /* localaddr vr11<%r8>, $0 */
	leaq     -8(%rbp), %r9       /* localaddr vr12<%r9>, $8 */
	movq     %r8, (%r9)          /* mov_q    (vr12)<%r9>, vr11<%r8> */
	movq     (%r9), %r9          /* mov_q    vr12<%r9>, (vr12)<%r9> */
	movl     $17, (%r9)          /* mov_l    (vr12)<%r9>, $17 */
	movl     (%r8), %eax         /* mov_l    vr0, (vr11)<%r8> */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $24, %rsp           /* leave    $16 */
	popq     %rbp
	popq     %rbx
	ret                          /* ret       */
