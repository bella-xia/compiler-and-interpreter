/* variable 'a' allocated vreg 10 */
/* variable 'b' allocated vreg 11 */
/* variable 'c' allocated vreg 12 */
/* Function 'main' uses 0 bytes of memory and 13 virtual registers */
/* allocate machine register %rbx to variable 'a' (vr10) */
/* allocate machine register %r12 to variable 'b' (vr11) */
/* allocate machine register %r13 to variable 'c' (vr12) */
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
	movq     $145003225438, %rdi /* mov_q    vr1, $145003225438 */
	call     print_i64           /* call     print_i64 */
	call     print_nl            /* call     print_nl */
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
