/* variable 'p' allocated 24 bytes of storage at offset 0 */
/* variable 's' allocated vreg 10 */
/* Function 'main' uses 24 bytes of memory and 11 virtual registers */
/* allocate machine register %rbx to variable 's' (vr10) */
/* Function 'main' uses 24 total bytes of memory storage for stack allocation */
/* Function 'main': placing memory storage at offset -24 from %rbp */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -24 from %rbp */
/* Function 'main': 24 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbx                /* enter    $24 */
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $24, %rsp
	leaq     -24(%rbp), %r8      /* localaddr vr12<%r8>, $0 */
	movb     $65, (%r8)          /* mov_b    (vr12)<%r8>, $65 */
	movq     %r8, %r10           /* add_q    vr18<%r9>, vr12<%r8>, $1 */
	addq     $1, %r10
	movq     %r10, %r9
	movb     $108, (%r9)         /* mov_b    (vr18)<%r9>, $108 */
	movq     %r8, %r10           /* add_q    vr18<%r9>, vr12<%r8>, $2 */
	addq     $2, %r10
	movq     %r10, %r9
	movb     $105, (%r9)         /* mov_b    (vr18)<%r9>, $105 */
	movq     %r8, %r10           /* add_q    vr18<%r9>, vr12<%r8>, $3 */
	addq     $3, %r10
	movq     %r10, %r9
	movb     $99, (%r9)          /* mov_b    (vr18)<%r9>, $99 */
	movq     %r8, %r10           /* add_q    vr18<%r9>, vr12<%r8>, $4 */
	addq     $4, %r10
	movq     %r10, %r9
	movb     $101, (%r9)         /* mov_b    (vr18)<%r9>, $101 */
	movq     %r8, %r10           /* add_q    vr18<%r9>, vr12<%r8>, $5 */
	addq     $5, %r10
	movq     %r10, %r9
	movb     $0, (%r9)           /* mov_b    (vr18)<%r9>, $0 */
	movq     %r8, %r10           /* add_q    vr14<%r9>, vr12<%r8>, $20 */
	addq     $20, %r10
	movq     %r10, %r9
	movl     $43, (%r9)          /* mov_l    (vr14)<%r9>, $43 */
	movq     %r8, %rdi           /* mov_q    vr1, vr12<%r8> */
	call     print_str           /* call     print_str */
	call     print_nl            /* call     print_nl */
	leaq     -24(%rbp), %r8      /* localaddr vr11<%r8>, $0 */
	movq     %r8, %r10           /* add_q    vr13<%r9>, vr11<%r8>, $20 */
	addq     $20, %r10
	movq     %r10, %r9
	movl     (%r9), %edi         /* mov_l    vr1, (vr13)<%r9> */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $24, %rsp           /* leave    $24 */
	popq     %rbp
	popq     %rbx
	ret                          /* ret       */
