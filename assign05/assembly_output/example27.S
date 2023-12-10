/* variable 'i' allocated vreg 10 */
/* variable 'j' allocated vreg 11 */
/* variable 'n' allocated vreg 12 */
/* variable 'm' allocated 400 bytes of storage at offset 0 */
/* Function 'main' uses 400 bytes of memory and 13 virtual registers */
/* allocate machine register %rbx to variable 'i' (vr10) */
/* allocate machine register %r12 to variable 'j' (vr11) */
/* allocate machine register %r13 to variable 'n' (vr12) */
/* Function 'main' uses 400 total bytes of memory storage for stack allocation */
/* Function 'main': placing memory storage at offset -400 from %rbp */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -400 from %rbp */
/* Function 'main': 408 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbx                /* enter    $400 */
	pushq    %r12
	pushq    %r13
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $408, %rsp
	movl     $0, %ebx            /* mov_l    vr10<%rbx>, $0 */
	jmp      .L1                 /* jmp      .L1 */
.L0:
	movl     $0, %r12d           /* mov_l    vr11<%r12>, $0 */
	movl     $0, %r13d           /* mov_l    vr12<%r13>, $0 */
	jmp      .L3                 /* jmp      .L3 */
.L2:
	leaq     -400(%rbp), %r8     /* localaddr vr15<%r8>, $0 */
	movl     %ebx, %r10d         /* sconv_lq vr16<%r9>, vr10<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr17<%rcx>, vr16<%r9>, $40 */
	imulq    $40, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr18<%r9>, vr15<%r8>, vr17<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     %r12d, %r10d        /* sconv_lq vr19<%r8>, vr11<%r12> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr20<%rcx>, vr19<%r8>, $4 */
	imulq    $4, %r10
	movq     %r10, %rcx
	movq     %r9, %r10           /* add_q    vr21<%r8>, vr18<%r9>, vr20<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r8
	movl     %r13d, (%r8)        /* mov_l    (vr21)<%r8>, vr12<%r13> */
	movl     %r13d, %r10d        /* add_l    vr15<%r8>, vr12<%r13>, vr10<%rbx> */
	addl     %ebx, %r10d
	movl     %r10d, %r8d
	movl     %r12d, %r10d        /* add_l    vr16<%r9>, vr11<%r12>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r9d
	movl     %r9d, %r12d         /* mov_l    vr11<%r12>, vr16<%r9> */
	movl     %r8d, %r13d         /* mov_l    vr12<%r13>, vr15<%r8> */
.L3:
	movl     %r12d, %r10d        /* cmplte_l vr18<%r8>, vr11<%r12>, $9 */
	cmpl     $9, %r10d
	setle    %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr18<%r8>, .L2 */
	jne      .L2
	movl     %ebx, %r10d         /* add_l    vr15<%r8>, vr10<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr10<%rbx>, vr15<%r8> */
.L1:
	movl     %ebx, %r10d         /* cmplte_l vr17<%r8>, vr10<%rbx>, $9 */
	cmpl     $9, %r10d
	setle    %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr17<%r8>, .L0 */
	jne      .L0
	movl     $0, %ebx            /* mov_l    vr10<%rbx>, $0 */
	jmp      .L5                 /* jmp      .L5 */
.L4:
	movl     $0, %r12d           /* mov_l    vr11<%r12>, $0 */
	jmp      .L7                 /* jmp      .L7 */
.L6:
	leaq     -400(%rbp), %r8     /* localaddr vr15<%r8>, $0 */
	movl     %ebx, %r10d         /* sconv_lq vr16<%r9>, vr10<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr17<%rcx>, vr16<%r9>, $40 */
	imulq    $40, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr18<%r9>, vr15<%r8>, vr17<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     %r12d, %r10d        /* sconv_lq vr19<%r8>, vr11<%r12> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr20<%rcx>, vr19<%r8>, $4 */
	imulq    $4, %r10
	movq     %r10, %rcx
	movq     %r9, %r10           /* add_q    vr21<%r8>, vr18<%r9>, vr20<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r8
	movl     (%r8), %edi         /* mov_l    vr1, (vr21)<%r8> */
	call     print_i32           /* call     print_i32 */
	call     print_space         /* call     print_space */
	movl     %r12d, %r10d        /* add_l    vr16<%r8>, vr11<%r12>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %r12d         /* mov_l    vr11<%r12>, vr16<%r8> */
.L7:
	movl     %r12d, %r10d        /* cmplte_l vr18<%r8>, vr11<%r12>, $9 */
	cmpl     $9, %r10d
	setle    %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr18<%r8>, .L6 */
	jne      .L6
	call     print_nl            /* call     print_nl */
	movl     %ebx, %r10d         /* add_l    vr15<%r8>, vr10<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr10<%rbx>, vr15<%r8> */
.L5:
	movl     %ebx, %r10d         /* cmplte_l vr17<%r8>, vr10<%rbx>, $9 */
	cmpl     $9, %r10d
	setle    %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr17<%r8>, .L4 */
	jne      .L4
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $408, %rsp          /* leave    $400 */
	popq     %rbp
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
