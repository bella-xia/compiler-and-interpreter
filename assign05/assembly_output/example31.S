/* variable 'arr' allocated vreg 10 */
/* variable 'i' allocated vreg 11 */
/* Function 'read_m500x500' uses 0 bytes of memory and 12 virtual registers */
/* allocate machine register %rbx to variable 'i' (vr11) */
/* allocate machine register %r12 to variable 'arr' (vr10) */
/* variable 'a' allocated 2000000 bytes of storage at offset 0 */
/* variable 'b' allocated 2000000 bytes of storage at offset 2000000 */
/* variable 'c' allocated 2000000 bytes of storage at offset 4000000 */
/* variable 'M' allocated vreg 10 */
/* variable 'i' allocated vreg 11 */
/* variable 'j' allocated vreg 12 */
/* variable 'k' allocated vreg 13 */
/* variable 'r' allocated vreg 14 */
/* variable 'val' allocated vreg 15 */
/* Function 'main' uses 6000000 bytes of memory and 16 virtual registers */
/* allocate machine register %rbx to variable 'i' (vr11) */
/* allocate machine register %r12 to variable 'j' (vr12) */
/* allocate machine register %r13 to variable 'M' (vr10) */
/* allocate machine register %r14 to variable 'k' (vr13) */
/* allocate machine register %r15 to variable 'val' (vr15) */
/* allocated in stack: vr14, */
/* Function 'read_m500x500' uses 0 total bytes of memory storage for vregs */
/* Function 'read_m500x500': placing vreg storage at offset 0 from %rbp */
/* Function 'read_m500x500': 0 bytes allocated in stack frame */
	.section .text

	.globl read_m500x500
read_m500x500:
	pushq    %rbx                /* enter    $0 */
	pushq    %r12
	pushq    %rbp
	movq     %rsp, %rbp
	movq     %rdi, %r12          /* mov_q    vr10<%r12>, vr1 */
	movl     $0, %ebx            /* mov_l    vr11<%rbx>, $0 */
	jmp      .L1                 /* jmp      .L1 */
.L0:
	call     read_i64            /* call     read_i64 */
	movl     %ebx, %r10d         /* sconv_lq vr14<%r8>, vr11<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr15<%r9>, vr14<%r8>, $8 */
	imulq    $8, %r10
	movq     %r10, %r9
	movq     %r12, %r10          /* add_q    vr16<%r8>, vr10<%r12>, vr15<%r9> */
	addq     %r9, %r10
	movq     %r10, %r8
	movq     %rax, (%r8)         /* mov_q    (vr16)<%r8>, vr0 */
	movl     %ebx, %r10d         /* add_l    vr14<%r8>, vr11<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr11<%rbx>, vr14<%r8> */
.L1:
	movl     %ebx, %r10d         /* cmplt_l  vr16<%r8>, vr11<%rbx>, $250000 */
	cmpl     $250000, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr16<%r8>, .L0 */
	jne      .L0
	popq     %rbp                /* leave    $0 */
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
/* Function 'main' uses 6000000 total bytes of memory storage for stack allocation */
/* Function 'main': placing memory storage at offset -6000000 from %rbp */
/* Function 'main' uses 8 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -6000008 from %rbp */
/* Function 'main': 6000008 bytes allocated in stack frame */

	.globl main
main:
	pushq    %rbx                /* enter    $6000000 */
	pushq    %r12
	pushq    %r13
	pushq    %r14
	pushq    %r15
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $6000008, %rsp
	leaq     -6000000(%rbp), %r8 /* localaddr vr16<%r8>, $0 */
	movq     %r8, %rdi           /* mov_q    vr1, vr16<%r8> */
	call     read_m500x500       /* call     read_m500x500 */
	leaq     -4000000(%rbp), %r8 /* localaddr vr16<%r8>, $2000000 */
	movq     %r8, %rdi           /* mov_q    vr1, vr16<%r8> */
	call     read_m500x500       /* call     read_m500x500 */
	movl     $500, %r13d         /* mov_l    vr10<%r13>, $500 */
	movl     $0, %ebx            /* mov_l    vr11<%rbx>, $0 */
	jmp      .L3                 /* jmp      .L3 */
.L2:
	movl     $0, %r12d           /* mov_l    vr12<%r12>, $0 */
	jmp      .L5                 /* jmp      .L5 */
.L4:
	leaq     -2000000(%rbp), %r8 /* localaddr vr19<%r8>, $4000000 */
	movl     %ebx, %r10d         /* sconv_lq vr20<%r9>, vr11<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr21<%rcx>, vr20<%r9>, $4000 */
	imulq    $4000, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr22<%r9>, vr19<%r8>, vr21<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     %r12d, %r10d        /* sconv_lq vr23<%r8>, vr12<%r12> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr24<%rcx>, vr23<%r8>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r9, %r10           /* add_q    vr25<%r8>, vr22<%r9>, vr24<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r8
	movq     $0, (%r8)           /* mov_q    (vr25)<%r8>, $0 */
	movl     %r12d, %r10d        /* add_l    vr19<%r8>, vr12<%r12>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %r12d         /* mov_l    vr12<%r12>, vr19<%r8> */
.L5:
	movl     %r12d, %r10d        /* cmplt_l  vr20<%r8>, vr12<%r12>, vr10<%r13> */
	cmpl     %r13d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr20<%r8>, .L4 */
	jne      .L4
	movl     %ebx, %r10d         /* add_l    vr18<%r8>, vr11<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr11<%rbx>, vr18<%r8> */
.L3:
	movl     %ebx, %r10d         /* cmplt_l  vr19<%r8>, vr11<%rbx>, vr10<%r13> */
	cmpl     %r13d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr19<%r8>, .L2 */
	jne      .L2
	movl     $0, %r14d           /* mov_l    vr13<%r14>, $0 */
	jmp      .L7                 /* jmp      .L7 */
.L6:
	movl     $0, %ebx            /* mov_l    vr11<%rbx>, $0 */
	jmp      .L9                 /* jmp      .L9 */
.L8:
	leaq     -6000000(%rbp), %r8 /* localaddr vr18<%r8>, $0 */
	movl     %ebx, %r10d         /* sconv_lq vr19<%r9>, vr11<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr20<%rcx>, vr19<%r9>, $4000 */
	imulq    $4000, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr21<%r9>, vr18<%r8>, vr20<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     %r14d, %r10d        /* sconv_lq vr22<%r8>, vr13<%r14> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr23<%rcx>, vr22<%r8>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r9, %r10           /* add_q    vr24<%r8>, vr21<%r9>, vr23<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r8
	movq     (%r8), %r10         /* mov_q    vr14<stack_0>, (vr24)<%r8> */
	movq     %r10, -6000008(%rbp)
	movl     $0, %r12d           /* mov_l    vr12<%r12>, $0 */
	jmp      .L11                /* jmp      .L11 */
.L10:
	leaq     -2000000(%rbp), %r8 /* localaddr vr19<%r8>, $4000000 */
	movl     %ebx, %r10d         /* sconv_lq vr20<%r9>, vr11<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr21<%rcx>, vr20<%r9>, $4000 */
	imulq    $4000, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr22<%r9>, vr19<%r8>, vr21<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     %r12d, %r10d        /* sconv_lq vr23<%r8>, vr12<%r12> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr24<%rcx>, vr23<%r8>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r9, %r10           /* add_q    vr25<%r8>, vr22<%r9>, vr24<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r8
	movq     (%r8), %r15         /* mov_q    vr15<%r15>, (vr25)<%r8> */
	leaq     -4000000(%rbp), %r8 /* localaddr vr19<%r8>, $2000000 */
	movl     %r14d, %r10d        /* sconv_lq vr20<%r9>, vr13<%r14> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr21<%rcx>, vr20<%r9>, $4000 */
	imulq    $4000, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr22<%r9>, vr19<%r8>, vr21<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     %r12d, %r10d        /* sconv_lq vr23<%r8>, vr12<%r12> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr24<%rcx>, vr23<%r8>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r9, %r10           /* add_q    vr25<%r8>, vr22<%r9>, vr24<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r8
	movq     (%r8), %r9          /* mov_q    vr26<%r9>, (vr25)<%r8> */
	movq     -6000008(%rbp), %r10 /* mul_q    vr27<%r8>, vr14<stack_0>, vr26<%r9> */
	imulq    %r9, %r10
	movq     %r10, %r8
	movq     %r15, %r10          /* add_q    vr28<%r9>, vr15<%r15>, vr27<%r8> */
	addq     %r8, %r10
	movq     %r10, %r9
	leaq     -2000000(%rbp), %r8 /* localaddr vr19<%r8>, $4000000 */
	movl     %ebx, %r10d         /* sconv_lq vr20<%rcx>, vr11<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %rcx
	movq     %rcx, %r10          /* mul_q    vr21<%rdx>, vr20<%rcx>, $4000 */
	imulq    $4000, %r10
	movq     %r10, %rdx
	movq     %r8, %r10           /* add_q    vr22<%rcx>, vr19<%r8>, vr21<%rdx> */
	addq     %rdx, %r10
	movq     %r10, %rcx
	movl     %r12d, %r10d        /* sconv_lq vr23<%r8>, vr12<%r12> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr24<%rdx>, vr23<%r8>, $8 */
	imulq    $8, %r10
	movq     %r10, %rdx
	movq     %rcx, %r10          /* add_q    vr25<%r8>, vr22<%rcx>, vr24<%rdx> */
	addq     %rdx, %r10
	movq     %r10, %r8
	movq     %r9, (%r8)          /* mov_q    (vr25)<%r8>, vr28<%r9> */
	movl     %r12d, %r10d        /* add_l    vr20<%r8>, vr12<%r12>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %r12d         /* mov_l    vr12<%r12>, vr20<%r8> */
.L11:
	movl     %r12d, %r10d        /* cmplt_l  vr21<%r8>, vr12<%r12>, vr10<%r13> */
	cmpl     %r13d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr21<%r8>, .L10 */
	jne      .L10
	movl     %ebx, %r10d         /* add_l    vr19<%r8>, vr11<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr11<%rbx>, vr19<%r8> */
.L9:
	movl     %ebx, %r10d         /* cmplt_l  vr20<%r8>, vr11<%rbx>, vr10<%r13> */
	cmpl     %r13d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr20<%r8>, .L8 */
	jne      .L8
	movl     %r14d, %r10d        /* add_l    vr18<%r8>, vr13<%r14>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %r14d         /* mov_l    vr13<%r14>, vr18<%r8> */
.L7:
	movl     %r14d, %r10d        /* cmplt_l  vr19<%r8>, vr13<%r14>, vr10<%r13> */
	cmpl     %r13d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr19<%r8>, .L6 */
	jne      .L6
	movl     $0, %ebx            /* mov_l    vr11<%rbx>, $0 */
	jmp      .L13                /* jmp      .L13 */
.L12:
	movl     $0, %r12d           /* mov_l    vr12<%r12>, $0 */
	jmp      .L15                /* jmp      .L15 */
.L14:
	leaq     -2000000(%rbp), %r8 /* localaddr vr18<%r8>, $4000000 */
	movl     %ebx, %r10d         /* sconv_lq vr19<%r9>, vr11<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr20<%rcx>, vr19<%r9>, $4000 */
	imulq    $4000, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr21<%r9>, vr18<%r8>, vr20<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     %r12d, %r10d        /* sconv_lq vr22<%r8>, vr12<%r12> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr23<%rcx>, vr22<%r8>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r9, %r10           /* add_q    vr24<%r8>, vr21<%r9>, vr23<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r8
	movq     (%r8), %rdi         /* mov_q    vr1, (vr24)<%r8> */
	call     print_i64           /* call     print_i64 */
	call     print_nl            /* call     print_nl */
	movl     %r12d, %r10d        /* add_l    vr19<%r8>, vr12<%r12>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %r12d         /* mov_l    vr12<%r12>, vr19<%r8> */
.L15:
	movl     %r12d, %r10d        /* cmplt_l  vr20<%r8>, vr12<%r12>, vr10<%r13> */
	cmpl     %r13d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr20<%r8>, .L14 */
	jne      .L14
	movl     %ebx, %r10d         /* add_l    vr18<%r8>, vr11<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr11<%rbx>, vr18<%r8> */
.L13:
	movl     %ebx, %r10d         /* cmplt_l  vr19<%r8>, vr11<%rbx>, vr10<%r13> */
	cmpl     %r13d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr19<%r8>, .L12 */
	jne      .L12
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $6000008, %rsp      /* leave    $6000000 */
	popq     %rbp
	popq     %r15
	popq     %r14
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
