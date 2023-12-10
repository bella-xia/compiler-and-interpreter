/* variable 'x' allocated vreg 10 */
/* Function 'main' uses 0 bytes of memory and 11 virtual registers */
/* allocate machine register %rbx to variable 'x' (vr10) */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset 0 from %rbp */
/* Function 'main': 8 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbx                /* enter    $0 */
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $8, %rsp
	movl     $3, %eax            /* mov_l    vr0, $3 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
	popq     %rbx
	ret                          /* ret       */
