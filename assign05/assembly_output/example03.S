/* variable 'i' allocated vreg 10 */
/* variable 'n' allocated vreg 11 */
/* variable 'sum' allocated vreg 12 */
/* Function 'main' uses 0 bytes of memory and 13 virtual registers */
/* allocate machine register %rbx to variable 'i' (vr10) */
/* allocate machine register %r12 to variable 'sum' (vr12) */
/* allocate machine register %r13 to variable 'n' (vr11) */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset 0 from %rbp */
/* Function 'main': 8 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbx                /* enter    $0 */
	pushq    %r12
	pushq    %r13
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $8, %rsp
	movl     $1, %ebx            /* mov_l    vr10<%rbx>, $1 */
	movl     $11, %r13d          /* mov_l    vr11<%r13>, $11 */
	movl     $0, %r12d           /* mov_l    vr12<%r12>, $0 */
.L0:
	movl     %r12d, %r10d        /* add_l    vr13<%r8>, vr12<%r12>, vr10<%rbx> */
	addl     %ebx, %r10d
	movl     %r10d, %r8d
	movl     %ebx, %r10d         /* add_l    vr14<%r9>, vr10<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r9d
	movl     %r8d, %r12d         /* mov_l    vr12<%r12>, vr13<%r8> */
	movl     %r9d, %r10d         /* cmplt_l  vr13<%r8>, vr14<%r9>, vr11<%r13> */
	cmpl     %r13d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	movl     %r9d, %ebx          /* mov_l    vr10<%rbx>, vr14<%r9> */
	cmpl     $0, %r8d            /* cjmp_t   vr13<%r8>, .L0 */
	jne      .L0
	movl     %r12d, %eax         /* mov_l    vr0, vr12<%r12> */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
