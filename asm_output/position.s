	.file	"position.cpp"
	.text
	.align 2
	.p2align 4
	.def	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0;	.scl	3;	.type	32;	.endef
	.seh_proc	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0
_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0:
.LFB3631:
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$240, %rsp
	.seh_stackalloc	240
	.seh_endprologue
	movl	320(%rsp), %ebx
	movzbl	%r8b, %ebp
	movq	%rcx, %rsi
	movq	%rdx, %rdi
	movl	%r9d, %r12d
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7reserveEy
	movq	(%rsi), %r8
	movabsq	$3688503277381496880, %rax
	movabsq	$3976738051646829616, %rdx
	movq	%rax, 32(%rsp)
	movabsq	$3544667369688283184, %rax
	movq	%rdx, 40(%rsp)
	movabsq	$3832902143785906737, %rdx
	movq	%rax, 48(%rsp)
	movabsq	$4121136918051239473, %rax
	movq	%rdx, 56(%rsp)
	movabsq	$3689066235924983858, %rdx
	movq	%rax, 64(%rsp)
	movabsq	$3977301010190316594, %rax
	movq	%rdx, 72(%rsp)
	movabsq	$3545230328231770162, %rdx
	movq	%rax, 80(%rsp)
	movabsq	$3833465102329393715, %rax
	movq	%rdx, 88(%rsp)
	movabsq	$4121699876594726451, %rdx
	movq	%rax, 96(%rsp)
	movabsq	$3689629194468470836, %rax
	movq	%rdx, 104(%rsp)
	movabsq	$3977863968733803572, %rdx
	movq	%rax, 112(%rsp)
	movabsq	$3545793286775257140, %rax
	movq	%rdx, 120(%rsp)
	movabsq	$3834028060872880693, %rdx
	movq	%rax, 128(%rsp)
	movabsq	$4122262835138213429, %rax
	movq	%rdx, 136(%rsp)
	movabsq	$3690192153011957814, %rdx
	movq	%rax, 144(%rsp)
	movabsq	$3978426927277290550, %rax
	movq	%rdx, 152(%rsp)
	movabsq	$3546356245318744118, %rdx
	movq	%rax, 160(%rsp)
	movabsq	$3834591019416367671, %rax
	movq	%rdx, 168(%rsp)
	movabsq	$4122825793681700407, %rdx
	movq	%rax, 176(%rsp)
	movabsq	$3690755111555444792, %rax
	movq	%rdx, 184(%rsp)
	movabsq	$3978989885820777528, %rdx
	movq	%rax, 192(%rsp)
	movabsq	$3546919203862231096, %rax
	movq	%rdx, 200(%rsp)
	movabsq	$3835153977959854649, %rdx
	movq	%rdx, 216(%rsp)
	movabsq	$16106987313379638, %rdx
	movq	%rax, 208(%rsp)
	movabsq	$4122263930388298034, %rax
	movb	$45, (%r8)
	addq	%rbp, %r8
	movq	%rax, 217(%rsp)
	movq	%rdx, 225(%rsp)
	cmpl	$99, %ebx
	jbe	.L2
	leal	-1(%r12), %ecx
	.p2align 4
	.p2align 3
.L3:
	movl	%ebx, %edx
	movl	%ebx, %eax
	imulq	$1374389535, %rdx, %rdx
	shrq	$37, %rdx
	imull	$100, %edx, %r9d
	subl	%r9d, %eax
	movl	%ebx, %r9d
	movl	%edx, %ebx
	movl	%ecx, %edx
	addl	%eax, %eax
	leal	1(%rax), %r10d
	movzbl	32(%rsp,%rax), %eax
	movzbl	32(%rsp,%r10), %r10d
	movb	%r10b, (%r8,%rdx)
	leal	-1(%rcx), %edx
	subl	$2, %ecx
	movb	%al, (%r8,%rdx)
	cmpl	$9999, %r9d
	ja	.L3
.L2:
	leal	48(%rbx), %eax
	cmpl	$9, %ebx
	jbe	.L5
	addl	%ebx, %ebx
	leal	1(%rbx), %eax
	movzbl	32(%rsp,%rax), %eax
	movb	%al, 1(%r8)
	movzbl	32(%rsp,%rbx), %eax
.L5:
	movb	%al, (%r8)
	movq	(%rsi), %rax
	movq	%rdi, 8(%rsi)
	movb	$0, (%rax,%rdi)
	addq	$240, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	ret
	.seh_endproc
	.align 2
	.p2align 4
	.globl	_ZN8Position23update_zobrist_for_moveERK6S_MOVE5PieceS3_hi
	.def	_ZN8Position23update_zobrist_for_moveERK6S_MOVE5PieceS3_hi;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN8Position23update_zobrist_for_moveERK6S_MOVE5PieceS3_hi
_ZN8Position23update_zobrist_for_moveERK6S_MOVE5PieceS3_hi:
.LFB2793:
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	.seh_endprologue
	movslq	88(%rsp), %r11
	leal	-1(%r8), %eax
	movl	%r9d, %ebp
	movl	80(%rsp), %r9d
	cmpb	$-3, %al
	ja	.L16
	movl	%r8d, %eax
	xorl	%edi, %edi
	andl	$7, %eax
	andl	$8, %r8d
	movl	$6, %r8d
	leal	6(%rax), %r10d
	cmovne	%r8d, %edi
	cmovne	%r10d, %eax
.L9:
	movl	(%rdx), %edx
	movslq	%eax, %r8
	leaq	_ZN7Zobrist5PieceE(%rip), %rbx
	imulq	$120, %r8, %rax
	movl	%edx, %r10d
	movl	%edx, %r12d
	andl	$127, %r10d
	sarl	$7, %r12d
	addq	%r10, %rax
	movl	%r12d, %esi
	movq	(%rbx,%rax,8), %rax
	xorq	200(%rcx), %rax
	andl	$127, %esi
	movq	%rax, 200(%rcx)
	testb	%bpl, %bpl
	je	.L10
	xorl	%r10d, %r10d
	cmpb	$-1, %bpl
	je	.L11
	movl	%ebp, %r10d
	andl	$7, %r10d
	andl	$8, %ebp
	leal	6(%r10), %esi
	cmovne	%esi, %r10d
.L11:
	movslq	%r10d, %r10
	movl	%r12d, %esi
	imulq	$120, %r10, %r10
	andl	$127, %esi
	addq	%rsi, %r10
	xorq	(%rbx,%r10,8), %rax
	movq	%rax, 200(%rcx)
.L10:
	sarl	$20, %edx
	andl	$15, %edx
	je	.L12
	addq	%rdi, %rdx
	andl	$31, %edx
	imulq	$120, %rdx, %rdx
	addq	%rsi, %rdx
	xorq	(%rbx,%rdx,8), %rax
	movq	%rax, %rdx
.L13:
	movzbl	128(%rcx), %eax
	movl	%r9d, %r8d
	leaq	_ZN7Zobrist6CastleE(%rip), %r10
	andl	$15, %r8d
	andl	$15, %eax
	movq	(%r10,%rax,8), %rax
	xorq	(%r10,%r8,8), %rax
	xorq	%rdx, %rax
	cmpl	$119, %r11d
	ja	.L14
	leaq	FILE_RANK_LOOKUPS(%rip), %rdx
	movzbl	(%rdx,%r11), %edx
	cmpb	$7, %dl
	jbe	.L22
.L14:
	movslq	124(%rcx), %rdx
	cmpl	$119, %edx
	ja	.L15
	leaq	FILE_RANK_LOOKUPS(%rip), %r8
	movzbl	(%r8,%rdx), %edx
	cmpb	$7, %dl
	ja	.L15
	leaq	_ZN7Zobrist6EpFileE(%rip), %r8
	xorq	(%r8,%rdx,8), %rax
.L15:
	xorq	_ZN7Zobrist4SideE(%rip), %rax
	movq	%rax, 200(%rcx)
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	ret
	.p2align 4,,10
	.p2align 3
.L12:
	imulq	$120, %r8, %r8
	addq	%rsi, %r8
	xorq	(%rbx,%r8,8), %rax
	movq	%rax, %rdx
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L22:
	leaq	_ZN7Zobrist6EpFileE(%rip), %r8
	xorq	(%r8,%rdx,8), %rax
	jmp	.L14
	.p2align 4,,10
	.p2align 3
.L16:
	xorl	%eax, %eax
	xorl	%edi, %edi
	jmp	.L9
	.seh_endproc
	.align 2
	.p2align 4
	.globl	_ZN8Position18update_zobrist_keyEv
	.def	_ZN8Position18update_zobrist_keyEv;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN8Position18update_zobrist_keyEv
_ZN8Position18update_zobrist_keyEv:
.LFB2794:
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	%rcx, %rbx
	call	_ZN7Zobrist7computeERK8Position
	movq	%rax, 200(%rbx)
	addq	$32, %rsp
	popq	%rbx
	ret
	.seh_endproc
	.align 2
	.p2align 4
	.globl	_ZN8Position5resetEv
	.def	_ZN8Position5resetEv;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN8Position5resetEv
_ZN8Position5resetEv:
.LFB2795:
	.seh_endprologue
	pcmpeqd	%xmm0, %xmm0
	movq	.LC0(%rip), %rax
	movups	%xmm0, 48(%rcx)
	movups	%xmm0, 16(%rcx)
	movups	%xmm0, 32(%rcx)
	movups	%xmm0, 64(%rcx)
	movups	%xmm0, 80(%rcx)
	movups	%xmm0, 96(%rcx)
	movups	%xmm0, (%rcx)
	pxor	%xmm0, %xmm0
	movups	%xmm0, 168(%rcx)
	movups	%xmm0, 180(%rcx)
	pxor	%xmm0, %xmm0
	movq	%rax, 216(%rcx)
	movq	%rax, 224(%rcx)
	movq	%rax, 232(%rcx)
	movq	$-1, 112(%rcx)
	movq	$0, 21(%rcx)
	movq	$0, 31(%rcx)
	movq	$0, 41(%rcx)
	movq	$0, 51(%rcx)
	movq	$0, 61(%rcx)
	movq	$0, 71(%rcx)
	movq	$0, 81(%rcx)
	movq	$0, 91(%rcx)
	movq	$0, 208(%rcx)
	movq	$0, 160(%rcx)
	movq	$0, 776(%rcx)
	movq	$0, 784(%rcx)
	movq	$0, 792(%rcx)
	movq	$0, 800(%rcx)
	movq	$0, 808(%rcx)
	movq	$0, 816(%rcx)
	movq	$0, 824(%rcx)
	movups	%xmm0, 144(%rcx)
	movq	%rax, 240(%rcx)
	movq	%rax, 248(%rcx)
	movq	%rax, 256(%rcx)
	movq	%rax, 264(%rcx)
	movq	%rax, 272(%rcx)
	movq	%rax, 280(%rcx)
	movq	%rax, 288(%rcx)
	movq	%rax, 296(%rcx)
	movq	%rax, 304(%rcx)
	movq	%rax, 312(%rcx)
	movq	%rax, 320(%rcx)
	movq	%rax, 328(%rcx)
	movq	%rax, 336(%rcx)
	movq	%rax, 344(%rcx)
	movq	%rax, 352(%rcx)
	movq	%rax, 360(%rcx)
	movq	%rax, 368(%rcx)
	movq	%rax, 376(%rcx)
	movq	%rax, 384(%rcx)
	movq	%rax, 392(%rcx)
	movq	%rax, 400(%rcx)
	movq	%rax, 408(%rcx)
	movq	%rax, 416(%rcx)
	movq	%rax, 424(%rcx)
	movq	%rax, 432(%rcx)
	movq	%rax, 440(%rcx)
	movq	%rax, 448(%rcx)
	movq	%rax, 456(%rcx)
	movq	%rax, 464(%rcx)
	movq	%rax, 472(%rcx)
	movq	%rax, 480(%rcx)
	movq	%rax, 488(%rcx)
	movq	%rax, 496(%rcx)
	movq	%rax, 504(%rcx)
	movq	%rax, 512(%rcx)
	movq	%rax, 520(%rcx)
	movq	%rax, 528(%rcx)
	movq	%rax, 536(%rcx)
	movq	%rax, 544(%rcx)
	movq	%rax, 552(%rcx)
	movq	%rax, 560(%rcx)
	movq	%rax, 568(%rcx)
	movq	%rax, 576(%rcx)
	movq	%rax, 584(%rcx)
	movq	%rax, 592(%rcx)
	movq	%rax, 600(%rcx)
	movq	%rax, 608(%rcx)
	movq	%rax, 616(%rcx)
	movq	%rax, 624(%rcx)
	movq	%rax, 632(%rcx)
	movq	%rax, 640(%rcx)
	movq	%rax, 648(%rcx)
	movq	%rax, 656(%rcx)
	movq	%rax, 664(%rcx)
	movq	%rax, 672(%rcx)
	movq	%rax, 680(%rcx)
	movq	%rax, 688(%rcx)
	movq	%rax, 696(%rcx)
	movq	%rax, 704(%rcx)
	movq	%rax, 712(%rcx)
	movq	%rax, 720(%rcx)
	movq	%rax, 728(%rcx)
	movq	%rax, 736(%rcx)
	movq	%rax, 744(%rcx)
	movq	%rax, 752(%rcx)
	movq	%rax, 760(%rcx)
	movq	%rax, 768(%rcx)
	movq	%rax, 136(%rcx)
	movq	832(%rcx), %rax
	movb	$2, 120(%rcx)
	movl	$-1, 124(%rcx)
	movl	$0, 856(%rcx)
	movl	$65536, 130(%rcx)
	movb	$0, 128(%rcx)
	movq	$0, 200(%rcx)
	cmpq	840(%rcx), %rax
	je	.L24
	movq	%rax, 840(%rcx)
.L24:
	ret
	.seh_endproc
	.section .rdata,"dr"
.LC4:
	.ascii "basic_string::append\0"
	.text
	.align 2
	.p2align 4
	.globl	_ZNK8Position6to_fenB5cxx11Ev
	.def	_ZNK8Position6to_fenB5cxx11Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZNK8Position6to_fenB5cxx11Ev
_ZNK8Position6to_fenB5cxx11Ev:
.LFB2815:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$152, %rsp
	.seh_stackalloc	152
	.seh_endprologue
	movl	$3518437209, %esi
	leaq	16(%rcx), %r12
	movb	$0, 16(%rcx)
	movq	%rcx, %rbx
	leaq	78(%rdx), %rdi
	movq	%rdx, 232(%rsp)
	movq	%r12, (%rcx)
	movq	$0, 8(%rcx)
	movl	$7, 60(%rsp)
.L28:
	leaq	-8(%rdi), %r14
.L179:
	movzbl	21(%r14), %r15d
	testb	%r15b, %r15b
	jne	.L43
	xorl	%ecx, %ecx
.L30:
	addq	$1, %r14
	addl	$1, %ecx
	cmpq	%r14, %rdi
	je	.L29
	movzbl	21(%r14), %r15d
	testb	%r15b, %r15b
	je	.L30
	cmpl	$9, %ecx
	jle	.L31
	movl	%ecx, %eax
	movl	$1, %r9d
	jmp	.L36
	.p2align 4,,10
	.p2align 3
.L32:
	cmpl	$999, %eax
	jbe	.L187
	cmpl	$9999, %eax
	jbe	.L188
	movl	%eax, %edx
	addl	$4, %r9d
	imulq	%rsi, %rdx
	shrq	$45, %rdx
	cmpl	$99999, %eax
	jbe	.L33
	movl	%edx, %eax
.L36:
	cmpl	$99, %eax
	ja	.L32
	addl	$1, %r9d
.L33:
	movl	%ecx, 32(%rsp)
	leaq	112(%rsp), %r13
	xorl	%r8d, %r8d
	movl	%r9d, %edx
	leaq	128(%rsp), %rbp
	movq	%r13, %rcx
	movq	$0, 120(%rsp)
	movq	%rbp, 112(%rsp)
	movb	$0, 128(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0
	movq	120(%rsp), %r8
.L120:
	movabsq	$9223372036854775806, %rax
	subq	8(%rbx), %rax
	movq	112(%rsp), %rdx
	cmpq	%r8, %rax
	jb	.L189
	movq	%rbx, %rcx
.LEHB0:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcy
.LEHE0:
	movq	112(%rsp), %rcx
	cmpq	%rbp, %rcx
	je	.L43
	movq	128(%rsp), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
.L43:
	movl	%r15d, %ecx
.LEHB1:
	call	_Z7to_char5Piece
.LEHE1:
	movl	%eax, %r15d
	movq	(%rbx), %rax
	movq	8(%rbx), %rbp
	cmpq	%rax, %r12
	je	.L128
	movq	16(%rbx), %rdx
.L39:
	leaq	1(%rbp), %r13
	cmpq	%r13, %rdx
	jb	.L190
.L40:
	movb	%r15b, (%rax,%rbp)
	movq	(%rbx), %rax
	addq	$1, %r14
	movq	%r13, 8(%rbx)
	movb	$0, 1(%rax,%rbp)
	cmpq	%r14, %rdi
	jne	.L179
.L41:
	movl	60(%rsp), %eax
	testl	%eax, %eax
	jne	.L191
	movq	(%rbx), %rax
	movq	8(%rbx), %rsi
	cmpq	%rax, %r12
	je	.L192
	movq	16(%rbx), %rdx
	leaq	1(%rsi), %rdi
	cmpq	%rdi, %rdx
	jb	.L193
.L57:
	movb	$32, (%rax,%rsi)
	movq	(%rbx), %rax
	movq	%rdi, 8(%rbx)
	movb	$0, 1(%rax,%rsi)
	movq	232(%rsp), %rax
	movq	8(%rbx), %rdi
	cmpb	$1, 120(%rax)
	movq	(%rbx), %rax
	sbbl	%esi, %esi
	andl	$21, %esi
	addl	$98, %esi
	cmpq	%rax, %r12
	je	.L133
	movq	16(%rbx), %rdx
.L59:
	leaq	1(%rdi), %rbp
	cmpq	%rbp, %rdx
	jb	.L194
.L180:
	movb	%sil, (%rax,%rdi)
	movq	(%rbx), %rax
	movq	%rbp, 8(%rbx)
	movb	$0, 1(%rax,%rdi)
	movq	(%rbx), %rax
	movq	8(%rbx), %rdi
	cmpq	%rax, %r12
	je	.L134
	movq	16(%rbx), %rdx
.L61:
	leaq	1(%rdi), %rsi
	cmpq	%rsi, %rdx
	jb	.L195
.L62:
	movb	$32, (%rax,%rdi)
	movq	(%rbx), %rax
	movl	$1, %ebp
	xorl	%edi, %edi
	movq	%rsi, 8(%rbx)
	movb	$0, (%rax,%rsi)
	movq	232(%rsp), %rax
	leaq	96(%rsp), %rsi
	movq	%rsi, 80(%rsp)
	movzbl	128(%rax), %eax
	movb	$0, 96(%rsp)
	movq	$0, 88(%rsp)
	testb	$1, %al
	jne	.L196
.L63:
	testb	$2, %al
	jne	.L197
	testb	$4, %al
	jne	.L198
	testb	$8, %al
	jne	.L121
	testq	%rdi, %rdi
	je	.L122
	leaq	128(%rsp), %rbp
	movq	80(%rsp), %r13
	movq	$1, 72(%rsp)
	movq	%rbp, 112(%rsp)
.L123:
	addq	$1, %rdi
	movq	%rbp, %rcx
.L73:
	movq	%rdi, %r8
	movq	%r13, %rdx
	call	memcpy
	movq	72(%rsp), %r8
.L75:
	movq	%r8, 120(%rsp)
.L71:
	movabsq	$9223372036854775806, %rax
	subq	8(%rbx), %rax
	movq	112(%rsp), %rdx
	cmpq	%r8, %rax
	jb	.L199
	movq	%rbx, %rcx
.LEHB2:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcy
.LEHE2:
	movq	112(%rsp), %rcx
	cmpq	%rbp, %rcx
	je	.L77
	movq	128(%rsp), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
.L77:
	movq	(%rbx), %rax
	movq	8(%rbx), %rdi
	cmpq	%rax, %r12
	je	.L139
	movq	16(%rbx), %rdx
.L78:
	leaq	1(%rdi), %r13
	cmpq	%r13, %rdx
	jb	.L200
.L186:
	movb	$32, (%rax,%rdi)
	movq	(%rbx), %rax
	movq	%r13, 8(%rbx)
	movb	$0, 1(%rax,%rdi)
	movq	232(%rsp), %rax
	movq	8(%rbx), %rdi
	movslq	124(%rax), %rax
	cmpl	$-1, %eax
	je	.L201
	cmpl	$119, %eax
	ja	.L141
	leaq	FILE_RANK_LOOKUPS(%rip), %rdx
	movzbl	120(%rdx,%rax), %r13d
	movzbl	(%rdx,%rax), %eax
	movq	(%rbx), %rdx
	addl	$97, %eax
	cmpq	%rdx, %r12
	je	.L142
.L217:
	movq	16(%rbx), %rcx
.L85:
	leaq	1(%rdi), %r15
	cmpq	%r15, %rcx
	jb	.L202
.L182:
	movb	%al, (%rdx,%rdi)
	movq	(%rbx), %rax
	addl	$49, %r13d
	movq	%r15, 8(%rbx)
	movb	$0, 1(%rax,%rdi)
	movq	(%rbx), %rax
	movq	8(%rbx), %r15
	cmpq	%rax, %r12
	je	.L143
	movq	16(%rbx), %rcx
.L87:
	leaq	1(%r15), %rdi
	cmpq	%rdi, %rcx
	jb	.L203
.L88:
	movb	%r13b, (%rax,%r15)
	movq	(%rbx), %rax
	movq	%rdi, 8(%rbx)
	movb	$0, (%rax,%rdi)
	movq	(%rbx), %rax
	movq	8(%rbx), %rdi
	cmpq	%rax, %r12
	je	.L144
.L216:
	movq	16(%rbx), %rdx
.L89:
	leaq	1(%rdi), %r13
	cmpq	%r13, %rdx
	jb	.L204
.L90:
	movb	$32, (%rax,%rdi)
	movq	(%rbx), %rax
	movq	%r13, 8(%rbx)
	movb	$0, 1(%rax,%rdi)
	movq	232(%rsp), %rax
	movzwl	130(%rax), %eax
	cmpl	$9, %eax
	jbe	.L91
	cmpl	$99, %eax
	jbe	.L145
	cmpl	$999, %eax
	jbe	.L146
	cmpl	$10000, %eax
	sbbq	%rdx, %rdx
	addq	$5, %rdx
	cmpl	$10000, %eax
	sbbl	%r9d, %r9d
	addl	$5, %r9d
.L92:
	movl	%eax, 32(%rsp)
	leaq	112(%rsp), %r13
	xorl	%r8d, %r8d
	movq	%r13, %rcx
	movq	%rbp, 112(%rsp)
	movq	$0, 120(%rsp)
	movb	$0, 128(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0
	movq	120(%rsp), %r8
.L110:
	movabsq	$9223372036854775806, %rax
	subq	8(%rbx), %rax
	movq	112(%rsp), %rdx
	cmpq	%r8, %rax
	jb	.L205
	movq	%rbx, %rcx
.LEHB3:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcy
.LEHE3:
	movq	112(%rsp), %rcx
	cmpq	%rbp, %rcx
	je	.L94
	movq	128(%rsp), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
.L94:
	movq	(%rbx), %rax
	movq	8(%rbx), %rdi
	cmpq	%rax, %r12
	je	.L148
	movq	16(%rbx), %rdx
.L95:
	leaq	1(%rdi), %r12
	cmpq	%r12, %rdx
	jb	.L206
.L96:
	movb	$32, (%rax,%rdi)
	movq	(%rbx), %rax
	movq	%r12, 8(%rbx)
	movb	$0, 1(%rax,%rdi)
	movq	232(%rsp), %rax
	movzwl	132(%rax), %eax
	cmpl	$9, %eax
	jbe	.L97
	cmpl	$99, %eax
	jbe	.L149
	cmpl	$999, %eax
	jbe	.L150
	cmpl	$10000, %eax
	sbbq	%rdx, %rdx
	addq	$5, %rdx
	cmpl	$10000, %eax
	sbbl	%r9d, %r9d
	addl	$5, %r9d
.L98:
	movl	%eax, 32(%rsp)
	xorl	%r8d, %r8d
	movq	%r13, %rcx
	movq	%rbp, 112(%rsp)
	movq	$0, 120(%rsp)
	movb	$0, 128(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0
	movq	120(%rsp), %r8
.L109:
	movabsq	$9223372036854775806, %rax
	subq	8(%rbx), %rax
	movq	112(%rsp), %rdx
	cmpq	%r8, %rax
	jb	.L207
	movq	%rbx, %rcx
.LEHB4:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcy
.LEHE4:
	movq	112(%rsp), %rcx
	cmpq	%rbp, %rcx
	je	.L100
	movq	128(%rsp), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
.L100:
	movq	80(%rsp), %rcx
	cmpq	%rsi, %rcx
	je	.L27
	movq	96(%rsp), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
.L27:
	movq	%rbx, %rax
	addq	$152, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L190:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rbp, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
.LEHB5:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
.LEHE5:
	movq	(%rbx), %rax
	jmp	.L40
	.p2align 4,,10
	.p2align 3
.L128:
	movl	$15, %edx
	jmp	.L39
	.p2align 4,,10
	.p2align 3
.L187:
	addl	$2, %r9d
	jmp	.L33
	.p2align 4,,10
	.p2align 3
.L188:
	addl	$3, %r9d
	jmp	.L33
	.p2align 4,,10
	.p2align 3
.L191:
	movq	(%rbx), %rax
	movq	8(%rbx), %r13
	cmpq	%rax, %r12
	je	.L131
	movq	16(%rbx), %rdx
.L54:
	leaq	1(%r13), %rbp
	cmpq	%rbp, %rdx
	jb	.L208
.L55:
	movb	$47, (%rax,%r13)
	movq	(%rbx), %rax
	subq	$10, %rdi
	movq	%rbp, 8(%rbx)
	subl	$1, 60(%rsp)
	movb	$0, (%rax,%rbp)
	jmp	.L28
	.p2align 4,,10
	.p2align 3
.L29:
	cmpl	$9, %ecx
	jle	.L124
	movl	%ecx, %eax
	movl	$1, %r9d
	jmp	.L48
	.p2align 4,,10
	.p2align 3
.L44:
	cmpl	$999, %eax
	jbe	.L209
	cmpl	$9999, %eax
	jbe	.L210
	movl	%eax, %edx
	addl	$4, %r9d
	imulq	%rsi, %rdx
	shrq	$45, %rdx
	cmpl	$99999, %eax
	jbe	.L45
	movl	%edx, %eax
.L48:
	cmpl	$99, %eax
	ja	.L44
	addl	$1, %r9d
.L45:
	movl	%ecx, 32(%rsp)
	leaq	112(%rsp), %r13
	xorl	%r8d, %r8d
	movl	%r9d, %edx
	leaq	128(%rsp), %rbp
	movq	%r13, %rcx
	movq	$0, 120(%rsp)
	movq	%rbp, 112(%rsp)
	movb	$0, 128(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0
	movq	120(%rsp), %r8
.L119:
	movabsq	$9223372036854775806, %rax
	subq	8(%rbx), %rax
	movq	112(%rsp), %rdx
	cmpq	%r8, %rax
	jb	.L211
	movq	%rbx, %rcx
.LEHB6:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcy
.LEHE6:
	movq	112(%rsp), %rcx
	cmpq	%rbp, %rcx
	je	.L41
	movq	128(%rsp), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
	jmp	.L41
.L208:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%r13, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
.LEHB7:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
	movq	(%rbx), %rax
	jmp	.L55
.L192:
	movl	$15, %edx
	leaq	1(%rsi), %rdi
	cmpq	%rdi, %rdx
	jnb	.L57
.L193:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
.LEHE7:
	movq	(%rbx), %rax
	jmp	.L57
.L131:
	movl	$15, %edx
	jmp	.L54
.L209:
	addl	$2, %r9d
	jmp	.L45
.L210:
	addl	$3, %r9d
	jmp	.L45
.L198:
	movq	%rsi, %rax
.L66:
	movb	$107, (%rax,%rdi)
	movq	80(%rsp), %rax
	movq	%rbp, 88(%rsp)
	movb	$0, (%rax,%rbp)
	movq	232(%rsp), %rax
	movq	88(%rsp), %rdi
	testb	$8, 128(%rax)
	jne	.L113
.L67:
	testq	%rdi, %rdi
	je	.L122
	movq	%rdi, 72(%rsp)
	movq	80(%rsp), %r13
	leaq	112(%rsp), %rcx
	leaq	128(%rsp), %rbp
	movq	%rbp, 112(%rsp)
	cmpq	$15, %rdi
	jbe	.L123
	leaq	72(%rsp), %rdx
	xorl	%r8d, %r8d
	leaq	80(%rsp), %r14
.LEHB8:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERyy
.LEHE8:
	movq	%rax, 112(%rsp)
	movq	%rax, %rcx
	movq	72(%rsp), %rax
	movq	%rax, 128(%rsp)
	leaq	1(%rax), %rdi
	testq	%rax, %rax
	je	.L212
	movq	$-1, %r8
	testq	%rdi, %rdi
	je	.L75
	jmp	.L73
.L212:
	movzbl	0(%r13), %eax
	movb	%al, (%rcx)
	movq	72(%rsp), %r8
	jmp	.L75
.L31:
	movl	%ecx, 32(%rsp)
	leaq	112(%rsp), %r13
	xorl	%r8d, %r8d
	leaq	128(%rsp), %rbp
	movl	$1, %r9d
	movl	$1, %edx
	movq	%r13, %rcx
	movq	%rbp, 112(%rsp)
	movq	$0, 120(%rsp)
	movb	$0, 128(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0
	movq	120(%rsp), %r8
	jmp	.L120
.L206:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rdi, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
	leaq	80(%rsp), %r14
.LEHB9:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
	movq	(%rbx), %rax
	jmp	.L96
.L204:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rdi, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
	leaq	80(%rsp), %r14
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
	movq	(%rbx), %rax
	jmp	.L90
.L200:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rdi, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
	leaq	80(%rsp), %r14
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
	movq	(%rbx), %rax
	jmp	.L186
.L122:
	leaq	112(%rsp), %rcx
	movl	$45, %r8d
	movl	$1, %edx
	leaq	128(%rsp), %rbp
	leaq	80(%rsp), %r14
	movq	%rbp, 112(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructEyc
.LEHE9:
	movq	120(%rsp), %r8
	jmp	.L71
.L197:
	movb	$81, 96(%rsp,%rdi)
	movq	80(%rsp), %rax
	movq	%rbp, 88(%rsp)
	movb	$0, (%rax,%rbp)
	movq	232(%rsp), %rax
	movq	88(%rsp), %rdi
	movzbl	128(%rax), %eax
	testb	$4, %al
	jne	.L213
	testb	$8, %al
	je	.L67
.L113:
	movq	80(%rsp), %rax
	cmpq	%rsi, %rax
	je	.L137
	movq	96(%rsp), %rdx
.L68:
	leaq	1(%rdi), %rbp
	cmpq	%rbp, %rdx
	jb	.L214
.L69:
	movb	$113, (%rax,%rdi)
	movq	80(%rsp), %rax
	movq	%rbp, 88(%rsp)
	movb	$0, (%rax,%rbp)
	movq	88(%rsp), %rdi
	jmp	.L67
.L195:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rdi, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
.LEHB10:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
	movq	(%rbx), %rax
	jmp	.L62
.L194:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rdi, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
.LEHE10:
	movq	(%rbx), %rax
	jmp	.L180
.L196:
	movl	$75, %edx
	movl	$2, %ebp
	movl	$1, %edi
	movq	$1, 88(%rsp)
	movw	%dx, 96(%rsp)
	jmp	.L63
.L149:
	movl	$2, %edx
	movl	$2, %r9d
	jmp	.L98
.L145:
	movl	$2, %edx
	movl	$2, %r9d
	jmp	.L92
.L203:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%r15, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
	leaq	80(%rsp), %r14
.LEHB11:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
	movq	(%rbx), %rax
	jmp	.L88
.L202:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rdi, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
	leaq	80(%rsp), %r14
	movb	%al, 60(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
	movq	(%rbx), %rdx
	movzbl	60(%rsp), %eax
	jmp	.L182
.L146:
	movl	$3, %edx
	movl	$3, %r9d
	jmp	.L92
.L150:
	movl	$3, %edx
	movl	$3, %r9d
	jmp	.L98
.L201:
	movq	(%rbx), %rax
	cmpq	%rax, %r12
	je	.L140
	movq	16(%rbx), %rdx
.L81:
	leaq	1(%rdi), %r13
	cmpq	%r13, %rdx
	jb	.L215
.L82:
	movb	$45, (%rax,%rdi)
	movq	(%rbx), %rax
	movq	%r13, 8(%rbx)
	movb	$0, (%rax,%r13)
	movq	(%rbx), %rax
	movq	8(%rbx), %rdi
	cmpq	%rax, %r12
	jne	.L216
.L144:
	movl	$15, %edx
	jmp	.L89
.L214:
	leaq	80(%rsp), %r14
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rdi, %rdx
	movq	$1, 32(%rsp)
	movq	%r14, %rcx
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
	movq	80(%rsp), %rax
	jmp	.L69
.L148:
	movl	$15, %edx
	jmp	.L95
.L139:
	movl	$15, %edx
	jmp	.L78
.L133:
	movl	$15, %edx
	jmp	.L59
.L134:
	movl	$15, %edx
	jmp	.L61
.L141:
	movq	(%rbx), %rdx
	movl	$-1, %r13d
	movl	$96, %eax
	cmpq	%rdx, %r12
	jne	.L217
.L142:
	movl	$15, %ecx
	jmp	.L85
.L143:
	movl	$15, %ecx
	jmp	.L87
.L213:
	movq	80(%rsp), %rax
	movl	$15, %edx
	leaq	1(%rdi), %rbp
	cmpq	%rsi, %rax
	cmovne	96(%rsp), %rdx
	cmpq	%rbp, %rdx
	jnb	.L66
	leaq	80(%rsp), %r14
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rdi, %rdx
	movq	$1, 32(%rsp)
	movq	%r14, %rcx
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
.LEHE11:
	movq	80(%rsp), %rax
	jmp	.L66
.L124:
	movl	%ecx, 32(%rsp)
	leaq	112(%rsp), %r13
	xorl	%r8d, %r8d
	leaq	128(%rsp), %rbp
	movl	$1, %r9d
	movl	$1, %edx
	movq	%r13, %rcx
	movq	%rbp, 112(%rsp)
	movq	$0, 120(%rsp)
	movb	$0, 128(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0
	movq	120(%rsp), %r8
	jmp	.L119
.L121:
	movq	80(%rsp), %rax
	leaq	1(%rdi), %rbp
	jmp	.L69
.L215:
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movq	%rdi, %rdx
	movq	%rbx, %rcx
	movq	$1, 32(%rsp)
	leaq	80(%rsp), %r14
.LEHB12:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy
.LEHE12:
	movq	(%rbx), %rax
	jmp	.L82
.L140:
	movl	$15, %edx
	jmp	.L81
.L97:
	movl	%eax, 32(%rsp)
	xorl	%r8d, %r8d
	movl	$1, %r9d
	movq	%r13, %rcx
	movl	$1, %edx
	movq	%rbp, 112(%rsp)
	movq	$0, 120(%rsp)
	movb	$0, 128(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0
	movq	120(%rsp), %r8
	jmp	.L109
.L91:
	movl	%eax, 32(%rsp)
	leaq	112(%rsp), %r13
	xorl	%r8d, %r8d
	movl	$1, %r9d
	movl	$1, %edx
	movq	%r13, %rcx
	movq	%rbp, 112(%rsp)
	movq	$0, 120(%rsp)
	movb	$0, 128(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE22__resize_and_overwriteIZNS_9to_stringEiEUlPcyE_EEvyT_.isra.0
	movq	120(%rsp), %r8
	jmp	.L110
.L137:
	movl	$15, %edx
	jmp	.L68
.L211:
	leaq	.LC4(%rip), %rcx
.LEHB13:
	call	_ZSt20__throw_length_errorPKc
.LEHE13:
.L205:
	leaq	.LC4(%rip), %rcx
.LEHB14:
	call	_ZSt20__throw_length_errorPKc
.LEHE14:
.L157:
	leaq	112(%rsp), %rcx
	movq	%rax, %rsi
	leaq	80(%rsp), %r14
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv
.L106:
	movq	%r14, %rcx
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv
.L103:
	movq	%rbx, %rcx
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv
	movq	%rsi, %rcx
.LEHB15:
	call	_Unwind_Resume
.LEHE15:
.L158:
.L183:
	movq	%r13, %rcx
	movq	%rax, %rsi
	leaq	80(%rsp), %r14
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv
	jmp	.L106
.L159:
	jmp	.L183
.L155:
.L184:
	movq	%r13, %rcx
	movq	%rax, %rsi
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv
	jmp	.L103
.L156:
	movq	%rax, %rsi
	jmp	.L106
.L199:
	leaq	.LC4(%rip), %rcx
.LEHB16:
	call	_ZSt20__throw_length_errorPKc
.LEHE16:
.L207:
	leaq	.LC4(%rip), %rcx
.LEHB17:
	call	_ZSt20__throw_length_errorPKc
.LEHE17:
.L153:
	jmp	.L184
.L189:
	leaq	.LC4(%rip), %rcx
.LEHB18:
	call	_ZSt20__throw_length_errorPKc
.LEHE18:
.L154:
	movq	%rax, %rsi
	jmp	.L103
	.seh_handler	__gxx_personality_seh0, @unwind, @except
	.seh_handlerdata
.LLSDA2815:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE2815-.LLSDACSB2815
.LLSDACSB2815:
	.uleb128 .LEHB0-.LFB2815
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L153-.LFB2815
	.uleb128 0
	.uleb128 .LEHB1-.LFB2815
	.uleb128 .LEHE1-.LEHB1
	.uleb128 .L154-.LFB2815
	.uleb128 0
	.uleb128 .LEHB2-.LFB2815
	.uleb128 .LEHE2-.LEHB2
	.uleb128 .L157-.LFB2815
	.uleb128 0
	.uleb128 .LEHB3-.LFB2815
	.uleb128 .LEHE3-.LEHB3
	.uleb128 .L158-.LFB2815
	.uleb128 0
	.uleb128 .LEHB4-.LFB2815
	.uleb128 .LEHE4-.LEHB4
	.uleb128 .L159-.LFB2815
	.uleb128 0
	.uleb128 .LEHB5-.LFB2815
	.uleb128 .LEHE5-.LEHB5
	.uleb128 .L154-.LFB2815
	.uleb128 0
	.uleb128 .LEHB6-.LFB2815
	.uleb128 .LEHE6-.LEHB6
	.uleb128 .L155-.LFB2815
	.uleb128 0
	.uleb128 .LEHB7-.LFB2815
	.uleb128 .LEHE7-.LEHB7
	.uleb128 .L154-.LFB2815
	.uleb128 0
	.uleb128 .LEHB8-.LFB2815
	.uleb128 .LEHE8-.LEHB8
	.uleb128 .L156-.LFB2815
	.uleb128 0
	.uleb128 .LEHB9-.LFB2815
	.uleb128 .LEHE9-.LEHB9
	.uleb128 .L156-.LFB2815
	.uleb128 0
	.uleb128 .LEHB10-.LFB2815
	.uleb128 .LEHE10-.LEHB10
	.uleb128 .L154-.LFB2815
	.uleb128 0
	.uleb128 .LEHB11-.LFB2815
	.uleb128 .LEHE11-.LEHB11
	.uleb128 .L156-.LFB2815
	.uleb128 0
	.uleb128 .LEHB12-.LFB2815
	.uleb128 .LEHE12-.LEHB12
	.uleb128 .L156-.LFB2815
	.uleb128 0
	.uleb128 .LEHB13-.LFB2815
	.uleb128 .LEHE13-.LEHB13
	.uleb128 .L155-.LFB2815
	.uleb128 0
	.uleb128 .LEHB14-.LFB2815
	.uleb128 .LEHE14-.LEHB14
	.uleb128 .L158-.LFB2815
	.uleb128 0
	.uleb128 .LEHB15-.LFB2815
	.uleb128 .LEHE15-.LEHB15
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB16-.LFB2815
	.uleb128 .LEHE16-.LEHB16
	.uleb128 .L157-.LFB2815
	.uleb128 0
	.uleb128 .LEHB17-.LFB2815
	.uleb128 .LEHE17-.LEHB17
	.uleb128 .L159-.LFB2815
	.uleb128 0
	.uleb128 .LEHB18-.LFB2815
	.uleb128 .LEHE18-.LEHB18
	.uleb128 .L153-.LFB2815
	.uleb128 0
.LLSDACSE2815:
	.text
	.seh_endproc
	.align 2
	.p2align 4
	.globl	_ZN8Position18save_derived_stateER6S_UNDO
	.def	_ZN8Position18save_derived_stateER6S_UNDO;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN8Position18save_derived_stateER6S_UNDO
_ZN8Position18save_derived_stateER6S_UNDO:
.LFB2816:
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	.seh_endprologue
	movdqu	144(%rcx), %xmm0
	movdqu	168(%rcx), %xmm1
	movdqu	180(%rcx), %xmm2
	movq	%rdx, %rax
	movq	136(%rcx), %rdx
	movq	%rcx, %r8
	leaq	216(%r8), %r9
	movups	%xmm0, 48(%rax)
	movdqu	776(%r8), %xmm3
	movdqu	792(%r8), %xmm4
	movdqu	808(%r8), %xmm5
	movq	%rdx, 36(%rax)
	movq	160(%rcx), %rdx
	movups	%xmm1, 72(%rax)
	movups	%xmm2, 84(%rax)
	movq	%rdx, 64(%rax)
	movq	208(%rcx), %rdx
	leaq	108(%rax), %rcx
	movq	%rdx, 100(%rax)
	movq	216(%r8), %rdx
	movq	%rdx, 108(%rax)
	movq	768(%r8), %rdx
	movq	%rdx, 660(%rax)
	leaq	116(%rax), %rdx
	andq	$-8, %rdx
	subq	%rdx, %rcx
	movq	%rdx, %rdi
	movq	824(%r8), %rdx
	subq	%rcx, %r9
	addl	$560, %ecx
	shrl	$3, %ecx
	movq	%r9, %rsi
	rep movsq
	movq	%rdx, 716(%rax)
	movups	%xmm3, 668(%rax)
	movups	%xmm4, 684(%rax)
	movups	%xmm5, 700(%rax)
	popq	%rsi
	popq	%rdi
	ret
	.seh_endproc
	.align 2
	.p2align 4
	.globl	_ZN8Position14rebuild_countsEv
	.def	_ZN8Position14rebuild_countsEv;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN8Position14rebuild_countsEv
_ZN8Position14rebuild_countsEv:
.LFB2817:
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	.seh_endprologue
	movq	.LC0(%rip), %rax
	pxor	%xmm0, %xmm0
	xorl	%edx, %edx
	leaq	240+FILE_RANK_LOOKUPS(%rip), %r11
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rbx
	leaq	256+MAILBOX_MAPS(%rip), %rsi
	movq	$0, 776(%rcx)
	movq	%rcx, %r8
	movq	$0, 784(%rcx)
	movq	$0, 792(%rcx)
	movq	$0, 800(%rcx)
	movq	$0, 808(%rcx)
	movq	$0, 816(%rcx)
	movq	$0, 824(%rcx)
	movq	%rax, 216(%rcx)
	movq	%rax, 224(%rcx)
	movq	%rax, 232(%rcx)
	movq	%rax, 240(%rcx)
	movq	%rax, 248(%rcx)
	movq	%rax, 256(%rcx)
	movq	%rax, 264(%rcx)
	movq	%rax, 272(%rcx)
	movq	%rax, 280(%rcx)
	movq	%rax, 288(%rcx)
	movq	%rax, 296(%rcx)
	movq	%rax, 304(%rcx)
	movq	%rax, 312(%rcx)
	movq	%rax, 320(%rcx)
	movq	%rax, 328(%rcx)
	movq	%rax, 336(%rcx)
	movq	%rax, 344(%rcx)
	movq	%rax, 352(%rcx)
	movq	%rax, 360(%rcx)
	movq	%rax, 368(%rcx)
	movq	%rax, 376(%rcx)
	movq	%rax, 384(%rcx)
	movq	%rax, 392(%rcx)
	movq	%rax, 400(%rcx)
	movq	%rax, 408(%rcx)
	movq	%rax, 416(%rcx)
	movq	%rax, 424(%rcx)
	movq	%rax, 432(%rcx)
	movq	%rax, 440(%rcx)
	movq	%rax, 448(%rcx)
	movq	%rax, 456(%rcx)
	movq	%rax, 464(%rcx)
	movq	%rax, 472(%rcx)
	movq	%rax, 480(%rcx)
	movq	%rax, 488(%rcx)
	movq	%rax, 496(%rcx)
	movq	%rax, 504(%rcx)
	movq	%rax, 512(%rcx)
	movq	%rax, 520(%rcx)
	movq	%rax, 528(%rcx)
	movq	%rax, 536(%rcx)
	movq	%rax, 544(%rcx)
	movq	%rax, 552(%rcx)
	movq	%rax, 560(%rcx)
	movq	%rax, 568(%rcx)
	movq	%rax, 576(%rcx)
	movq	%rax, 584(%rcx)
	movq	%rax, 592(%rcx)
	movq	%rax, 600(%rcx)
	movq	%rax, 608(%rcx)
	movq	%rax, 616(%rcx)
	movq	%rax, 624(%rcx)
	movq	%rax, 632(%rcx)
	movq	%rax, 640(%rcx)
	movq	%rax, 648(%rcx)
	movq	%rax, 656(%rcx)
	movq	%rax, 664(%rcx)
	movq	%rax, 672(%rcx)
	movups	%xmm0, 168(%rcx)
	movups	%xmm0, 180(%rcx)
	pxor	%xmm0, %xmm0
	movq	%rax, 680(%rcx)
	movq	%rax, 688(%rcx)
	movq	%rax, 696(%rcx)
	movq	%rax, 704(%rcx)
	movq	%rax, 712(%rcx)
	movq	%rax, 720(%rcx)
	movq	%rax, 728(%rcx)
	movq	%rax, 736(%rcx)
	movq	%rax, 744(%rcx)
	movq	%rax, 752(%rcx)
	movq	%rax, 760(%rcx)
	movq	%rax, 768(%rcx)
	movq	$0, 208(%rcx)
	movq	$0, 160(%rcx)
	movq	%rax, 136(%rcx)
	movups	%xmm0, 144(%rcx)
	jmp	.L224
	.p2align 4,,10
	.p2align 3
.L222:
	cmpb	$1, %bpl
	je	.L226
.L223:
	movl	(%rbx,%rax,4), %eax
	addl	%eax, 208(%r8,%r9,4)
.L221:
	addq	$1, %rdx
	cmpq	$120, %rdx
	je	.L227
.L224:
	movzbl	(%r8,%rdx), %eax
	leal	-1(%rax), %ecx
	cmpb	$-3, %cl
	ja	.L221
	cmpb	$1, (%r11,%rdx)
	jne	.L221
	movl	%eax, %r9d
	movl	%eax, %ebp
	andl	$7, %eax
	shrb	$3, %r9b
	leaq	(%rax,%rax,4), %r13
	andl	$7, %ebp
	andl	$1, %r9d
	imulq	$70, %r9, %r10
	leaq	0(,%r9,8), %rcx
	subq	%r9, %rcx
	addq	%rax, %rcx
	movslq	776(%r8,%rcx,4), %r12
	leaq	(%r10,%r13,2), %r10
	addq	%r12, %r10
	movl	%edx, 216(%r8,%r10,4)
	addl	$1, 776(%r8,%rcx,4)
	addl	$1, 168(%r8,%rax,4)
	cmpb	$6, %bpl
	jne	.L222
	movl	%edx, 136(%r8,%r9,4)
	addq	$1, %rdx
	cmpq	$120, %rdx
	jne	.L224
.L227:
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	ret
	.p2align 4,,10
	.p2align 3
.L226:
	movl	(%rsi,%rdx,4), %ecx
	testl	%ecx, %ecx
	js	.L223
	movl	$1, %r10d
	salq	%cl, %r10
	orq	%r10, 144(%r8,%r9,8)
	orq	%r10, 160(%r8)
	jmp	.L223
	.seh_endproc
	.section	.text$_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PyS9_EN11_Save_errnoD1Ev,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PyS9_EN11_Save_errnoD1Ev
	.def	_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PyS9_EN11_Save_errnoD1Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PyS9_EN11_Save_errnoD1Ev
_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PyS9_EN11_Save_errnoD1Ev:
.LFB2852:
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$40, %rsp
	.seh_stackalloc	40
	.seh_endprologue
	movq	__imp__errno(%rip), %rsi
	movq	%rcx, %rbx
	call	*%rsi
	movl	(%rax), %eax
	testl	%eax, %eax
	je	.L230
	addq	$40, %rsp
	popq	%rbx
	popq	%rsi
	ret
	.p2align 4,,10
	.p2align 3
.L230:
	movl	(%rbx), %ebx
	call	*%rsi
	movl	%ebx, (%rax)
	addq	$40, %rsp
	popq	%rbx
	popq	%rsi
	ret
	.seh_handler	__gxx_personality_seh0, @unwind, @except
	.seh_handlerdata
.LLSDA2852:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE2852-.LLSDACSB2852
.LLSDACSB2852:
.LLSDACSE2852:
	.section	.text$_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PyS9_EN11_Save_errnoD1Ev,"x"
	.linkonce discard
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC6:
	.ascii "basic_string: construction from null is not valid\0"
.LC7:
	.ascii "vector::_M_realloc_append\0"
.LC8:
	.ascii "stoi\0"
	.text
	.align 2
	.p2align 4
	.globl	_ZN8Position12set_from_fenERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE
	.def	_ZN8Position12set_from_fenERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN8Position12set_from_fenERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE
_ZN8Position12set_from_fenERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE:
.LFB2796:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$568, %rsp
	.seh_stackalloc	568
	movups	%xmm6, 544(%rsp)
	.seh_savexmm	%xmm6, 544
	.seh_endprologue
	movq	.refptr._ZTVSt15basic_streambufIcSt11char_traitsIcEE(%rip), %rax
	addq	$16, %rax
	movq	%rax, %xmm6
	call	_ZN8Position5resetEv
	movq	%rcx, %r13
	leaq	280(%rsp), %rax
	movq	%rdx, %rbx
	movq	%rax, %rcx
	movq	%rax, 56(%rsp)
	leaq	160(%rsp), %r12
	call	_ZNSt8ios_baseC2Ev
	pxor	%xmm0, %xmm0
	xorl	%r9d, %r9d
	xorl	%edx, %edx
	movq	.refptr._ZTVSt9basic_iosIcSt11char_traitsIcEE(%rip), %rax
	movups	%xmm0, 512(%rsp)
	movw	%r9w, 504(%rsp)
	addq	$16, %rax
	movups	%xmm0, 528(%rsp)
	movq	%rax, 280(%rsp)
	movq	.refptr._ZTTNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE(%rip), %rax
	movq	$0, 496(%rsp)
	movq	8(%rax), %rsi
	movq	16(%rax), %rdi
	movq	-24(%rsi), %rax
	movq	%rsi, 160(%rsp)
	movq	%rsi, 88(%rsp)
	movq	%rdi, 160(%rsp,%rax)
	movq	$0, 168(%rsp)
	movq	-24(%rsi), %rcx
	movq	%rdi, 96(%rsp)
	addq	%r12, %rcx
.LEHB19:
	call	_ZNSt9basic_iosIcSt11char_traitsIcEE4initEPSt15basic_streambufIcS1_E
.LEHE19:
	movq	.refptr._ZTVNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE(%rip), %rax
	pxor	%xmm0, %xmm0
	movq	$0, 224(%rsp)
	movups	%xmm6, 176(%rsp)
	addq	$24, %rax
	movups	%xmm0, 192(%rsp)
	movq	%rax, 160(%rsp)
	movq	.refptr._ZTVNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE(%rip), %rax
	movups	%xmm0, 208(%rsp)
	addq	$64, %rax
	movq	%rax, 280(%rsp)
	leaq	232(%rsp), %rax
	movq	%rax, %rcx
	movq	%rax, 80(%rsp)
	call	_ZNSt6localeC1Ev
	movq	8(%rbx), %rdi
	movq	.refptr._ZTVNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEEE(%rip), %rsi
	movl	$0, 240(%rsp)
	movq	(%rbx), %rbx
	leaq	16(%rsi), %rax
	testq	%rbx, %rbx
	movq	%rax, 176(%rsp)
	leaq	264(%rsp), %rax
	sete	%dl
	testq	%rdi, %rdi
	movq	%rax, 64(%rsp)
	movq	%rax, 248(%rsp)
	setne	%al
	andb	%al, %dl
	movb	%dl, 79(%rsp)
	jne	.L379
	movq	%rdi, 128(%rsp)
	cmpq	$15, %rdi
	ja	.L380
	cmpq	$1, %rdi
	jne	.L235
	movzbl	(%rbx), %eax
	movb	%al, 264(%rsp)
.L237:
	movq	64(%rsp), %rax
.L236:
	movq	%rdi, 256(%rsp)
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	leaq	176(%rsp), %rcx
	movb	$0, (%rax,%rdi)
	movq	248(%rsp), %rdx
	movl	$8, 240(%rsp)
.LEHB20:
	call	_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7_M_syncEPcyy
.LEHE20:
	movq	56(%rsp), %rcx
	leaq	176(%rsp), %rdx
.LEHB21:
	call	_ZNSt9basic_iosIcSt11char_traitsIcEE4initEPSt15basic_streambufIcS1_E
.LEHE21:
	leaq	144(%rsp), %rax
	xorl	%ebp, %ebp
	xorl	%ebx, %ebx
	xorl	%esi, %esi
	movq	%rax, 104(%rsp)
	leaq	128(%rsp), %r15
	movq	%rax, 128(%rsp)
	leaq	112(%rsp), %rax
	movq	$0, 136(%rsp)
	movb	$0, 144(%rsp)
	movq	%rax, 32(%rsp)
.L246:
	movq	%r15, %rdx
	movq	%r12, %rcx
.LEHB22:
	call	_ZStrsIcSt11char_traitsIcESaIcEERSt13basic_istreamIT_T0_ES7_RNSt7__cxx1112basic_stringIS4_S5_T1_EE
	movq	(%rax), %rdx
	movq	-24(%rdx), %rdx
	testb	$5, 32(%rax,%rdx)
	jne	.L381
.L273:
	cmpq	%rbx, %rbp
	je	.L247
	movq	136(%rsp), %rax
	leaq	16(%rbx), %rcx
	movq	128(%rsp), %rdi
	movq	%rcx, (%rbx)
	movq	%rax, 112(%rsp)
	cmpq	$15, %rax
	ja	.L248
	leaq	1(%rax), %r8
	testq	%rax, %rax
	je	.L249
.L250:
	movq	%rdi, %rdx
	call	memcpy
	movq	112(%rsp), %rax
.L252:
	movq	%rax, 8(%rbx)
	movq	%r15, %rdx
	movq	%r12, %rcx
	addq	$32, %rbx
	call	_ZStrsIcSt11char_traitsIcESaIcEERSt13basic_istreamIT_T0_ES7_RNSt7__cxx1112basic_stringIS4_S5_T1_EE
	movq	(%rax), %rdx
	movq	-24(%rdx), %rdx
	testb	$5, 32(%rax,%rdx)
	je	.L273
.L381:
	movq	%rbx, %rax
	subq	%rsi, %rax
	cmpq	$192, %rax
	jne	.L275
	movq	(%rsi), %rdx
	movq	8(%rsi), %r9
	addq	%rdx, %r9
	cmpq	%r9, %rdx
	je	.L275
	movl	$7, 32(%rsp)
	movq	%rdx, %rdi
	xorl	%r14d, %r14d
	jmp	.L280
	.p2align 4,,10
	.p2align 3
.L277:
	movsbl	%al, %ecx
	leal	-48(%rcx), %eax
	cmpl	$9, %eax
	ja	.L278
.L383:
	addq	$1, %rdi
	addl	%eax, %r14d
	cmpq	%rdi, %r9
	je	.L382
.L280:
	movzbl	(%rdi), %eax
	cmpb	$47, %al
	jne	.L277
	cmpl	$8, %r14d
	jne	.L275
	addq	$1, %rdi
	cmpq	%rdi, %r9
	je	.L275
	movzbl	(%rdi), %eax
	cmpb	$47, %al
	je	.L275
	movsbl	%al, %ecx
	subl	$1, 32(%rsp)
	xorl	%r14d, %r14d
	leal	-48(%rcx), %eax
	cmpl	$9, %eax
	jbe	.L383
.L278:
	movq	%r9, 48(%rsp)
	movl	%ecx, 40(%rsp)
	call	_Z9from_charc
	testb	%al, %al
	je	.L275
	movzbl	32(%rsp), %eax
	movl	40(%rsp), %ecx
	leal	(%rax,%rax,4), %r10d
	movzbl	%r14b, %eax
	leal	21(%rax,%r10,2), %eax
	movl	%eax, 72(%rsp)
	call	_Z9from_charc
	movslq	72(%rsp), %rdx
	movq	48(%rsp), %r9
	addq	$1, %rdi
	addl	$1, %r14d
	movb	%al, 0(%r13,%rdx)
	cmpq	%rdi, %r9
	jne	.L280
.L382:
	movl	32(%rsp), %ecx
	testl	%ecx, %ecx
	jne	.L275
	cmpl	$8, %r14d
	jne	.L275
	cmpq	$1, 40(%rsi)
	je	.L384
	.p2align 4
	.p2align 3
.L275:
	movq	128(%rsp), %rcx
	cmpq	104(%rsp), %rcx
	je	.L305
	movq	144(%rsp), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
.L305:
	movq	.refptr._ZTVNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE(%rip), %rax
	movq	248(%rsp), %rcx
	addq	$24, %rax
	movq	%rax, 160(%rsp)
	movq	.refptr._ZTVNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE(%rip), %rax
	addq	$64, %rax
	movq	%rax, 280(%rsp)
	cmpq	64(%rsp), %rcx
	je	.L306
	movq	264(%rsp), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
.L306:
	movq	.refptr._ZTVSt15basic_streambufIcSt11char_traitsIcEE(%rip), %rax
	movq	80(%rsp), %rcx
	addq	$16, %rax
	movq	%rax, 176(%rsp)
	call	_ZNSt6localeD1Ev
	movq	88(%rsp), %rax
	movq	96(%rsp), %rdi
	movq	56(%rsp), %rcx
	movq	%rax, 160(%rsp)
	movq	-24(%rax), %rax
	movq	%rdi, 160(%rsp,%rax)
	movq	.refptr._ZTVSt9basic_iosIcSt11char_traitsIcEE(%rip), %rax
	movq	%rsi, %rdi
	movq	$0, 168(%rsp)
	addq	$16, %rax
	movq	%rax, 280(%rsp)
	call	_ZNSt8ios_baseD2Ev
	cmpq	%rsi, %rbx
	je	.L311
	.p2align 4
	.p2align 3
.L307:
	movq	(%rdi), %rcx
	leaq	16(%rdi), %rax
	cmpq	%rax, %rcx
	je	.L310
	movq	16(%rdi), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
.L310:
	addq	$32, %rdi
	cmpq	%rdi, %rbx
	jne	.L307
.L311:
	testq	%rsi, %rsi
	je	.L231
	movq	%rbp, %rdx
	movq	%rsi, %rcx
	subq	%rsi, %rdx
	call	_ZdlPvy
.L231:
	movzbl	79(%rsp), %eax
	movups	544(%rsp), %xmm6
	addq	$568, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L235:
	testq	%rdi, %rdi
	je	.L237
	movq	64(%rsp), %rcx
	jmp	.L234
	.p2align 4,,10
	.p2align 3
.L249:
	movzbl	(%rdi), %eax
	movb	%al, (%rcx)
	movq	112(%rsp), %rax
	jmp	.L252
	.p2align 4,,10
	.p2align 3
.L248:
	movq	32(%rsp), %rdx
	xorl	%r8d, %r8d
	movq	%rbx, %rcx
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERyy
	movq	%rax, (%rbx)
	movq	%rax, %rcx
	movq	112(%rsp), %rax
	movq	%rax, 16(%rbx)
	leaq	1(%rax), %r8
	testq	%rax, %rax
	je	.L249
	movq	$-1, %rax
	testq	%r8, %r8
	je	.L252
	jmp	.L250
	.p2align 4,,10
	.p2align 3
.L247:
	movabsq	$288230376151711743, %rdi
	movq	%rbp, %r9
	subq	%rsi, %r9
	movq	%r9, %rax
	sarq	$5, %rax
	cmpq	%rdi, %rax
	je	.L385
	testq	%rax, %rax
	movl	$1, %edi
	movq	%r9, 40(%rsp)
	cmovne	%rax, %rdi
	addq	%rax, %rdi
	movabsq	$288230376151711743, %rax
	cmpq	%rax, %rdi
	cmova	%rax, %rdi
	salq	$5, %rdi
	movq	%rdi, %rcx
	call	_Znwy
.LEHE22:
	movq	40(%rsp), %r9
	movq	%rax, %r14
	movq	128(%rsp), %r10
	addq	%rax, %r9
	movq	136(%rsp), %rax
	leaq	16(%r9), %rcx
	movq	%rcx, (%r9)
	movq	%rax, 120(%rsp)
	cmpq	$15, %rax
	ja	.L255
	leaq	1(%rax), %r8
	testq	%rax, %rax
	je	.L256
.L257:
	movq	%r10, %rdx
	movq	%r9, 40(%rsp)
	call	memcpy
	movq	120(%rsp), %rax
	movq	40(%rsp), %r9
.L259:
	movq	%rax, 8(%r9)
	cmpq	%rsi, %rbx
	je	.L320
.L388:
	leaq	16(%rsi), %rcx
	movq	%rsi, %rdx
	movq	%r14, %rax
	movq	%rbp, %r11
	jmp	.L269
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L261:
	movq	%r8, (%rax)
	movq	16(%rdx), %r8
	movq	%r8, 16(%rax)
.L377:
	movq	8(%rdx), %r9
.L268:
	addq	$32, %rdx
	movq	%r9, 8(%rax)
	addq	$32, %rcx
	addq	$32, %rax
	cmpq	%rdx, %rbx
	je	.L386
.L269:
	leaq	16(%rax), %r10
	movq	%r10, (%rax)
	movq	(%rdx), %r8
	cmpq	%rcx, %r8
	jne	.L261
	movq	8(%rdx), %r9
	leaq	1(%r9), %r8
	cmpl	$8, %r8d
	jnb	.L262
	testb	$4, %r8b
	jne	.L387
	testl	%r8d, %r8d
	je	.L268
	movzbl	(%rcx), %r9d
	movb	%r9b, (%r10)
	testb	$2, %r8b
	je	.L377
	movl	%r8d, %r8d
	movzwl	-2(%rcx,%r8), %r9d
	movw	%r9w, -2(%r10,%r8)
	movq	8(%rdx), %r9
	jmp	.L268
	.p2align 4,,10
	.p2align 3
.L386:
	movq	%r11, %rbp
.L260:
	leaq	32(%rax), %rbx
	testq	%rsi, %rsi
	je	.L270
	movq	%rbp, %rdx
	movq	%rsi, %rcx
	subq	%rsi, %rdx
	call	_ZdlPvy
.L270:
	leaq	(%r14,%rdi), %rbp
	movq	%r14, %rsi
	jmp	.L246
	.p2align 4,,10
	.p2align 3
.L262:
	movq	(%rcx), %r9
	movq	%r9, (%r10)
	movl	%r8d, %r9d
	movq	-8(%rcx,%r9), %rbp
	movq	%rbp, -8(%r10,%r9)
	leaq	24(%rax), %r9
	movq	%rcx, %rbp
	andq	$-8, %r9
	subq	%r9, %r10
	addl	%r10d, %r8d
	subq	%r10, %rbp
	andl	$-8, %r8d
	cmpl	$8, %r8d
	jb	.L377
	movq	%r11, 40(%rsp)
	andl	$-8, %r8d
	xorl	%r10d, %r10d
	movq	%rax, 48(%rsp)
.L266:
	movl	%r10d, %r11d
	addl	$8, %r10d
	movq	0(%rbp,%r11), %rax
	movq	%rax, (%r9,%r11)
	cmpl	%r8d, %r10d
	jb	.L266
	movq	40(%rsp), %r11
	movq	48(%rsp), %rax
	movq	8(%rdx), %r9
	jmp	.L268
	.p2align 4,,10
	.p2align 3
.L256:
	movzbl	(%r10), %eax
	movb	%al, (%rcx)
	movq	120(%rsp), %rax
	movq	%rax, 8(%r9)
	cmpq	%rsi, %rbx
	jne	.L388
.L320:
	movq	%r14, %rax
	jmp	.L260
	.p2align 4,,10
	.p2align 3
.L255:
	leaq	120(%rsp), %rdx
	xorl	%r8d, %r8d
	movq	%r9, %rcx
	movq	%r10, 48(%rsp)
	movq	%r9, 40(%rsp)
.LEHB23:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERyy
.LEHE23:
	movq	40(%rsp), %r9
	movq	%rax, %rcx
	movq	48(%rsp), %r10
	movq	%rax, (%r9)
	movq	120(%rsp), %rax
	testq	%rax, %rax
	movq	%rax, 16(%r9)
	leaq	1(%rax), %r8
	je	.L256
	movq	$-1, %rax
	testq	%r8, %r8
	je	.L259
	jmp	.L257
	.p2align 4,,10
	.p2align 3
.L380:
	leaq	248(%rsp), %rcx
	xorl	%r8d, %r8d
	leaq	128(%rsp), %rdx
.LEHB24:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERyy
.LEHE24:
	movq	%rax, 248(%rsp)
	movq	%rax, %rcx
	movq	128(%rsp), %rax
	movq	%rax, 264(%rsp)
.L234:
	movq	%rdi, %r8
	movq	%rbx, %rdx
	call	memcpy
	movq	128(%rsp), %rdi
	movq	248(%rsp), %rax
	jmp	.L236
.L384:
	movq	32(%rsi), %rax
	movzbl	(%rax), %eax
	cmpb	$119, %al
	je	.L321
	cmpb	$98, %al
	jne	.L275
	movl	$1, %eax
.L281:
	movq	72(%rsi), %rcx
	movb	%al, 120(%r13)
	movb	$0, 128(%r13)
	movq	64(%rsi), %rax
	cmpq	$1, %rcx
	je	.L389
	addq	%rax, %rcx
	cmpq	%rax, %rcx
	jne	.L289
	jmp	.L374
	.p2align 4,,10
	.p2align 3
.L285:
	cmpb	$81, %dl
	je	.L390
	cmpb	$107, %dl
	je	.L391
	cmpb	$113, %dl
	jne	.L275
	movzbl	128(%r13), %edx
	orl	$8, %edx
.L286:
	addq	$1, %rax
	movb	%dl, 128(%r13)
	cmpq	%rax, %rcx
	je	.L374
.L289:
	movzbl	(%rax), %edx
	cmpb	$75, %dl
	jne	.L285
	movzbl	128(%r13), %edx
	orl	$1, %edx
	jmp	.L286
.L387:
	movl	(%rcx), %r9d
	movl	%r8d, %r8d
	movl	%r9d, (%r10)
	movl	-4(%rcx,%r8), %r9d
	movl	%r9d, -4(%r10,%r8)
	movq	8(%rdx), %r9
	jmp	.L268
.L321:
	xorl	%eax, %eax
	jmp	.L281
	.p2align 4,,10
	.p2align 3
.L390:
	movzbl	128(%r13), %edx
	orl	$2, %edx
	jmp	.L286
	.p2align 4,,10
	.p2align 3
.L391:
	movzbl	128(%r13), %edx
	orl	$4, %edx
	jmp	.L286
.L374:
	movq	104(%rsi), %rax
	cmpq	$1, %rax
	je	.L392
	cmpq	$2, %rax
	jne	.L275
	movq	96(%rsi), %rdx
	movzbl	(%rdx), %eax
	subl	$97, %eax
	cmpb	$7, %al
	ja	.L275
	movzbl	1(%rdx), %edx
	cmpb	$51, %dl
	je	.L333
	cmpb	$54, %dl
	jne	.L275
.L333:
	subl	$49, %edx
	movzbl	%al, %eax
	movzbl	%dl, %edx
	leal	(%rdx,%rdx,4), %edx
	leal	21(%rax,%rdx,2), %eax
.L292:
	movl	%eax, 124(%r13)
	movq	128(%rsi), %r14
	movq	__imp__errno(%rip), %rdi
.LEHB25:
	call	*%rdi
	movl	(%rax), %eax
	movl	%eax, 48(%rsp)
	movl	%eax, 112(%rsp)
	call	*%rdi
.LEHE25:
	movl	$0, (%rax)
	leaq	120(%rsp), %rax
	movl	$10, %r8d
	movq	%r14, %rcx
	movq	%rax, %rdx
	movq	%rax, 32(%rsp)
	call	strtol
	movl	%eax, 40(%rsp)
	cmpq	120(%rsp), %r14
	je	.L393
.LEHB26:
	call	*%rdi
.LEHE26:
	cmpl	$34, (%rax)
	je	.L394
	call	*%rdi
	movl	(%rax), %edx
	testl	%edx, %edx
	je	.L395
.L296:
	movzwl	40(%rsp), %eax
	movq	160(%rsi), %r14
	movw	%ax, 130(%r13)
.LEHB27:
	call	*%rdi
	movl	(%rax), %eax
	movl	%eax, 40(%rsp)
	movl	%eax, 112(%rsp)
	call	*%rdi
.LEHE27:
	movl	$0, (%rax)
	movq	32(%rsp), %rdx
	movl	$10, %r8d
	movq	%r14, %rcx
	call	strtol
	movl	%eax, 32(%rsp)
	cmpq	120(%rsp), %r14
	je	.L396
.LEHB28:
	call	*%rdi
.LEHE28:
	cmpl	$34, (%rax)
	je	.L397
	call	*%rdi
	movl	(%rax), %eax
	testl	%eax, %eax
	je	.L398
.L302:
	movzwl	32(%rsp), %eax
	movq	%r13, %rcx
	movw	%ax, 132(%r13)
	call	_ZN8Position14rebuild_countsEv
	movq	%r13, %rcx
.LEHB29:
	call	_ZN7Zobrist7computeERK8Position
.LEHE29:
	movq	%rax, 200(%r13)
	movb	$1, 79(%rsp)
	jmp	.L275
.L389:
	cmpb	$45, (%rax)
	je	.L374
	leaq	1(%rax), %rcx
	jmp	.L289
	.p2align 4,,10
	.p2align 3
.L392:
	movq	96(%rsi), %rax
	cmpb	$45, (%rax)
	jne	.L275
	movl	$-1, %eax
	jmp	.L292
.L398:
	call	*%rdi
	movl	40(%rsp), %edi
	movl	%edi, (%rax)
	jmp	.L302
.L395:
	call	*%rdi
	movl	48(%rsp), %ecx
	movl	%ecx, (%rax)
	jmp	.L296
.L379:
	leaq	.LC6(%rip), %rcx
.LEHB30:
	call	_ZSt19__throw_logic_errorPKc
.LEHE30:
.L327:
	movq	%rax, %rbx
.L240:
	movq	.refptr._ZTVSt15basic_streambufIcSt11char_traitsIcEE(%rip), %rax
	movq	80(%rsp), %rcx
	movq	%rbx, %rdi
	addq	$16, %rax
	movq	%rax, 176(%rsp)
	call	_ZNSt6localeD1Ev
	jmp	.L241
.L385:
	leaq	.LC7(%rip), %rcx
.LEHB31:
	call	_ZSt20__throw_length_errorPKc
.LEHE31:
.L328:
	leaq	248(%rsp), %rcx
	movq	%rax, %rbx
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv
	jmp	.L240
.L332:
	movq	%rax, %rdi
	jmp	.L272
.L329:
	movq	%rdi, %rdx
	movq	%r14, %rcx
	movq	%rax, %rdi
	call	_ZdlPvy
.L272:
	movq	%r15, %rcx
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv
	movq	%r12, %rcx
	call	_ZNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEED1Ev
.L245:
	movq	%rsi, %r12
.L312:
	cmpq	%r12, %rbx
	je	.L399
	movq	(%r12), %rcx
	leaq	16(%r12), %rax
	cmpq	%rax, %rcx
	je	.L313
	movq	16(%r12), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
.L313:
	addq	$32, %r12
	jmp	.L312
.L326:
	leaq	248(%rsp), %rcx
	addq	$16, %rsi
	movq	%rax, %rdi
	movq	%rsi, 176(%rsp)
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv
	movq	.refptr._ZTVSt15basic_streambufIcSt11char_traitsIcEE(%rip), %rax
	movq	80(%rsp), %rcx
	addq	$16, %rax
	movq	%rax, 176(%rsp)
	call	_ZNSt6localeD1Ev
.L241:
	movq	88(%rsp), %rax
	movq	96(%rsp), %rsi
	xorl	%r8d, %r8d
	movq	%rax, 160(%rsp)
	movq	-24(%rax), %rax
	movq	%rsi, 160(%rsp,%rax)
	movq	%r8, 168(%rsp)
.L244:
	movq	.refptr._ZTVSt9basic_iosIcSt11char_traitsIcEE(%rip), %rax
	movq	56(%rsp), %rcx
	xorl	%ebp, %ebp
	xorl	%ebx, %ebx
	xorl	%esi, %esi
	addq	$16, %rax
	movq	%rax, 280(%rsp)
	call	_ZNSt8ios_baseD2Ev
	jmp	.L245
.L399:
	testq	%rsi, %rsi
	je	.L315
	movq	%rbp, %rdx
	movq	%rsi, %rcx
	subq	%rsi, %rdx
	call	_ZdlPvy
.L315:
	movq	%rdi, %rcx
.LEHB32:
	call	_Unwind_Resume
.LEHE32:
.L325:
	movq	%rax, %rdi
	jmp	.L244
.L396:
	leaq	.LC8(%rip), %rcx
.LEHB33:
	call	_ZSt24__throw_invalid_argumentPKc
.L397:
	leaq	.LC8(%rip), %rcx
	call	_ZSt20__throw_out_of_rangePKc
.LEHE33:
.L331:
.L378:
	leaq	112(%rsp), %rcx
	movq	%rax, %rdi
	call	_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PyS9_EN11_Save_errnoD1Ev
.L299:
	movq	%rdi, %rcx
	call	__cxa_begin_catch
.LEHB34:
	call	__cxa_end_catch
.LEHE34:
	jmp	.L275
.L394:
	leaq	.LC8(%rip), %rcx
.LEHB35:
	call	_ZSt20__throw_out_of_rangePKc
.L393:
	leaq	.LC8(%rip), %rcx
	call	_ZSt24__throw_invalid_argumentPKc
.LEHE35:
.L324:
	movq	%rax, %rdi
	jmp	.L299
.L330:
	jmp	.L378
	.seh_handler	__gxx_personality_seh0, @unwind, @except
	.seh_handlerdata
	.align 4
.LLSDA2796:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATT2796-.LLSDATTD2796
.LLSDATTD2796:
	.byte	0x1
	.uleb128 .LLSDACSE2796-.LLSDACSB2796
.LLSDACSB2796:
	.uleb128 .LEHB19-.LFB2796
	.uleb128 .LEHE19-.LEHB19
	.uleb128 .L325-.LFB2796
	.uleb128 0
	.uleb128 .LEHB20-.LFB2796
	.uleb128 .LEHE20-.LEHB20
	.uleb128 .L328-.LFB2796
	.uleb128 0
	.uleb128 .LEHB21-.LFB2796
	.uleb128 .LEHE21-.LEHB21
	.uleb128 .L326-.LFB2796
	.uleb128 0
	.uleb128 .LEHB22-.LFB2796
	.uleb128 .LEHE22-.LEHB22
	.uleb128 .L332-.LFB2796
	.uleb128 0
	.uleb128 .LEHB23-.LFB2796
	.uleb128 .LEHE23-.LEHB23
	.uleb128 .L329-.LFB2796
	.uleb128 0
	.uleb128 .LEHB24-.LFB2796
	.uleb128 .LEHE24-.LEHB24
	.uleb128 .L327-.LFB2796
	.uleb128 0
	.uleb128 .LEHB25-.LFB2796
	.uleb128 .LEHE25-.LEHB25
	.uleb128 .L324-.LFB2796
	.uleb128 0x1
	.uleb128 .LEHB26-.LFB2796
	.uleb128 .LEHE26-.LEHB26
	.uleb128 .L330-.LFB2796
	.uleb128 0x1
	.uleb128 .LEHB27-.LFB2796
	.uleb128 .LEHE27-.LEHB27
	.uleb128 .L324-.LFB2796
	.uleb128 0x1
	.uleb128 .LEHB28-.LFB2796
	.uleb128 .LEHE28-.LEHB28
	.uleb128 .L331-.LFB2796
	.uleb128 0x1
	.uleb128 .LEHB29-.LFB2796
	.uleb128 .LEHE29-.LEHB29
	.uleb128 .L332-.LFB2796
	.uleb128 0
	.uleb128 .LEHB30-.LFB2796
	.uleb128 .LEHE30-.LEHB30
	.uleb128 .L327-.LFB2796
	.uleb128 0
	.uleb128 .LEHB31-.LFB2796
	.uleb128 .LEHE31-.LEHB31
	.uleb128 .L332-.LFB2796
	.uleb128 0
	.uleb128 .LEHB32-.LFB2796
	.uleb128 .LEHE32-.LEHB32
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB33-.LFB2796
	.uleb128 .LEHE33-.LEHB33
	.uleb128 .L331-.LFB2796
	.uleb128 0x1
	.uleb128 .LEHB34-.LFB2796
	.uleb128 .LEHE34-.LEHB34
	.uleb128 .L332-.LFB2796
	.uleb128 0
	.uleb128 .LEHB35-.LFB2796
	.uleb128 .LEHE35-.LEHB35
	.uleb128 .L330-.LFB2796
	.uleb128 0x1
.LLSDACSE2796:
	.byte	0x1
	.byte	0
	.align 4
	.long	0

.LLSDATT2796:
	.text
	.seh_endproc
	.align 2
	.p2align 4
	.globl	_ZN8Position12set_startposEv
	.def	_ZN8Position12set_startposEv;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN8Position12set_startposEv
_ZN8Position12set_startposEv:
.LFB2818:
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$80, %rsp
	.seh_stackalloc	80
	.seh_endprologue
	xorl	%r8d, %r8d
	movq	%rcx, %rbx
	leaq	40(%rsp), %rdx
	leaq	48(%rsp), %rcx
	movq	$56, 40(%rsp)
	leaq	64(%rsp), %rdi
	movq	%rdi, 48(%rsp)
.LEHB36:
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERyy
.LEHE36:
	movq	40(%rsp), %rdx
	movabsq	$8245636181365255794, %r9
	movabsq	$8102099357864587311, %r10
	movabsq	$3539882221867397483, %rcx
	movq	%rax, 48(%rsp)
	movabsq	$3402521436835295088, %r8
	movabsq	$5857811436340923970, %r11
	movq	%rdx, 64(%rsp)
	movq	%r9, (%rax)
	movabsq	$5787213827046125368, %r9
	movq	%r10, 8(%rax)
	movabsq	$5427191930111873104, %r10
	movq	%rcx, 48(%rax)
	movq	%rbx, %rcx
	movq	%r8, 16(%rax)
	movq	%r9, 24(%rax)
	movq	%r10, 32(%rax)
	movq	%r11, 40(%rax)
	movq	48(%rsp), %rax
	movq	%rdx, 56(%rsp)
	movb	$0, (%rax,%rdx)
	leaq	48(%rsp), %rdx
.LEHB37:
	call	_ZN8Position12set_from_fenERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE
.LEHE37:
	movq	48(%rsp), %rcx
	cmpq	%rdi, %rcx
	je	.L400
	movq	64(%rsp), %rax
	leaq	1(%rax), %rdx
	call	_ZdlPvy
	nop
.L400:
	addq	$80, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	ret
.L403:
	movq	%rax, %rbx
	leaq	48(%rsp), %rcx
	call	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv
	movq	%rbx, %rcx
.LEHB38:
	call	_Unwind_Resume
	nop
.LEHE38:
	.seh_handler	__gxx_personality_seh0, @unwind, @except
	.seh_handlerdata
.LLSDA2818:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE2818-.LLSDACSB2818
.LLSDACSB2818:
	.uleb128 .LEHB36-.LFB2818
	.uleb128 .LEHE36-.LEHB36
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB37-.LFB2818
	.uleb128 .LEHE37-.LEHB37
	.uleb128 .L403-.LFB2818
	.uleb128 0
	.uleb128 .LEHB38-.LFB2818
	.uleb128 .LEHE38-.LEHB38
	.uleb128 0
	.uleb128 0
.LLSDACSE2818:
	.text
	.seh_endproc
	.globl	_ZN7Zobrist6EpFileE
	.section	.data$_ZN7Zobrist6EpFileE,"w"
	.linkonce same_size
	.align 32
_ZN7Zobrist6EpFileE:
	.space 64
	.globl	_ZN7Zobrist6CastleE
	.section	.data$_ZN7Zobrist6CastleE,"w"
	.linkonce same_size
	.align 32
_ZN7Zobrist6CastleE:
	.space 128
	.globl	_ZN7Zobrist4SideE
	.section	.data$_ZN7Zobrist4SideE,"w"
	.linkonce same_size
	.align 8
_ZN7Zobrist4SideE:
	.space 8
	.globl	_ZN7Zobrist5PieceE
	.section	.data$_ZN7Zobrist5PieceE,"w"
	.linkonce same_size
	.align 32
_ZN7Zobrist5PieceE:
	.space 11520
	.globl	MAILBOX_MAPS
	.section	.rdata$MAILBOX_MAPS,"dr"
	.linkonce same_size
	.align 32
MAILBOX_MAPS:
	.long	21
	.long	22
	.long	23
	.long	24
	.long	25
	.long	26
	.long	27
	.long	28
	.long	31
	.long	32
	.long	33
	.long	34
	.long	35
	.long	36
	.long	37
	.long	38
	.long	41
	.long	42
	.long	43
	.long	44
	.long	45
	.long	46
	.long	47
	.long	48
	.long	51
	.long	52
	.long	53
	.long	54
	.long	55
	.long	56
	.long	57
	.long	58
	.long	61
	.long	62
	.long	63
	.long	64
	.long	65
	.long	66
	.long	67
	.long	68
	.long	71
	.long	72
	.long	73
	.long	74
	.long	75
	.long	76
	.long	77
	.long	78
	.long	81
	.long	82
	.long	83
	.long	84
	.long	85
	.long	86
	.long	87
	.long	88
	.long	91
	.long	92
	.long	93
	.long	94
	.long	95
	.long	96
	.long	97
	.long	98
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	0
	.long	1
	.long	2
	.long	3
	.long	4
	.long	5
	.long	6
	.long	7
	.long	-1
	.long	-1
	.long	8
	.long	9
	.long	10
	.long	11
	.long	12
	.long	13
	.long	14
	.long	15
	.long	-1
	.long	-1
	.long	16
	.long	17
	.long	18
	.long	19
	.long	20
	.long	21
	.long	22
	.long	23
	.long	-1
	.long	-1
	.long	24
	.long	25
	.long	26
	.long	27
	.long	28
	.long	29
	.long	30
	.long	31
	.long	-1
	.long	-1
	.long	32
	.long	33
	.long	34
	.long	35
	.long	36
	.long	37
	.long	38
	.long	39
	.long	-1
	.long	-1
	.long	40
	.long	41
	.long	42
	.long	43
	.long	44
	.long	45
	.long	46
	.long	47
	.long	-1
	.long	-1
	.long	48
	.long	49
	.long	50
	.long	51
	.long	52
	.long	53
	.long	54
	.long	55
	.long	-1
	.long	-1
	.long	56
	.long	57
	.long	58
	.long	59
	.long	60
	.long	61
	.long	62
	.long	63
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.globl	FILE_RANK_LOOKUPS
	.section	.rdata$FILE_RANK_LOOKUPS,"dr"
	.linkonce same_size
	.align 32
FILE_RANK_LOOKUPS:
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	0
	.byte	1
	.byte	2
	.byte	3
	.byte	4
	.byte	5
	.byte	6
	.byte	7
	.byte	-1
	.byte	-1
	.byte	0
	.byte	1
	.byte	2
	.byte	3
	.byte	4
	.byte	5
	.byte	6
	.byte	7
	.byte	-1
	.byte	-1
	.byte	0
	.byte	1
	.byte	2
	.byte	3
	.byte	4
	.byte	5
	.byte	6
	.byte	7
	.byte	-1
	.byte	-1
	.byte	0
	.byte	1
	.byte	2
	.byte	3
	.byte	4
	.byte	5
	.byte	6
	.byte	7
	.byte	-1
	.byte	-1
	.byte	0
	.byte	1
	.byte	2
	.byte	3
	.byte	4
	.byte	5
	.byte	6
	.byte	7
	.byte	-1
	.byte	-1
	.byte	0
	.byte	1
	.byte	2
	.byte	3
	.byte	4
	.byte	5
	.byte	6
	.byte	7
	.byte	-1
	.byte	-1
	.byte	0
	.byte	1
	.byte	2
	.byte	3
	.byte	4
	.byte	5
	.byte	6
	.byte	7
	.byte	-1
	.byte	-1
	.byte	0
	.byte	1
	.byte	2
	.byte	3
	.byte	4
	.byte	5
	.byte	6
	.byte	7
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	-1
	.byte	-1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	-1
	.byte	-1
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	-1
	.byte	-1
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	-1
	.byte	-1
	.byte	4
	.byte	4
	.byte	4
	.byte	4
	.byte	4
	.byte	4
	.byte	4
	.byte	4
	.byte	-1
	.byte	-1
	.byte	5
	.byte	5
	.byte	5
	.byte	5
	.byte	5
	.byte	5
	.byte	5
	.byte	5
	.byte	-1
	.byte	-1
	.byte	6
	.byte	6
	.byte	6
	.byte	6
	.byte	6
	.byte	6
	.byte	6
	.byte	6
	.byte	-1
	.byte	-1
	.byte	7
	.byte	7
	.byte	7
	.byte	7
	.byte	7
	.byte	7
	.byte	7
	.byte	7
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	0
	.byte	0
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	0
	.byte	0
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	0
	.byte	0
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	0
	.byte	0
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	0
	.byte	0
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	0
	.byte	0
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	0
	.byte	0
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	1
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.section .rdata,"dr"
	.align 16
_ZL15PIECE_VALUES_MG:
	.long	0
	.long	100
	.long	320
	.long	330
	.long	500
	.long	900
	.long	20000
	.align 8
.LC0:
	.long	-1
	.long	-1
	.def	__gxx_personality_seh0;	.scl	2;	.type	32;	.endef
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.1.0"
	.def	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7reserveEy;	.scl	2;	.type	32;	.endef
	.def	_ZN7Zobrist7computeERK8Position;	.scl	2;	.type	32;	.endef
	.def	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcy;	.scl	2;	.type	32;	.endef
	.def	_ZdlPvy;	.scl	2;	.type	32;	.endef
	.def	_Z7to_char5Piece;	.scl	2;	.type	32;	.endef
	.def	memcpy;	.scl	2;	.type	32;	.endef
	.def	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEyyPKcy;	.scl	2;	.type	32;	.endef
	.def	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERyy;	.scl	2;	.type	32;	.endef
	.def	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructEyc;	.scl	2;	.type	32;	.endef
	.def	_ZSt20__throw_length_errorPKc;	.scl	2;	.type	32;	.endef
	.def	_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv;	.scl	2;	.type	32;	.endef
	.def	_Unwind_Resume;	.scl	2;	.type	32;	.endef
	.def	_ZNSt8ios_baseC2Ev;	.scl	2;	.type	32;	.endef
	.def	_ZNSt9basic_iosIcSt11char_traitsIcEE4initEPSt15basic_streambufIcS1_E;	.scl	2;	.type	32;	.endef
	.def	_ZNSt6localeC1Ev;	.scl	2;	.type	32;	.endef
	.def	_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7_M_syncEPcyy;	.scl	2;	.type	32;	.endef
	.def	_ZStrsIcSt11char_traitsIcESaIcEERSt13basic_istreamIT_T0_ES7_RNSt7__cxx1112basic_stringIS4_S5_T1_EE;	.scl	2;	.type	32;	.endef
	.def	_Z9from_charc;	.scl	2;	.type	32;	.endef
	.def	_ZNSt6localeD1Ev;	.scl	2;	.type	32;	.endef
	.def	_ZNSt8ios_baseD2Ev;	.scl	2;	.type	32;	.endef
	.def	_Znwy;	.scl	2;	.type	32;	.endef
	.def	strtol;	.scl	2;	.type	32;	.endef
	.def	_ZSt19__throw_logic_errorPKc;	.scl	2;	.type	32;	.endef
	.def	_ZNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEED1Ev;	.scl	2;	.type	32;	.endef
	.def	_ZSt24__throw_invalid_argumentPKc;	.scl	2;	.type	32;	.endef
	.def	_ZSt20__throw_out_of_rangePKc;	.scl	2;	.type	32;	.endef
	.def	__cxa_begin_catch;	.scl	2;	.type	32;	.endef
	.def	__cxa_end_catch;	.scl	2;	.type	32;	.endef
	.section	.rdata$.refptr._ZTVNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEEE, "dr"
	.p2align	3, 0
	.globl	.refptr._ZTVNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEEE
	.linkonce	discard
.refptr._ZTVNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEEE:
	.quad	_ZTVNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEEE
	.section	.rdata$.refptr._ZTVNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE, "dr"
	.p2align	3, 0
	.globl	.refptr._ZTVNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE
	.linkonce	discard
.refptr._ZTVNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE:
	.quad	_ZTVNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE
	.section	.rdata$.refptr._ZTTNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE, "dr"
	.p2align	3, 0
	.globl	.refptr._ZTTNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE
	.linkonce	discard
.refptr._ZTTNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE:
	.quad	_ZTTNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEE
	.section	.rdata$.refptr._ZTVSt9basic_iosIcSt11char_traitsIcEE, "dr"
	.p2align	3, 0
	.globl	.refptr._ZTVSt9basic_iosIcSt11char_traitsIcEE
	.linkonce	discard
.refptr._ZTVSt9basic_iosIcSt11char_traitsIcEE:
	.quad	_ZTVSt9basic_iosIcSt11char_traitsIcEE
	.section	.rdata$.refptr._ZTVSt15basic_streambufIcSt11char_traitsIcEE, "dr"
	.p2align	3, 0
	.globl	.refptr._ZTVSt15basic_streambufIcSt11char_traitsIcEE
	.linkonce	discard
.refptr._ZTVSt15basic_streambufIcSt11char_traitsIcEE:
	.quad	_ZTVSt15basic_streambufIcSt11char_traitsIcEE
