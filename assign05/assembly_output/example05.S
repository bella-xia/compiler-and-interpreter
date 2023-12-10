/* variable 'a' allocated vreg 10 */
/* variable 'b' allocated vreg 11 */
/* variable 'result' allocated vreg 12 */
/* Function 'sum' uses 0 bytes of memory and 13 virtual registers */
/* allocate machine register %rbx to variable 'result' (vr12) */
/* allocate machine register %r12 to variable 'a' (vr10) */
/* allocate machine register %r13 to variable 'b' (vr11) */
/* variable 'x' allocated vreg 10 */
/* variable 'y' allocated vreg 11 */
/* variable 'z' allocated vreg 12 */
/* Function 'main' uses 0 bytes of memory and 13 virtual registers */
/* allocate machine register %rbx to variable 'x' (vr10) */
/* allocate machine register %r12 to variable 'y' (vr11) */
/* allocate machine register %r13 to variable 'z' (vr12) */
/* Function 'sum' uses 0 total bytes of memory storage for vregs */
/* Function 'sum': placing vreg storage at offset 0 from %rbp */
/* Function 'sum': 8 bytes allocated in stack frame */
	.section .text

	.globl sum
sum:
	pushq    %rbx                /* enter    $0 */
	pushq    %r12
	pushq    %r13
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $8, %rsp
	movl     %edi, %r10d         /* add_l    vr13<%r8>, vr1, vr2 */
	addl     %esi, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %eax          /* mov_l    vr0, vr13<%r8> */
	jmp      .Lsum_return        /* jmp      .Lsum_return */
.Lsum_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
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
	pushq    %r12
	pushq    %r13
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $8, %rsp
	movl     $2, %edi            /* mov_l    vr1, $2 */
	movl     $3, %esi            /* mov_l    vr2, $3 */
	call     sum                 /* call     sum */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
	popq     %r13
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
