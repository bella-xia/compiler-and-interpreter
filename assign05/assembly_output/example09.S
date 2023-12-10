/* variable 'a' allocated vreg 10 */
/* variable 'n' allocated vreg 11 */
/* variable 'i' allocated vreg 12 */
/* variable 'sum' allocated vreg 13 */
/* Function 'sum' uses 0 bytes of memory and 14 virtual registers */
/* allocate machine register %rbx to variable 'i' (vr12) */
/* allocate machine register %r12 to variable 'sum' (vr13) */
/* allocate machine register %r13 to variable 'a' (vr10) */
/* allocate machine register %r14 to variable 'n' (vr11) */
/* variable 'arr' allocated 12 bytes of storage at offset 0 */
/* variable 'result' allocated vreg 10 */
/* Function 'main' uses 12 bytes of memory and 11 virtual registers */
/* allocate machine register %rbx to variable 'result' (vr10) */
/* Function 'sum' uses 0 total bytes of memory storage for vregs */
/* Function 'sum': placing vreg storage at offset 0 from %rbp */
/* Function 'sum': 0 bytes allocated in stack frame */
	.section .text

	.globl sum
sum:
	pushq    %rbx                /* enter    $0 */
	pushq    %r12
	pushq    %r13
	pushq    %r14
	pushq    %rbp
	movq     %rsp, %rbp
	movq     %rdi, %r13          /* mov_q    vr10<%r13>, vr1 */
	movl     %esi, %r14d         /* mov_l    vr11<%r14>, vr2 */
	movl     $0, %ebx            /* mov_l    vr12<%rbx>, $0 */
	movl     $0, %r12d           /* mov_l    vr13<%r12>, $0 */
	jmp      .L1                 /* jmp      .L1 */
.L0:
	movl     %ebx, %r10d         /* sconv_lq vr15<%r8>, vr12<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr16<%r9>, vr15<%r8>, $4 */
	imulq    $4, %r10
	movq     %r10, %r9
	movq     %r13, %r10          /* add_q    vr17<%r8>, vr10<%r13>, vr16<%r9> */
	addq     %r9, %r10
	movq     %r10, %r8
	movl     (%r8), %r9d         /* mov_l    vr18<%r9>, (vr17)<%r8> */
	movl     %r12d, %r10d        /* add_l    vr19<%r8>, vr13<%r12>, vr18<%r9> */
	addl     %r9d, %r10d
	movl     %r10d, %r8d
	movl     %ebx, %r10d         /* add_l    vr16<%r9>, vr12<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r9d
	movl     %r9d, %ebx          /* mov_l    vr12<%rbx>, vr16<%r9> */
	movl     %r8d, %r12d         /* mov_l    vr13<%r12>, vr19<%r8> */
.L1:
	movl     %ebx, %r10d         /* cmplt_l  vr17<%r8>, vr12<%rbx>, vr11<%r14> */
	cmpl     %r14d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr17<%r8>, .L0 */
	jne      .L0
	movl     %r12d, %eax         /* mov_l    vr0, vr13<%r12> */
	jmp      .Lsum_return        /* jmp      .Lsum_return */
.Lsum_return:
	popq     %rbp                /* leave    $0 */
	popq     %r14
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
/* Function 'main' uses 16 total bytes of memory storage for stack allocation */
/* Function 'main': placing memory storage at offset -16 from %rbp */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -16 from %rbp */
/* Function 'main': 24 bytes allocated in stack frame */

	.globl main
main:
	pushq    %rbx                /* enter    $12 */
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $24, %rsp
	leaq     -16(%rbp), %r8      /* localaddr vr12<%r8>, $0 */
	movl     $1, (%r8)           /* mov_l    (vr12)<%r8>, $1 */
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr12<%r8>, $4 */
	addq     $4, %r10
	movq     %r10, %r9
	movl     $2, (%r9)           /* mov_l    (vr16)<%r9>, $2 */
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr12<%r8>, $8 */
	addq     $8, %r10
	movq     %r10, %r9
	movl     $3, (%r9)           /* mov_l    (vr16)<%r9>, $3 */
	movq     %r8, %rdi           /* mov_q    vr1, vr12<%r8> */
	movl     $3, %esi            /* mov_l    vr2, $3 */
	call     sum                 /* call     sum */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $24, %rsp           /* leave    $12 */
	popq     %rbp
	popq     %rbx
	ret                          /* ret       */
