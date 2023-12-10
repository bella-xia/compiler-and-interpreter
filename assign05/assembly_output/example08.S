/* variable 'a' allocated vreg 10 */
/* variable 'b' allocated vreg 11 */
/* variable 'c' allocated vreg 12 */
/* Function 'main' uses 0 bytes of memory and 13 virtual registers */
/* allocate machine register %rbx to variable 'c' (vr12) */
/* allocate machine register %r12 to variable 'a' (vr10) */
/* allocate machine register %r13 to variable 'b' (vr11) */
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
	movl     $0, %r8d            /* mov_l    vr15<%r8>, $0 */
	cmpl     $0, %r8d            /* cjmp_f   vr15<%r8>, .L1 */
	je       .L1
	movl     $1, %ebx            /* mov_l    vr12<%rbx>, $1 */
	jmp      .L0                 /* jmp      .L0 */
.L1:
	movl     $2, %ebx            /* mov_l    vr12<%rbx>, $2 */
.L0:
	movl     %ebx, %eax          /* mov_l    vr0, vr12<%rbx> */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
