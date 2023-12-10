/* variable 'n' allocated vreg 10 */
/* variable 'a' allocated vreg 11 */
/* variable 'b' allocated vreg 12 */
/* variable 'c' allocated vreg 13 */
/* variable 'i' allocated vreg 14 */
/* Function 'fib' uses 0 bytes of memory and 15 virtual registers */
/* allocate machine register %rbx to variable 'b' (vr12) */
/* allocate machine register %r12 to variable 'i' (vr14) */
/* allocate machine register %r13 to variable 'n' (vr10) */
/* allocate machine register %r14 to variable 'a' (vr11) */
/* allocate machine register %r15 to variable 'c' (vr13) */
/* variable 'n' allocated vreg 10 */
/* Function 'main' uses 0 bytes of memory and 11 virtual registers */
/* allocate machine register %rbx to variable 'n' (vr10) */
/* Function 'fib' uses 0 total bytes of memory storage for vregs */
/* Function 'fib': placing vreg storage at offset 0 from %rbp */
/* Function 'fib': 8 bytes allocated in stack frame */
	.section .text

	.globl fib
fib:
	pushq    %rbx                /* enter    $0 */
	pushq    %r12
	pushq    %r13
	pushq    %r14
	pushq    %r15
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $8, %rsp
	movl     %edi, %r10d         /* cmplte_l vr16<%r8>, vr1, $1 */
	cmpl     $1, %r10d
	setle    %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	movl     %edi, %r13d         /* mov_l    vr10<%r13>, vr1 */
	cmpl     $0, %r8d            /* cjmp_f   vr16<%r8>, .L0 */
	je       .L0
	movl     %r13d, %eax         /* mov_l    vr0, vr10<%r13> */
	jmp      .Lfib_return        /* jmp      .Lfib_return */
.L0:
	movl     $0, %r14d           /* mov_l    vr11<%r14>, $0 */
	movl     $1, %ebx            /* mov_l    vr12<%rbx>, $1 */
	movl     $2, %r12d           /* mov_l    vr14<%r12>, $2 */
	jmp      .L2                 /* jmp      .L2 */
.L1:
	movl     %r14d, %r10d        /* add_l    vr15<%r8>, vr11<%r14>, vr12<%rbx> */
	addl     %ebx, %r10d
	movl     %r10d, %r8d
	movl     %r12d, %r10d        /* add_l    vr16<%r9>, vr14<%r12>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r9d
	movl     %ebx, %r14d         /* mov_l    vr11<%r14>, vr12<%rbx> */
	movl     %r8d, %ebx          /* mov_l    vr12<%rbx>, vr15<%r8> */
	movl     %r9d, %r12d         /* mov_l    vr14<%r12>, vr16<%r9> */
.L2:
	movl     %r12d, %r10d        /* cmplte_l vr15<%r8>, vr14<%r12>, vr10<%r13> */
	cmpl     %r13d, %r10d
	setle    %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr15<%r8>, .L1 */
	jne      .L1
	movl     %ebx, %eax          /* mov_l    vr0, vr12<%rbx> */
	jmp      .Lfib_return        /* jmp      .Lfib_return */
.Lfib_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
	popq     %r15
	popq     %r14
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset 0 from %rbp */
/* Function 'main': 8 bytes allocated in stack frame */

	.globl main
main:
	pushq    %rbx                /* enter    $0 */
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $8, %rsp
	call     read_i32            /* call     read_i32 */
	movl     %eax, %edi          /* mov_l    vr1, vr0 */
	call     fib                 /* call     fib */
	movl     %eax, %edi          /* mov_l    vr1, vr0 */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
	popq     %rbx
	ret                          /* ret       */
