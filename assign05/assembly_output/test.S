	.section .rodata

s_str_fmt: .string "%s"
s_i32_fmt: .string "%d"
s_i64_fmt: .string "%ld"
s_newline: .string "\n"
s_space:   .string " "

	.section .text

	.globl print_str
print_str:
	subq $8, %rsp
	xorl %eax, %eax
	movq %rdi, %rsi
	movq $s_str_fmt, %rdi
	call printf
	addq $8, %rsp
	ret

	.globl print_i32
print_i32:
	subq $8, %rsp
	xorl %eax, %eax
	movq %rdi, %rsi
	movq $s_i32_fmt, %rdi
	call printf
	addq $8, %rsp
	ret

	.globl read_i32
read_i32:
	subq $8, %rsp         /* align stack, also mem storage for input var */
	xorl %eax, %eax
	movq $s_i32_fmt, %rdi
	movq %rsp, %rsi
	call scanf
	movl (%rsp), %eax
	addq $8, %rsp
	ret

	.globl print_i64
print_i64:
	subq $8, %rsp
	xorl %eax, %eax
	movq %rdi, %rsi
	movq $s_i64_fmt, %rdi
	call printf
	addq $8, %rsp
	ret

	.globl read_i64
read_i64:
	subq $8, %rsp        /* align stack, also mem storage for input var */
	xorl %eax, %eax
	movq $s_i64_fmt, %rdi
	movq %rsp, %rsi
	call scanf
	movq (%rsp), %rax
	addq $8, %rsp
	ret

	.globl print_nl
print_nl:
	subq $8, %rsp
	xorl %eax, %eax
	movq $s_str_fmt, %rdi
	movq $s_newline, %rsi
	call printf
	addq $8, %rsp
	ret

	.globl print_space
print_space:
	subq $8, %rsp
	xorl %eax, %eax
	movq $s_str_fmt, %rdi
	movq $s_space, %rsi
	call printf
	addq $8, %rsp
	ret

	.globl swap
swap:
	pushq    %rbx                /* enter    $0 */
	pushq    %r12
	pushq    %r13
	pushq    %r14
	pushq    %rbp
	movq     %rsp, %rbp
	movl     %esi, %r10d         /* sconv_lq vr14<%r8>, vr2 */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr15<%r9>, vr14<%r8>, $4 */
	imulq    $4, %r10
	movq     %r10, %r9
	movq     %rdi, %r10          /* add_q    vr16<%r8>, vr1, vr15<%r9> */
	addq     %r9, %r10
	movq     %r10, %r8
	movl     (%r8), %r14d        /* mov_l    vr13<%r14>, (vr16)<%r8> */
	movl     %edx, %r10d         /* sconv_lq vr14<%r8>, vr3 */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr15<%r9>, vr14<%r8>, $4 */
	imulq    $4, %r10
	movq     %r10, %r9
	movq     %rdi, %r10          /* add_q    vr16<%r8>, vr1, vr15<%r9> */
	addq     %r9, %r10
	movq     %r10, %r8
	movl     %esi, %r10d         /* sconv_lq vr17<%r9>, vr2 */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr18<%rcx>, vr17<%r9>, $4 */
	imulq    $4, %r10
	movq     %r10, %rcx
	movq     %rdi, %r10          /* add_q    vr19<%r9>, vr1, vr18<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     (%r8), %ecx         /* mov_l    vr20<%rcx>, (vr16)<%r8> */
	movl     %ecx, (%r9)         /* mov_l    (vr19)<%r9>, vr20<%rcx> */
	movl     %edx, %r10d         /* sconv_lq vr14<%r8>, vr3 */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr15<%r9>, vr14<%r8>, $4 */
	imulq    $4, %r10
	movq     %r10, %r9
	movq     %rdi, %r10          /* add_q    vr16<%r8>, vr1, vr15<%r9> */
	addq     %r9, %r10
	movq     %r10, %r8
	movl     %r14d, (%r8)        /* mov_l    (vr16)<%r8>, vr13<%r14> */
	popq     %rbp                /* leave    $0 */
	popq     %r14
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */

	.globl bubble
bubble:
	pushq    %rbx                /* enter    $0 */
	pushq    %r12
	pushq    %r13
	pushq    %r14
	pushq    %rbp
	movq     %rsp, %rbp
	movq     %rdi, %r12          /* mov_q    vr10<%r12>, vr1 */
	movl     %esi, %r13d         /* mov_l    vr11<%r13>, vr2 */
	movl     $1, %ebx            /* mov_l    vr12<%rbx>, $1 */
	jmp      .L1                 /* jmp      .L1 */
.L0:
	movl     %ebx, %r10d         /* sconv_lq vr15<%r8>, vr12<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r8
	movq     %r8, %r10           /* mul_q    vr16<%r9>, vr15<%r8>, $4 */
	imulq    $4, %r10
	movq     %r10, %r9
	movq     %r12, %r10          /* add_q    vr17<%r8>, vr10<%r12>, vr16<%r9> */
	addq     %r9, %r10
	movq     %r10, %r8
	movl     %ebx, %r10d         /* sub_l    vr19<%r9>, vr12<%rbx>, $1 */
	subl     $1, %r10d
	movl     %r10d, %r9d
	movl     %r9d, %r10d         /* sconv_lq vr20<%rcx>, vr19<%r9> */
	movslq   %r10d, %r10
	movq     %r10, %rcx
	movq     %rcx, %r10          /* mul_q    vr21<%r9>, vr20<%rcx>, $4 */
	imulq    $4, %r10
	movq     %r10, %r9
	movq     %r12, %r10          /* add_q    vr22<%rcx>, vr10<%r12>, vr21<%r9> */
	addq     %r9, %r10
	movq     %r10, %rcx
	movl     (%r8), %r9d         /* mov_l    vr23<%r9>, (vr17)<%r8> */
	movl     (%rcx), %r8d        /* mov_l    vr24<%r8>, (vr22)<%rcx> */
	movl     %r9d, %r10d         /* cmplt_l  vr25<%rcx>, vr23<%r9>, vr24<%r8> */
	cmpl     %r8d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %ecx
	cmpl     $0, %ecx            /* cjmp_f   vr25<%rcx>, .L2 */
	je       .L2
	movq     %r8, %rdi           /* mov_q    vr1, vr10<%r8> */
	movl     %ebx, %r10d         /* sub_l    vr27<%r9>, vr12<%rbx>, $1 */
	subl     $1, %r10d
	movl     %r10d, %r9d
	movl     %r9d, %esi          /* mov_l    vr2, vr27<%r9> */
	movl     %ebx, %edx          /* mov_l    vr3, vr12<%rbx> */
	call     swap                /* call     swap */
.L2:
	movl     %ebx, %r10d         /* add_l    vr16<%r8>, vr12<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr12<%rbx>, vr16<%r8> */
.L1:
	movl     %ebx, %r10d         /* cmplt_l  vr17<%r8>, vr12<%rbx>, vr11<%r13> */
	cmpl     %r13d, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr17<%r8>, .L0 */
	jne      .L0
	popq     %rbp                /* leave    $0 */
	popq     %r14
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */

	.globl main
main:
	pushq    %rbx                /* enter    $20 */
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $24, %rsp
	leaq     -24(%rbp), %r8      /* localaddr vr12<%r8>, $0 */
	movl     $52, (%r8)          /* mov_l    (vr12)<%r8>, $52 */
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr12<%r8>, $4 */
	addq     $4, %r10
	movq     %r10, %r9
	movl     $31, (%r9)          /* mov_l    (vr16)<%r9>, $31 */
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr12<%r8>, $8 */
	addq     $8, %r10
	movq     %r10, %r9
	movl     $8, (%r9)           /* mov_l    (vr16)<%r9>, $8 */
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr12<%r8>, $12 */
	addq     $12, %r10
	movq     %r10, %r9
	movl     $41, (%r9)          /* mov_l    (vr16)<%r9>, $41 */
	movq     %r8, %r10           /* add_q    vr16<%r9>, vr12<%r8>, $16 */
	addq     $16, %r10
	movq     %r10, %r9
	movl     $75, (%r9)          /* mov_l    (vr16)<%r9>, $75 */
	movq     %r8, %rdi           /* mov_q    vr1, vr12<%r8> */
	movl     $5, %esi            /* mov_l    vr2, $5 */
	call     bubble              /* call     bubble */
	movl     $0, %ebx            /* mov_l    vr10<%rbx>, $0 */
	jmp      .L4                 /* jmp      .L4 */
.L3:
	leaq     -24(%rbp), %r8      /* localaddr vr12<%r8>, $0 */
	movl     %ebx, %r10d         /* sconv_lq vr13<%r9>, vr10<%rbx> */
	movslq   %r10d, %r10
	movq     %r10, %r9
	movq     %r9, %r10           /* mul_q    vr14<%rcx>, vr13<%r9>, $4 */
	imulq    $4, %r10
	movq     %r10, %rcx
	movq     %r8, %r10           /* add_q    vr15<%r9>, vr12<%r8>, vr14<%rcx> */
	addq     %rcx, %r10
	movq     %r10, %r9
	movl     (%r9), %edi         /* mov_l    vr1, (vr15)<%r9> */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movl     %ebx, %r10d         /* add_l    vr13<%r8>, vr10<%rbx>, $1 */
	addl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr10<%rbx>, vr13<%r8> */
.L4:
	movl     %ebx, %r10d         /* cmplt_l  vr15<%r8>, vr10<%rbx>, $5 */
	cmpl     $5, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr15<%r8>, .L3 */
	jne      .L3
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $24, %rsp           /* leave    $20 */
	popq     %rbp
	popq     %rbx
	ret                          /* ret       */