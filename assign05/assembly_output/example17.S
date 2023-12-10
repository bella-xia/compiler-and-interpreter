/* variable 'a' allocated vreg 10 */
/* variable 'b' allocated vreg 11 */
/* Function 'main' uses 0 bytes of memory and 12 virtual registers */
/* allocate machine register %rbx to variable 'a' (vr10) */
/* allocate machine register %r12 to variable 'b' (vr11) */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset 0 from %rbp */
/* Function 'main': 0 bytes allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbx                /* enter    $0 */
	pushq    %r12
	pushq    %rbp
	movq     %rsp, %rbp
	movl     $9, %edi            /* mov_l    vr1, $9 */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	popq     %rbp                /* leave    $0 */
	popq     %r12
	popq     %rbx
	ret                          /* ret       */
