/* variable 'n' allocated vreg 10 */
/* Function 'main' uses 0 bytes of memory and 11 virtual registers */
/* allocate machine register %rbx to variable 'n' (vr10) */
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
	movl     $10, %ebx           /* mov_l    vr10<%rbx>, $10 */
	jmp      .L1                 /* jmp      .L1 */
.L0:
	movl     %ebx, %edi          /* mov_l    vr1, vr10<%rbx> */
	call     print_i32           /* call     print_i32 */
	call     print_nl            /* call     print_nl */
	movl     %ebx, %r10d         /* sub_l    vr12<%r8>, vr10<%rbx>, $1 */
	subl     $1, %r10d
	movl     %r10d, %r8d
	movl     %r8d, %ebx          /* mov_l    vr10<%rbx>, vr12<%r8> */
.L1:
	movl     %ebx, %r10d         /* cmpgt_l  vr12<%r8>, vr10<%rbx>, $0 */
	cmpl     $0, %r10d
	setg     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, %r8d
	cmpl     $0, %r8d            /* cjmp_t   vr12<%r8>, .L0 */
	jne      .L0
	movl     $0, %eax            /* mov_l    vr0, $0 */
	jmp      .Lmain_return       /* jmp      .Lmain_return */
.Lmain_return:
	addq     $8, %rsp            /* leave    $0 */
	popq     %rbp
	popq     %rbx
	ret                          /* ret       */
