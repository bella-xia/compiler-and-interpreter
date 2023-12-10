/* variable 'points' allocated 24 bytes of storage at offset 0 */
/* variable 'i' allocated vreg 10 */
/* variable 'count' allocated vreg 11 */
/* Function 'main' uses 24 bytes of memory and 12 virtual registers */
/* allocate machine register %rbx to variable 'i' (vr10) */
/* allocate machine register %r12 to variable 'count' (vr11) */
/* Function 'main' uses 24 total bytes of memory storage for stack allocation */
/* Function 'main': placing memory storage at offset -24 from %rbp */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -24 from %rbp */
/* Function 'main': 32 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbx                /* enter    $24 */
	pushq    %r12
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $32, %rsp
	movl     $0, %ebx            /* mov_l    vr10<%rbx>, $0 */
	movl     $11, %r12d          /* mov_l    vr11<%r12>, $11 */
	jmp      .L1                 /* jmp      .L1 */
.L0:
	leaq     -24(%rbp), %r8      /* localaddr vr13<%r8>, $0 */
	movl     %ebx, %r10d         /* sconv_lq vr14<%r9>, vr10<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr15<%rcx>, vr14<%r9>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr13<%r8>, vr15<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     %r12d, (%r9)        /* mov_l    (vr16)<%r9>, vr11<%r12> */
	movl     %r12d, %r10d        /* add_l    vr14<%r9>, vr11<%r12>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r9d
	movl     %r9d, %r12d         /* mov_l    vr11<%r12>, vr14<%r9> */
	movl     %ebx, %r10d         /* sconv_lq vr14<%r9>, vr10<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr15<%rcx>, vr14<%r9>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr13<%r8>, vr15<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* add_q    vr18<%r8>, vr16<%r9>, $4 */
	addq     $4, %r10
	movq     %r10, %r8
	movl     %r12d, (%r8)        /* mov_l    (vr18)<%r8>, vr11<%r12> */
	movl     %r12d, %r10d        /* add_l    vr14<%r8>, vr11<%r12>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %r12d         /* mov_l    vr11<%r12>, vr14<%r8> */
	movl     %ebx, %r10d         /* add_l    vr14<%r8>, vr10<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr10<%rbx>, vr14<%r8> */
.L1:
	movl     %ebx, %r10d         /* cmplt_l  vr16<%r8>, vr10<%rbx>, $3 */
	cmpl     $3, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr16<%r8>, .L0 */
	jne      .L0
	movl     $2, %ebx            /* mov_l    vr10<%rbx>, $2 */
	jmp      .L3                 /* jmp      .L3 */
.L2:
	leaq     -24(%rbp), %r8      /* localaddr vr13<%r8>, $0 */
	movl     %ebx, %r10d         /* sconv_lq vr14<%r9>, vr10<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr15<%rcx>, vr14<%r9>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr13<%r8>, vr15<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     (%r9), %edi         /* mov_l    vr1, (vr16)<%r9> */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	leaq     -24(%rbp), %r8      /* localaddr vr13<%r8>, $0 */
	movl     %ebx, %r10d         /* sconv_lq vr14<%r9>, vr10<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr15<%rcx>, vr14<%r9>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr13<%r8>, vr15<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* add_q    vr18<%r8>, vr16<%r9>, $4 */
	addq     $4, %r10
	movq     %r10, %r8
	movl     (%r8), %edi         /* mov_l    vr1, (vr18)<%r8> */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movl     %ebx, %r10d         /* sub_l    vr14<%r8>, vr10<%rbx>, $1 */
	subl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr10<%rbx>, vr14<%r8> */
.L3:
	movl     %ebx, %r10d         /* cmpgte_l vr16<%r8>, vr10<%rbx>, $0 */
	cmpl     $0, %r10d
	setge    %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr16<%r8>, .L2 */
	jne      .L2
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $32, %rsp           /* leave    $24 */
	popq     %rbp
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
