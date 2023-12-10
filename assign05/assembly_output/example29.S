/* variable 'arr' allocated vreg 10 */
/* variable 'i' allocated vreg 11 */
/* Function 'read_m500x500' uses 0 bytes of memory and 12 virtual registers */
/* allocate machine register %rbx to variable 'i' (vr11) */
/* allocate machine register %r12 to variable 'arr' (vr10) */
/* variable 'a' allocated 2000000 bytes of storage at offset 0 */
/* variable 'b' allocated 2000000 bytes of storage at offset 2000000 */
/* variable 'c' allocated 2000000 bytes of storage at offset 4000000 */
/* variable 'M' allocated vreg 10 */
/* variable 'NELTS' allocated vreg 11 */
/* variable 'SZ' allocated vreg 12 */
/* variable 'i' allocated vreg 13 */
/* variable 'j' allocated vreg 14 */
/* variable 'k' allocated vreg 15 */
/* variable 'r' allocated vreg 16 */
/* variable 'val' allocated vreg 17 */
/* Function 'main' uses 6000000 bytes of memory and 18 virtual registers */
/* allocate machine register %rbx to variable 'i' (vr13) */
/* allocate machine register %r12 to variable 'M' (vr10) */
/* allocate machine register %r13 to variable 'j' (vr14) */
/* allocate machine register %r14 to variable 'k' (vr15) */
/* allocate machine register %r15 to variable 'val' (vr17) */
/* allocated in stack: vr11, vr16, vr12, */
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
/* Function 'main' uses 24 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -6000024 from %rbp */
/* Function 'main': 6000024 bytes allocated in stack frame */

	.globl main
main:
	pushq    %rbx                /* enter    $6000000 */
	pushq    %r12
	pushq    %r13
	pushq    %r14
	pushq    %r15
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $6000024, %rsp
	leaq     -6000000(%rbp), %r8 /* localaddr vr18<%r8>, $0 */
	movq     %r8, %rdi           /* mov_q    vr1, vr18<%r8> */
	call     read_m500x500       /* call     read_m500x500 */
	leaq     -4000000(%rbp), %r8 /* localaddr vr18<%r8>, $2000000 */
	movq     %r8, %rdi           /* mov_q    vr1, vr18<%r8> */
	call     read_m500x500       /* call     read_m500x500 */
	movl     $500, %r12d         /* mov_l    vr10<%r12>, $500 */
	movl     $250000, -6000024(%rbp) /* mov_l    vr11<stack_0>, $250000 */
	movl     $0, %ebx            /* mov_l    vr13<%rbx>, $0 */
	jmp      .L3                 /* jmp      .L3 */
.L2:
	leaq     -2000000(%rbp), %r8 /* localaddr vr20<%r8>, $4000000 */
	movl     %ebx, %r10d         /* sconv_lq vr21<%r9>, vr13<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr22<%rcx>, vr21<%r9>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr23<%r9>, vr20<%r8>, vr22<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movq     $0, (%r9)           /* mov_q    (vr23)<%r9>, $0 */
	movl     %ebx, %r10d         /* add_l    vr20<%r8>, vr13<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr13<%rbx>, vr20<%r8> */
.L3:
	movl     %ebx, %r10d         /* cmplt_l  vr21<%r8>, vr13<%rbx>, vr11<stack_0> */
	cmpl     -6000024(%rbp), %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr21<%r8>, .L2 */
	jne      .L2
	movl     $0, %r14d           /* mov_l    vr15<%r14>, $0 */
	jmp      .L5                 /* jmp      .L5 */
.L4:
	movl     $0, %ebx            /* mov_l    vr13<%rbx>, $0 */
	jmp      .L7                 /* jmp      .L7 */
.L6:
	leaq     -6000000(%rbp), %r8 /* localaddr vr20<%r8>, $0 */
	movl     %ebx, %r10d         /* mul_l    vr21<%r9>, vr13<%rbx>, vr10<%r12> */
	imull    %r12d, %r10d
	movl     %r10d, %r9d
	movl     %r9d, %r10d         /* add_l    vr22<%rcx>, vr21<%r9>, vr15<%r14> */
	addl     %r14d, %r10d
	movl     %r10d, %ecx
	movl     %ecx, %r10d         /* sconv_lq vr23<%r9>, vr22<%rcx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr24<%rcx>, vr23<%r9>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr25<%r9>, vr20<%r8>, vr24<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movq     (%r9), %r10         /* mov_q    vr16<stack_1>, (vr25)<%r9> */
	movq     %r10, -6000016(%rbp)
	movl     $0, %r13d           /* mov_l    vr14<%r13>, $0 */
	jmp      .L9                 /* jmp      .L9 */
.L8:
	leaq     -2000000(%rbp), %r8 /* localaddr vr21<%r8>, $4000000 */
	movl     %ebx, %r10d         /* mul_l    vr22<%r9>, vr13<%rbx>, vr10<%r12> */
	imull    %r12d, %r10d
	movl     %r10d, %r9d
	movl     %r9d, %r10d         /* add_l    vr23<%rcx>, vr22<%r9>, vr14<%r13> */
	addl     %r13d, %r10d
	movl     %r10d, %ecx
	movl     %ecx, %r10d         /* sconv_lq vr24<%r9>, vr23<%rcx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr25<%rcx>, vr24<%r9>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr26<%r9>, vr21<%r8>, vr25<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movq     (%r9), %r15         /* mov_q    vr17<%r15>, (vr26)<%r9> */
	leaq     -4000000(%rbp), %r8 /* localaddr vr21<%r8>, $2000000 */
	movl     %r14d, %r10d        /* mul_l    vr22<%r9>, vr15<%r14>, vr10<%r12> */
	imull    %r12d, %r10d
	movl     %r10d, %r9d
	movl     %r9d, %r10d         /* add_l    vr23<%rcx>, vr22<%r9>, vr14<%r13> */
	addl     %r13d, %r10d
	movl     %r10d, %ecx
	movl     %ecx, %r10d         /* sconv_lq vr24<%r9>, vr23<%rcx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr25<%rcx>, vr24<%r9>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr26<%r9>, vr21<%r8>, vr25<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movq     (%r9), %r8          /* mov_q    vr27<%r8>, (vr26)<%r9> */
	movq     -6000016(%rbp), %r10 /* mul_q    vr28<%r9>, vr16<stack_1>, vr27<%r8> */
	imulq    %r8, %r10
	movq     %r10, %r9
	movq     %r15, %r10          /* add_q    vr29<%r8>, vr17<%r15>, vr28<%r9> */
	addq     %r9, %r10
	movq     %r10, %r8
	leaq     -2000000(%rbp), %r9 /* localaddr vr21<%r9>, $4000000 */
	movl     %ebx, %r10d         /* mul_l    vr22<%rcx>, vr13<%rbx>, vr10<%r12> */
	imull    %r12d, %r10d
	movl     %r10d, %ecx
	movl     %ecx, %r10d         /* add_l    vr23<%rdx>, vr22<%rcx>, vr14<%r13> */
	addl     %r13d, %r10d
	movl     %r10d, %edx
	movl     %edx, %r10d         /* sconv_lq vr24<%rcx>, vr23<%rdx> */
	movslq   %r10d, %r10
	movq     %r10, %rcx
	movq     %rcx, %r10          /* mul_q    vr25<%rdx>, vr24<%rcx>, $8 */
	imulq    $8, %r10
	movq     %r10, %rdx
	movq     %r9, %r10           /* add_q    vr26<%rcx>, vr21<%r9>, vr25<%rdx> */
	addq     %rdx, %r10
	movq     %r10, %rcx
	movq     %r8, (%rcx)         /* mov_q    (vr26)<%rcx>, vr29<%r8> */
	movl     %r13d, %r10d        /* add_l    vr22<%r8>, vr14<%r13>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %r13d         /* mov_l    vr14<%r13>, vr22<%r8> */
.L9:
	movl     %r13d, %r10d        /* cmplt_l  vr23<%r8>, vr14<%r13>, vr10<%r12> */
	cmpl     %r12d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr23<%r8>, .L8 */
	jne      .L8
	movl     %ebx, %r10d         /* add_l    vr21<%r8>, vr13<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr13<%rbx>, vr21<%r8> */
.L7:
	movl     %ebx, %r10d         /* cmplt_l  vr22<%r8>, vr13<%rbx>, vr10<%r12> */
	cmpl     %r12d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr22<%r8>, .L6 */
	jne      .L6
	movl     %r14d, %r10d        /* add_l    vr20<%r8>, vr15<%r14>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %r14d         /* mov_l    vr15<%r14>, vr20<%r8> */
.L5:
	movl     %r14d, %r10d        /* cmplt_l  vr21<%r8>, vr15<%r14>, vr10<%r12> */
	cmpl     %r12d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr21<%r8>, .L4 */
	jne      .L4
	movl     $0, %ebx            /* mov_l    vr13<%rbx>, $0 */
	jmp      .L11                /* jmp      .L11 */
.L10:
	leaq     -2000000(%rbp), %r8 /* localaddr vr19<%r8>, $4000000 */
	movl     %ebx, %r10d         /* sconv_lq vr20<%r9>, vr13<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr21<%rcx>, vr20<%r9>, $8 */
	imulq    $8, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr22<%r9>, vr19<%r8>, vr21<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movq     (%r9), %rdi         /* mov_q    vr1, (vr22)<%r9> */
	call     print_i64           /* call     print_i64 */
	call     print_nl            /* call     print_nl */
	movl     %ebx, %r10d         /* add_l    vr20<%r8>, vr13<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr13<%rbx>, vr20<%r8> */
.L11:
	movl     %ebx, %r10d         /* cmplt_l  vr22<%r8>, vr13<%rbx>, $250000 */
	cmpl     $250000, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr22<%r8>, .L10 */
	jne      .L10
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $6000024, %rsp      /* leave    $6000000 */
	popq     %rbp
	popq     %r15
	popq     %r14
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
