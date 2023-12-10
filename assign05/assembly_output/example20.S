/* variable 'c' allocated vreg 10 */
/* Function 'ctoi' uses 0 bytes of memory and 11 virtual registers */
/* allocate machine register %rbx to variable 'c' (vr10) */
/* Function 'main' uses 0 bytes of memory and 10 virtual registers */
/* Function 'ctoi' uses 0 total bytes of memory storage for vregs */
/* Function 'ctoi': placing vreg storage at offset 0 from %rbp */
/* Function 'ctoi': 8 bytes allocated in stack frame */
	.section .text

	.globl ctoi
ctoi:
	pushq    %rbx                /* enter    $0 */
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $8, %rsp
	movb     %dil, %r10b         /* sconv_bl vr11<%r8>, vr1 */
	movsbl   %r10b, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %eax          /* mov_l    vr0, vr11<%r8> */
	jmp      .Lctoi_return       /* jmp      .Lctoi_return */
.Lctoi_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
	popq     %rbx
	ret                          /* ret       */
/* Function 'main' uses 0 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset 0 from %rbp */
/* Function 'main': 0 bytes allocated in stack frame */

	.globl main
main:
	pushq    %rbp                /* enter    $0 */
	movq     %rsp, %rbp
	movb     $72, %dil           /* mov_b    vr1, $72 */
	call     ctoi                /* call     ctoi */
	movl     %eax, %edi          /* mov_l    vr1, vr0 */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movb     $101, %dil          /* mov_b    vr1, $101 */
	call     ctoi                /* call     ctoi */
	movl     %eax, %edi          /* mov_l    vr1, vr0 */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movb     $108, %dil          /* mov_b    vr1, $108 */
	call     ctoi                /* call     ctoi */
	movl     %eax, %edi          /* mov_l    vr1, vr0 */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movb     $108, %dil          /* mov_b    vr1, $108 */
	call     ctoi                /* call     ctoi */
	movl     %eax, %edi          /* mov_l    vr1, vr0 */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movb     $111, %dil          /* mov_b    vr1, $111 */
	call     ctoi                /* call     ctoi */
	movl     %eax, %edi          /* mov_l    vr1, vr0 */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	popq     %rbp                /* leave    $0 */
	ret                          /* ret       */
