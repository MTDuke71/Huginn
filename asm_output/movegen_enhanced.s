	.file	"movegen_enhanced.cpp"
	.text
	.p2align 4
	.def	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.0;	.scl	3;	.type	32;	.endef
	.seh_proc	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.0
_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.0:
.LFB3937:
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
	subq	$104, %rsp
	.seh_stackalloc	104
	.seh_endprologue
	movzbl	%r8b, %eax
	movq	%rdx, %r15
	movl	%r8d, 88(%rsp)
	movq	%rcx, %rdi
	imulq	$28, %rax, %rdx
	movslq	796(%rcx,%rdx), %rdx
	testl	%edx, %edx
	jle	.L1
	cmpb	$1, %r8b
	leaq	240+FILE_RANK_LOOKUPS(%rip), %rsi
	sete	95(%rsp)
	imulq	$280, %rax, %rbx
	imulq	$70, %rax, %rax
	leaq	(%rbx,%rcx), %rbp
	addq	%rdx, %rax
	leaq	(%rcx,%rax,4), %r13
	movq	%r13, 80(%rsp)
	.p2align 4
	.p2align 3
.L18:
	movl	416(%rbp), %r14d
	cmpl	$-1, %r14d
	je	.L3
	movq	%rbp, 72(%rsp)
	leaq	32+_ZZ20generate_queen_movesRK8PositionR10S_MOVELIST5ColorE10directions(%rip), %r12
	leaq	_ZZ20generate_queen_movesRK8PositionR10S_MOVELIST5ColorE10directions(%rip), %rbp
	.p2align 4
	.p2align 3
.L17:
	movslq	0(%rbp), %r13
	leal	(%r14,%r13), %eax
	movslq	%eax, %rbx
	cmpl	$119, %eax
	jbe	.L6
	jmp	.L14
	.p2align 4,,10
	.p2align 3
.L42:
	movzbl	(%rdi,%rbx), %r8d
	movl	%ebx, %edx
	testb	%r8b, %r8b
	jne	.L41
	movb	$0, 40(%rsp)
	movl	%r14d, %ecx
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movl	$0, 48(%rsp)
	addq	%r13, %rbx
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%r15), %rdx
	movl	%eax, %ecx
	movq	%rdx, %rax
	movl	%ecx, (%r15,%rdx,8)
	movl	$0, 4(%r15,%rdx,8)
	addl	$1, %eax
	movl	%eax, 2048(%r15)
	cmpl	$119, %ebx
	ja	.L14
.L6:
	cmpb	$0, (%rsi,%rbx)
	jne	.L42
.L14:
	addq	$4, %rbp
	cmpq	%rbp, %r12
	jne	.L17
.L44:
	movq	72(%rsp), %rbp
.L3:
	addq	$4, %rbp
	cmpq	%rbp, 80(%rsp)
	jne	.L18
.L1:
	addq	$104, %rsp
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
.L41:
	cmpb	$-1, %r8b
	je	.L43
	movl	%r8d, %eax
	shrb	$3, %al
	cmpb	$0, 88(%rsp)
	jne	.L9
	testb	$1, %al
	je	.L14
.L10:
	andl	$7, %r8d
.L8:
	movl	$0, 48(%rsp)
	xorl	%r9d, %r9d
	movl	%r14d, %ecx
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%r15), %r8
	movzbl	120(%rdi), %r9d
	movl	%eax, (%r15,%r8,8)
	movq	%r8, %rdx
	andl	$0, 4(%r15,%r8,8)
	testb	%r9b, %r9b
	je	.L11
	movl	$1000000, %ecx
	cmpb	$1, %r9b
	jne	.L12
.L11:
	movl	%eax, %ecx
	sarl	$14, %ecx
	andl	$15, %ecx
	je	.L22
	andl	$7, %ecx
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rbx
	movl	(%rbx,%rcx,4), %ecx
	leal	(%rcx,%rcx,4), %ecx
	leal	1000000(%rcx,%rcx), %ecx
.L12:
	andl	$127, %eax
	cmpl	$119, %eax
	jg	.L13
	movzbl	(%rdi,%rax), %eax
	leal	-1(%rax), %r9d
	cmpb	$-3, %r9b
	ja	.L13
	andl	$7, %eax
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rbx
	subl	(%rbx,%rax,4), %ecx
.L13:
	addl	$1, %edx
	addq	$4, %rbp
	movl	%ecx, 4(%r15,%r8,8)
	movl	%edx, 2048(%r15)
	cmpq	%rbp, %r12
	jne	.L17
	jmp	.L44
	.p2align 4,,10
	.p2align 3
.L9:
	testb	$1, %al
	jne	.L14
	cmpb	$0, 95(%rsp)
	jne	.L10
	addq	$4, %rbp
	cmpq	%rbp, %r12
	jne	.L17
	jmp	.L44
	.p2align 4,,10
	.p2align 3
.L43:
	xorl	%r8d, %r8d
	cmpb	$1, 88(%rsp)
	ja	.L8
	addq	$4, %rbp
	cmpq	%rbp, %r12
	jne	.L17
	jmp	.L44
.L22:
	movl	$1000000, %ecx
	jmp	.L12
	.seh_endproc
	.p2align 4
	.def	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.1;	.scl	3;	.type	32;	.endef
	.seh_proc	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.1
_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.1:
.LFB3938:
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
	subq	$104, %rsp
	.seh_stackalloc	104
	.seh_endprologue
	movq	208(%rsp), %r13
	movzbl	%r8b, %eax
	movq	%rcx, %rdi
	movq	%rdx, %r15
	movzbl	%r9b, %edx
	leaq	0(,%rax,8), %rcx
	movl	%r8d, 88(%rsp)
	subq	%rax, %rcx
	addq	%rdx, %rcx
	movslq	776(%rdi,%rcx,4), %rcx
	testl	%ecx, %ecx
	jle	.L45
	cmpb	$1, %r8b
	leaq	(%rdx,%rdx,4), %rdx
	leaq	16(%r13), %rbp
	sete	95(%rsp)
	imulq	$280, %rax, %rax
	leaq	(%rdi,%rdx,8), %rbx
	leaq	240+FILE_RANK_LOOKUPS(%rip), %rsi
	addq	%rax, %rbx
	leaq	(%rbx,%rcx,4), %rax
	movq	%rbx, %r12
	movq	%rax, 80(%rsp)
	.p2align 4
	.p2align 3
.L48:
	movl	216(%r12), %ebx
	movq	208(%rsp), %r14
	cmpl	$-1, %ebx
	je	.L64
	movq	%r12, 72(%rsp)
	movq	%r14, %r12
	movl	%ebx, %r14d
	.p2align 4
	.p2align 3
.L63:
	movslq	(%r12), %r13
	leal	(%r14,%r13), %eax
	movslq	%eax, %rbx
	cmpl	$119, %eax
	jbe	.L52
	jmp	.L60
	.p2align 4,,10
	.p2align 3
.L85:
	movzbl	(%rdi,%rbx), %r8d
	movl	%ebx, %edx
	testb	%r8b, %r8b
	jne	.L84
	movb	$0, 40(%rsp)
	movl	%r14d, %ecx
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movl	$0, 48(%rsp)
	addq	%r13, %rbx
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%r15), %rdx
	movl	%eax, %ecx
	movq	%rdx, %rax
	movl	%ecx, (%r15,%rdx,8)
	movl	$0, 4(%r15,%rdx,8)
	addl	$1, %eax
	movl	%eax, 2048(%r15)
	cmpl	$119, %ebx
	ja	.L60
.L52:
	cmpb	$0, (%rsi,%rbx)
	jne	.L85
.L60:
	addq	$4, %r12
	cmpq	%r12, %rbp
	jne	.L63
.L87:
	movq	72(%rsp), %r12
.L64:
	addq	$4, %r12
	cmpq	%r12, 80(%rsp)
	jne	.L48
.L45:
	addq	$104, %rsp
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
.L84:
	cmpb	$-1, %r8b
	je	.L86
	movl	%r8d, %eax
	shrb	$3, %al
	cmpb	$0, 88(%rsp)
	jne	.L55
	testb	$1, %al
	je	.L60
.L56:
	andl	$7, %r8d
.L54:
	movl	$0, 48(%rsp)
	xorl	%r9d, %r9d
	movl	%r14d, %ecx
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%r15), %r8
	movzbl	120(%rdi), %r9d
	movl	%eax, (%r15,%r8,8)
	movq	%r8, %rdx
	andl	$0, 4(%r15,%r8,8)
	testb	%r9b, %r9b
	je	.L57
	movl	$1000000, %ecx
	cmpb	$1, %r9b
	jne	.L58
.L57:
	movl	%eax, %ecx
	sarl	$14, %ecx
	andl	$15, %ecx
	je	.L67
	andl	$7, %ecx
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rbx
	movl	(%rbx,%rcx,4), %ecx
	leal	(%rcx,%rcx,4), %ecx
	leal	1000000(%rcx,%rcx), %ecx
.L58:
	andl	$127, %eax
	cmpl	$119, %eax
	jg	.L59
	movzbl	(%rdi,%rax), %eax
	leal	-1(%rax), %r9d
	cmpb	$-3, %r9b
	ja	.L59
	andl	$7, %eax
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rbx
	subl	(%rbx,%rax,4), %ecx
.L59:
	addl	$1, %edx
	addq	$4, %r12
	movl	%ecx, 4(%r15,%r8,8)
	movl	%edx, 2048(%r15)
	cmpq	%r12, %rbp
	jne	.L63
	jmp	.L87
	.p2align 4,,10
	.p2align 3
.L55:
	testb	$1, %al
	jne	.L60
	cmpb	$0, 95(%rsp)
	jne	.L56
	addq	$4, %r12
	cmpq	%r12, %rbp
	jne	.L63
	jmp	.L87
	.p2align 4,,10
	.p2align 3
.L86:
	xorl	%r8d, %r8d
	cmpb	$1, 88(%rsp)
	ja	.L54
	addq	$4, %r12
	cmpq	%r12, %rbp
	jne	.L63
	jmp	.L87
.L67:
	movl	$1000000, %ecx
	jmp	.L58
	.seh_endproc
	.p2align 4
	.globl	_Z19generate_pawn_movesRK8PositionR10S_MOVELIST5Color
	.def	_Z19generate_pawn_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z19generate_pawn_movesRK8PositionR10S_MOVELIST5Color
_Z19generate_pawn_movesRK8PositionR10S_MOVELIST5Color:
.LFB3075:
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
	subq	$104, %rsp
	.seh_stackalloc	104
	.seh_endprologue
	movzbl	%r8b, %eax
	movq	%rdx, %rdi
	imulq	$28, %rax, %rdx
	movslq	780(%rcx,%rdx), %rdx
	testl	%edx, %edx
	jle	.L88
	imulq	$280, %rax, %rsi
	leaq	FILE_RANK_LOOKUPS(%rip), %r14
	movq	%rcx, %r10
	movl	%r8d, %ebp
	imulq	$70, %rax, %rax
	addq	%rcx, %rsi
	addq	%rdx, %rax
	leaq	(%rcx,%rax,4), %r15
	.p2align 4
	.p2align 3
.L143:
	movl	256(%rsi), %ecx
	cmpl	$-1, %ecx
	je	.L90
	testb	%bpl, %bpl
	jne	.L145
	movb	$7, 78(%rsp)
	movl	$11, %r12d
	movl	$10, %ebx
	movl	$9, %r13d
	movl	$1, %r11d
.L91:
	leal	(%rcx,%rbx), %edx
	cmpl	$119, %edx
	ja	.L93
	movslq	%edx, %rax
	cmpb	$0, 240(%r14,%rax)
	je	.L93
	cmpb	$0, (%r10,%rax)
	jne	.L93
	movq	%r10, 176(%rsp)
	movzbl	78(%rsp), %r9d
	cmpb	120(%r14,%rax), %r9b
	je	.L181
	movl	$0, 48(%rsp)
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	movl	%ecx, 68(%rsp)
	movb	%r11b, 79(%rsp)
	movl	%edx, 72(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r8
	movl	68(%rsp), %ecx
	movl	%eax, %r9d
	movq	176(%rsp), %r10
	movq	%r8, %rax
	movl	%r9d, (%rdi,%r8,8)
	movl	$0, 4(%rdi,%r8,8)
	addl	$1, %eax
	cmpl	$119, %ecx
	movl	%eax, 2048(%rdi)
	ja	.L93
	movslq	%ecx, %rax
	movzbl	79(%rsp), %r11d
	movl	72(%rsp), %edx
	cmpb	%r11b, 120(%r14,%rax)
	je	.L182
	.p2align 4
	.p2align 3
.L93:
	movl	%r12d, 92(%rsp)
	leaq	88(%rsp), %rbx
	movl	%r13d, 88(%rsp)
	movl	%ecx, %r13d
.L142:
	movl	(%rbx), %r12d
	addl	%r13d, %r12d
	cmpl	$119, %r12d
	ja	.L112
	movslq	%r12d, %rax
	cmpb	$0, 240(%r14,%rax)
	je	.L112
	movzbl	(%r10,%rax), %r8d
	testb	%r8b, %r8b
	je	.L113
	cmpb	$-1, %r8b
	je	.L183
	movl	%r8d, %edx
	shrb	$3, %dl
	testb	%bpl, %bpl
	je	.L184
	andl	$1, %edx
	jne	.L113
	cmpb	$1, %bpl
	je	.L120
	.p2align 4
	.p2align 3
.L113:
	cmpl	%r12d, 124(%r10)
	je	.L185
	.p2align 4
	.p2align 3
.L112:
	addq	$4, %rbx
	leaq	96(%rsp), %rax
	cmpq	%rax, %rbx
	jne	.L142
.L90:
	addq	$4, %rsi
	cmpq	%r15, %rsi
	jne	.L143
.L88:
	addq	$104, %rsp
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
.L145:
	movb	$0, 78(%rsp)
	movl	$-9, %r12d
	movl	$-10, %ebx
	movl	$-11, %r13d
	movl	$6, %r11d
	jmp	.L91
	.p2align 4,,10
	.p2align 3
.L184:
	andl	$1, %edx
	je	.L113
.L120:
	movzbl	78(%rsp), %ecx
	andl	$7, %r8d
	cmpb	120(%r14,%rax), %cl
	je	.L186
.L116:
	movb	$0, 40(%rsp)
	xorl	%r9d, %r9d
	movl	%r12d, %edx
	movl	%r13d, %ecx
	movl	$0, 48(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 176(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movq	176(%rsp), %r10
	movslq	2048(%rdi), %r9
	movzbl	120(%r10), %r11d
	movl	%eax, (%rdi,%r9,8)
	movq	%r9, %rdx
	andl	$0, 4(%rdi,%r9,8)
	testb	%r11b, %r11b
	je	.L139
	movl	$1000000, %r8d
	cmpb	$1, %r11b
	jne	.L140
.L139:
	movl	%eax, %r8d
	sarl	$14, %r8d
	andl	$15, %r8d
	je	.L157
	andl	$7, %r8d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r11
	movl	(%r11,%r8,4), %r8d
	leal	(%r8,%r8,4), %r8d
	leal	1000000(%r8,%r8), %r8d
.L140:
	andl	$127, %eax
	cmpl	$119, %eax
	jg	.L141
	movzbl	(%r10,%rax), %eax
	leal	-1(%rax), %r11d
	cmpb	$-3, %r11b
	ja	.L141
	andl	$7, %eax
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r11
	subl	(%r11,%rax,4), %r8d
.L141:
	addl	$1, %edx
	addq	$4, %rbx
	leaq	96(%rsp), %rax
	movl	%r8d, 4(%rdi,%r9,8)
	movl	%edx, 2048(%rdi)
	cmpq	%rax, %rbx
	jne	.L142
	jmp	.L90
	.p2align 4,,10
	.p2align 3
.L185:
	movb	$0, 40(%rsp)
	movl	%r12d, %edx
	movl	%r13d, %ecx
	addq	$4, %rbx
	movl	$0, 48(%rsp)
	movl	$1, %r8d
	movl	$1, %r9d
	movl	$0, 32(%rsp)
	movq	%r10, 176(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %rdx
	movq	176(%rsp), %r10
	movl	%eax, %r8d
	movq	%rdx, %rax
	movl	%r8d, (%rdi,%rdx,8)
	addl	$1, %eax
	movl	$1000105, 4(%rdi,%rdx,8)
	movl	%eax, 2048(%rdi)
	leaq	96(%rsp), %rax
	cmpq	%rax, %rbx
	jne	.L142
	jmp	.L90
	.p2align 4,,10
	.p2align 3
.L181:
	movl	$0, 48(%rsp)
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movb	$5, 40(%rsp)
	movl	$0, 32(%rsp)
	movl	%edx, 72(%rsp)
	movl	%ecx, 68(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r9
	movl	68(%rsp), %ecx
	movl	$2000000, %r8d
	movl	%eax, %r11d
	movl	72(%rsp), %edx
	movq	176(%rsp), %r10
	andl	$0, 4(%rdi,%r9,8)
	sarl	$20, %r11d
	testl	$15728640, %eax
	movq	%r9, %rbx
	movl	%eax, (%rdi,%r9,8)
	je	.L96
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r8
	imull	$100, (%r8,%r11,4), %r8d
	addl	$2000000, %r8d
.L96:
	movl	%eax, %r11d
	sarl	$14, %r11d
	testl	$245760, %eax
	je	.L99
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rax
	movl	(%rax,%r11,4), %eax
	leal	(%rax,%rax,4), %eax
	leal	(%r8,%rax,2), %r8d
.L99:
	addl	$1, %ebx
	movl	%r8d, 4(%rdi,%r9,8)
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movl	%ebx, 2048(%rdi)
	movl	$0, 48(%rsp)
	movb	$4, 40(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 176(%rsp)
	movl	%edx, 72(%rsp)
	movl	%ecx, 68(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r9
	movl	68(%rsp), %ecx
	movl	$2000000, %r8d
	movl	%eax, %r11d
	movl	72(%rsp), %edx
	movq	176(%rsp), %r10
	andl	$0, 4(%rdi,%r9,8)
	sarl	$20, %r11d
	testl	$15728640, %eax
	movq	%r9, %rbx
	movl	%eax, (%rdi,%r9,8)
	je	.L100
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r8
	imull	$100, (%r8,%r11,4), %r8d
	addl	$2000000, %r8d
.L100:
	movl	%eax, %r11d
	sarl	$14, %r11d
	testl	$245760, %eax
	je	.L103
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rax
	movl	(%rax,%r11,4), %eax
	leal	(%rax,%rax,4), %eax
	leal	(%r8,%rax,2), %r8d
.L103:
	addl	$1, %ebx
	movl	%r8d, 4(%rdi,%r9,8)
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movl	%ebx, 2048(%rdi)
	movl	$0, 48(%rsp)
	movb	$3, 40(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 176(%rsp)
	movl	%edx, 72(%rsp)
	movl	%ecx, 68(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r9
	movl	68(%rsp), %ecx
	movl	$2000000, %r8d
	movl	%eax, %r11d
	movl	72(%rsp), %edx
	movq	176(%rsp), %r10
	andl	$0, 4(%rdi,%r9,8)
	sarl	$20, %r11d
	testl	$15728640, %eax
	movq	%r9, %rbx
	movl	%eax, (%rdi,%r9,8)
	je	.L104
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r8
	imull	$100, (%r8,%r11,4), %r8d
	addl	$2000000, %r8d
.L104:
	movl	%eax, %r11d
	sarl	$14, %r11d
	testl	$245760, %eax
	je	.L107
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rax
	movl	(%rax,%r11,4), %eax
	leal	(%rax,%rax,4), %eax
	leal	(%r8,%rax,2), %r8d
.L107:
	addl	$1, %ebx
	movl	%r8d, 4(%rdi,%r9,8)
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movl	%ebx, 2048(%rdi)
	movl	$0, 48(%rsp)
	movb	$2, 40(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 176(%rsp)
	movl	%ecx, 68(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r9
	movl	68(%rsp), %ecx
	movl	$2000000, %r8d
	movl	%eax, %r11d
	movq	176(%rsp), %r10
	andl	$0, 4(%rdi,%r9,8)
	sarl	$20, %r11d
	testl	$15728640, %eax
	movq	%r9, %rdx
	movl	%eax, (%rdi,%r9,8)
	je	.L108
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r8
	imull	$100, (%r8,%r11,4), %r8d
	addl	$2000000, %r8d
.L108:
	movl	%eax, %r11d
	sarl	$14, %r11d
	testl	$245760, %eax
	je	.L111
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rax
	movl	(%rax,%r11,4), %eax
	leal	(%rax,%rax,4), %eax
	leal	(%r8,%rax,2), %r8d
.L111:
	addl	$1, %edx
	movl	%r8d, 4(%rdi,%r9,8)
	movl	%edx, 2048(%rdi)
	jmp	.L93
	.p2align 4,,10
	.p2align 3
.L183:
	cmpb	$1, %bpl
	jbe	.L113
	movzbl	78(%rsp), %ecx
	xorl	%r8d, %r8d
	cmpb	120(%r14,%rax), %cl
	jne	.L116
	movb	$0, 68(%rsp)
	movzbl	68(%rsp), %r8d
	jmp	.L115
.L182:
	addl	%ebx, %edx
	cmpl	$119, %edx
	ja	.L93
	movslq	%edx, %rax
	cmpb	$0, 240(%r14,%rax)
	je	.L93
	cmpb	$0, (%r10,%rax)
	jne	.L93
	movl	$0, 48(%rsp)
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	movb	$0, 40(%rsp)
	movl	$1, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %rdx
	movl	68(%rsp), %ecx
	movl	%eax, %r8d
	movq	176(%rsp), %r10
	movq	%rdx, %rax
	movl	%r8d, (%rdi,%rdx,8)
	addl	$1, %eax
	movl	$0, 4(%rdi,%rdx,8)
	movl	%eax, 2048(%rdi)
	jmp	.L93
	.p2align 4,,10
	.p2align 3
.L186:
	movb	%r8b, 68(%rsp)
.L115:
	movb	$5, 40(%rsp)
	xorl	%r9d, %r9d
	movl	%r12d, %edx
	movl	%r13d, %ecx
	movl	$0, 48(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 176(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r9
	movq	176(%rsp), %r10
	movl	$2000000, %r8d
	movl	%eax, %r11d
	andl	$0, 4(%rdi,%r9,8)
	sarl	$20, %r11d
	testl	$15728640, %eax
	movq	%r9, %rdx
	movl	%eax, (%rdi,%r9,8)
	je	.L123
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r8
	imull	$100, (%r8,%r11,4), %r8d
	addl	$2000000, %r8d
.L123:
	movl	%eax, %r11d
	sarl	$14, %r11d
	testl	$245760, %eax
	je	.L126
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rax
	movl	(%rax,%r11,4), %eax
	leal	(%rax,%rax,4), %eax
	leal	(%r8,%rax,2), %r8d
.L126:
	addl	$1, %edx
	movl	%r8d, 4(%rdi,%r9,8)
	movzbl	68(%rsp), %r8d
	xorl	%r9d, %r9d
	movl	%edx, 2048(%rdi)
	movl	%r13d, %ecx
	movl	%r12d, %edx
	movl	$0, 48(%rsp)
	movb	$4, 40(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 176(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r9
	movq	176(%rsp), %r10
	movl	$2000000, %r8d
	movl	%eax, %r11d
	andl	$0, 4(%rdi,%r9,8)
	sarl	$20, %r11d
	testl	$15728640, %eax
	movq	%r9, %rdx
	movl	%eax, (%rdi,%r9,8)
	je	.L127
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r8
	imull	$100, (%r8,%r11,4), %r8d
	addl	$2000000, %r8d
.L127:
	movl	%eax, %r11d
	sarl	$14, %r11d
	testl	$245760, %eax
	je	.L130
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rax
	movl	(%rax,%r11,4), %eax
	leal	(%rax,%rax,4), %eax
	leal	(%r8,%rax,2), %r8d
.L130:
	addl	$1, %edx
	movl	%r8d, 4(%rdi,%r9,8)
	movzbl	68(%rsp), %r8d
	xorl	%r9d, %r9d
	movl	%edx, 2048(%rdi)
	movl	%r13d, %ecx
	movl	%r12d, %edx
	movl	$0, 48(%rsp)
	movb	$3, 40(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 176(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r9
	movq	176(%rsp), %r10
	movl	$2000000, %r8d
	movl	%eax, %r11d
	andl	$0, 4(%rdi,%r9,8)
	sarl	$20, %r11d
	testl	$15728640, %eax
	movq	%r9, %rdx
	movl	%eax, (%rdi,%r9,8)
	je	.L131
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r8
	imull	$100, (%r8,%r11,4), %r8d
	addl	$2000000, %r8d
.L131:
	movl	%eax, %r11d
	sarl	$14, %r11d
	testl	$245760, %eax
	je	.L134
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rax
	movl	(%rax,%r11,4), %eax
	leal	(%rax,%rax,4), %eax
	leal	(%r8,%rax,2), %r8d
.L134:
	addl	$1, %edx
	movl	%r8d, 4(%rdi,%r9,8)
	movzbl	68(%rsp), %r8d
	xorl	%r9d, %r9d
	movl	%edx, 2048(%rdi)
	movl	%r13d, %ecx
	movl	%r12d, %edx
	movl	$0, 48(%rsp)
	movb	$2, 40(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 176(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r9
	movq	176(%rsp), %r10
	movl	$2000000, %r8d
	movl	%eax, %r11d
	andl	$0, 4(%rdi,%r9,8)
	sarl	$20, %r11d
	testl	$15728640, %eax
	movq	%r9, %rdx
	movl	%eax, (%rdi,%r9,8)
	je	.L135
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r8
	imull	$100, (%r8,%r11,4), %r8d
	addl	$2000000, %r8d
.L135:
	movl	%eax, %r11d
	sarl	$14, %r11d
	testl	$245760, %eax
	je	.L141
	andl	$7, %r11d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rax
	movl	(%rax,%r11,4), %eax
	leal	(%rax,%rax,4), %eax
	leal	(%r8,%rax,2), %r8d
	jmp	.L141
.L157:
	movl	$1000000, %r8d
	jmp	.L140
	.seh_endproc
	.p2align 4
	.globl	_Z21generate_knight_movesRK8PositionR10S_MOVELIST5Color
	.def	_Z21generate_knight_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z21generate_knight_movesRK8PositionR10S_MOVELIST5Color
_Z21generate_knight_movesRK8PositionR10S_MOVELIST5Color:
.LFB3076:
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
	subq	$88, %rsp
	.seh_stackalloc	88
	.seh_endprologue
	movzbl	%r8b, %eax
	movq	%rdx, %rdi
	imulq	$28, %rax, %rdx
	movslq	784(%rcx,%rdx), %rdx
	testl	%edx, %edx
	jle	.L187
	testb	%r8b, %r8b
	je	.L241
	imulq	$280, %rax, %r13
	movl	%r8d, 76(%rsp)
	leaq	FILE_RANK_LOOKUPS(%rip), %r15
	imulq	$70, %rax, %rax
	movq	%rdi, 168(%rsp)
	movq	%rcx, 160(%rsp)
	movq	160(%rsp), %r14
	addq	%rcx, %r13
	addq	%rdx, %rax
	movq	%r13, %rdi
	cmpb	$1, %r8b
	movq	168(%rsp), %r13
	sete	72(%rsp)
	leaq	(%rcx,%rax,4), %r12
	.p2align 4
	.p2align 3
.L212:
	movl	296(%rdi), %esi
	cmpl	$-1, %esi
	je	.L203
	leaq	KNIGHT_DELTAS(%rip), %rbx
	leaq	32(%rbx), %rbp
	.p2align 4
	.p2align 3
.L211:
	movl	(%rbx), %edx
	addl	%esi, %edx
	cmpl	$119, %edx
	ja	.L204
	movslq	%edx, %rax
	cmpb	$0, 240(%r15,%rax)
	je	.L204
	movzbl	(%r14,%rax), %r8d
	testb	%r8b, %r8b
	je	.L242
	cmpb	$-1, %r8b
	je	.L243
	testb	$8, %r8b
	jne	.L204
	cmpb	$0, 72(%rsp)
	jne	.L244
	.p2align 4
	.p2align 3
.L204:
	addq	$4, %rbx
	cmpq	%rbp, %rbx
	jne	.L211
.L203:
	addq	$4, %rdi
	cmpq	%r12, %rdi
	jne	.L212
.L187:
	addq	$88, %rsp
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
.L241:
	movl	%r8d, 72(%rsp)
	movq	%rcx, %r14
	leaq	(%rcx,%rdx,4), %rbp
	leaq	FILE_RANK_LOOKUPS(%rip), %r15
	movq	%rdi, 168(%rsp)
	movq	168(%rsp), %r12
	movq	%rcx, %rdi
	.p2align 4
	.p2align 3
.L201:
	movl	296(%r14), %esi
	cmpl	$-1, %esi
	je	.L190
	leaq	KNIGHT_DELTAS(%rip), %rbx
	leaq	32(%rbx), %r13
	.p2align 4
	.p2align 3
.L192:
	movl	(%rbx), %edx
	addl	%esi, %edx
	cmpl	$119, %edx
	ja	.L193
	movslq	%edx, %rax
	cmpb	$0, 240(%r15,%rax)
	je	.L193
	movzbl	(%rdi,%rax), %r8d
	testb	%r8b, %r8b
	je	.L194
	cmpb	$-1, %r8b
	je	.L193
	testb	$8, %r8b
	jne	.L245
	.p2align 4
	.p2align 3
.L193:
	addq	$4, %rbx
	cmpq	%rbx, %r13
	jne	.L192
.L190:
	addq	$4, %r14
	cmpq	%r14, %rbp
	jne	.L201
	jmp	.L187
	.p2align 4,,10
	.p2align 3
.L242:
	movl	$0, 48(%rsp)
	movl	%esi, %ecx
	xorl	%r9d, %r9d
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%r13), %rdx
	movl	%eax, %ecx
	movq	%rdx, %rax
	movl	%ecx, 0(%r13,%rdx,8)
	movl	$0, 4(%r13,%rdx,8)
	addl	$1, %eax
	movl	%eax, 2048(%r13)
	jmp	.L204
	.p2align 4,,10
	.p2align 3
.L194:
	movl	$0, 48(%rsp)
	movl	%esi, %ecx
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%r12), %rdx
	movl	%eax, %ecx
	movq	%rdx, %rax
	movl	%ecx, (%r12,%rdx,8)
	movl	$0, 4(%r12,%rdx,8)
	addl	$1, %eax
	movl	%eax, 2048(%r12)
	jmp	.L193
	.p2align 4,,10
	.p2align 3
.L245:
	movb	$0, 40(%rsp)
	xorl	%r9d, %r9d
	andl	$7, %r8d
	movl	%esi, %ecx
	movl	$0, 48(%rsp)
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movzbl	120(%rdi), %edx
	movslq	2048(%r12), %r8
	movl	$8, %r9d
	movl	%eax, (%r12,%r8,8)
	movq	%r8, %rcx
	andl	$0, 4(%r12,%r8,8)
	testb	%dl, %dl
	je	.L199
	cmpb	$1, %dl
	jne	.L214
	movzbl	72(%rsp), %r9d
.L199:
	movl	%eax, %edx
	sarl	$14, %edx
	andl	$15, %edx
	je	.L214
	orl	%r9d, %edx
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r10
	andl	$7, %edx
	movl	(%r10,%rdx,4), %edx
	leal	(%rdx,%rdx,4), %edx
	leal	1000000(%rdx,%rdx), %edx
.L198:
	andl	$127, %eax
	cmpl	$119, %eax
	jg	.L197
	movzbl	(%rdi,%rax), %eax
	leal	-1(%rax), %r9d
	cmpb	$-3, %r9b
	ja	.L197
	andl	$7, %eax
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r9
	subl	(%r9,%rax,4), %edx
.L197:
	addl	$1, %ecx
	movl	%edx, 4(%r12,%r8,8)
	movl	%ecx, 2048(%r12)
	jmp	.L193
	.p2align 4,,10
	.p2align 3
.L244:
	andl	$7, %r8d
.L207:
	movl	$0, 48(%rsp)
	xorl	%r9d, %r9d
	movl	%esi, %ecx
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%r13), %r8
	movzbl	120(%r14), %r9d
	movl	%eax, 0(%r13,%r8,8)
	movq	%r8, %rdx
	andl	$0, 4(%r13,%r8,8)
	testb	%r9b, %r9b
	je	.L208
	movl	$1000000, %ecx
	cmpb	$1, %r9b
	jne	.L209
.L208:
	movl	%eax, %ecx
	sarl	$14, %ecx
	andl	$15, %ecx
	je	.L218
	andl	$7, %ecx
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r11
	movl	(%r11,%rcx,4), %ecx
	leal	(%rcx,%rcx,4), %ecx
	leal	1000000(%rcx,%rcx), %ecx
.L209:
	andl	$127, %eax
	cmpl	$119, %eax
	jg	.L210
	movzbl	(%r14,%rax), %eax
	leal	-1(%rax), %r9d
	cmpb	$-3, %r9b
	ja	.L210
	andl	$7, %eax
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r9
	subl	(%r9,%rax,4), %ecx
.L210:
	addl	$1, %edx
	movl	%ecx, 4(%r13,%r8,8)
	movl	%edx, 2048(%r13)
	jmp	.L204
	.p2align 4,,10
	.p2align 3
.L243:
	xorl	%r8d, %r8d
	cmpb	$1, 76(%rsp)
	je	.L204
	jmp	.L207
.L214:
	movl	$1000000, %edx
	jmp	.L198
.L218:
	movl	$1000000, %ecx
	jmp	.L209
	.seh_endproc
	.p2align 4
	.globl	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii
	.def	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii
_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii:
.LFB3077:
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
	subq	$104, %rsp
	.seh_stackalloc	104
	.seh_endprologue
	movq	208(%rsp), %r14
	movl	216(%rsp), %r13d
	movzbl	%r8b, %eax
	movq	%rcx, %rbp
	movq	%rdx, %rsi
	movzbl	%r9b, %edx
	leaq	0(,%rax,8), %rcx
	movl	%r8d, 88(%rsp)
	subq	%rax, %rcx
	addq	%rdx, %rcx
	movslq	776(%rbp,%rcx,4), %rcx
	testl	%ecx, %ecx
	jle	.L246
	cmpb	$1, %r8b
	leaq	(%rdx,%rdx,4), %rdx
	leaq	240+FILE_RANK_LOOKUPS(%rip), %rdi
	sete	95(%rsp)
	imulq	$280, %rax, %rax
	addq	%rbp, %rax
	leaq	(%rax,%rdx,8), %r8
	movslq	%r13d, %rax
	leaq	(%r8,%rcx,4), %rdx
	leaq	(%r14,%rax,4), %r12
	movq	%r8, %rcx
.L249:
	movl	216(%rcx), %ebx
	cmpl	$-1, %ebx
	je	.L251
	testl	%r13d, %r13d
	jle	.L251
	movq	%rdx, 80(%rsp)
	movq	%rcx, %rdx
	movl	%r13d, 216(%rsp)
	movq	%rsi, %r13
.L266:
	movq	%rdx, 72(%rsp)
	movq	208(%rsp), %rsi
	movl	%ebx, %r14d
	.p2align 4
	.p2align 3
.L265:
	movslq	(%rsi), %r15
	leal	(%r15,%r14), %eax
	movslq	%eax, %rbx
	cmpl	$119, %eax
	jbe	.L254
	jmp	.L262
	.p2align 4,,10
	.p2align 3
.L288:
	movzbl	0(%rbp,%rbx), %r8d
	movl	%ebx, %edx
	testb	%r8b, %r8b
	jne	.L287
	movb	$0, 40(%rsp)
	movl	%r14d, %ecx
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movl	$0, 48(%rsp)
	addq	%r15, %rbx
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%r13), %rdx
	movl	%eax, %ecx
	movq	%rdx, %rax
	movl	%ecx, 0(%r13,%rdx,8)
	movl	$0, 4(%r13,%rdx,8)
	addl	$1, %eax
	movl	%eax, 2048(%r13)
	cmpl	$119, %ebx
	ja	.L262
.L254:
	cmpb	$0, (%rdi,%rbx)
	jne	.L288
.L262:
	addq	$4, %rsi
	cmpq	%rsi, %r12
	jne	.L265
.L290:
	movq	72(%rsp), %rdx
	movq	80(%rsp), %rax
	addq	$4, %rdx
	cmpq	%rdx, %rax
	je	.L246
	movl	216(%rdx), %ebx
	cmpl	$-1, %ebx
	jne	.L266
	movq	%r13, %rsi
	movl	216(%rsp), %r13d
	movq	%rdx, %rcx
	movq	%rax, %rdx
.L251:
	addq	$4, %rcx
	cmpq	%rcx, %rdx
	jne	.L249
.L246:
	addq	$104, %rsp
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
.L287:
	cmpb	$-1, %r8b
	je	.L289
	movl	%r8d, %eax
	shrb	$3, %al
	cmpb	$0, 88(%rsp)
	jne	.L257
	testb	$1, %al
	je	.L262
.L258:
	andl	$7, %r8d
.L256:
	movl	$0, 48(%rsp)
	xorl	%r9d, %r9d
	movl	%r14d, %ecx
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%r13), %rcx
	movzbl	120(%rbp), %r9d
	movl	%eax, 0(%r13,%rcx,8)
	movq	%rcx, %rdx
	andl	$0, 4(%r13,%rcx,8)
	testb	%r9b, %r9b
	je	.L259
	movl	$1000000, %r8d
	cmpb	$1, %r9b
	jne	.L260
.L259:
	movl	%eax, %r8d
	sarl	$14, %r8d
	andl	$15, %r8d
	je	.L269
	andl	$7, %r8d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rbx
	movl	(%rbx,%r8,4), %r8d
	leal	(%r8,%r8,4), %r8d
	leal	1000000(%r8,%r8), %r8d
.L260:
	andl	$127, %eax
	cmpl	$119, %eax
	jg	.L261
	movzbl	0(%rbp,%rax), %eax
	leal	-1(%rax), %r9d
	cmpb	$-3, %r9b
	ja	.L261
	andl	$7, %eax
	leaq	_ZL15PIECE_VALUES_MG(%rip), %rbx
	subl	(%rbx,%rax,4), %r8d
.L261:
	addl	$1, %edx
	addq	$4, %rsi
	movl	%r8d, 4(%r13,%rcx,8)
	movl	%edx, 2048(%r13)
	cmpq	%rsi, %r12
	jne	.L265
	jmp	.L290
	.p2align 4,,10
	.p2align 3
.L257:
	testb	$1, %al
	jne	.L262
	cmpb	$0, 95(%rsp)
	jne	.L258
	addq	$4, %rsi
	cmpq	%rsi, %r12
	jne	.L265
	jmp	.L290
	.p2align 4,,10
	.p2align 3
.L289:
	xorl	%r8d, %r8d
	cmpb	$1, 88(%rsp)
	ja	.L256
	addq	$4, %rsi
	cmpq	%rsi, %r12
	jne	.L265
	jmp	.L290
.L269:
	movl	$1000000, %r8d
	jmp	.L260
	.seh_endproc
	.p2align 4
	.globl	_Z21generate_bishop_movesRK8PositionR10S_MOVELIST5Color
	.def	_Z21generate_bishop_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z21generate_bishop_movesRK8PositionR10S_MOVELIST5Color
_Z21generate_bishop_movesRK8PositionR10S_MOVELIST5Color:
.LFB3078:
	subq	$56, %rsp
	.seh_stackalloc	56
	.seh_endprologue
	leaq	_ZZ21generate_bishop_movesRK8PositionR10S_MOVELIST5ColorE10directions(%rip), %rax
	movl	$3, %r9d
	movq	%rax, 32(%rsp)
	call	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.1
	nop
	addq	$56, %rsp
	ret
	.seh_endproc
	.p2align 4
	.globl	_Z19generate_rook_movesRK8PositionR10S_MOVELIST5Color
	.def	_Z19generate_rook_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z19generate_rook_movesRK8PositionR10S_MOVELIST5Color
_Z19generate_rook_movesRK8PositionR10S_MOVELIST5Color:
.LFB3079:
	subq	$56, %rsp
	.seh_stackalloc	56
	.seh_endprologue
	leaq	_ZZ19generate_rook_movesRK8PositionR10S_MOVELIST5ColorE10directions(%rip), %rax
	movl	$4, %r9d
	movq	%rax, 32(%rsp)
	call	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.1
	nop
	addq	$56, %rsp
	ret
	.seh_endproc
	.p2align 4
	.globl	_Z20generate_queen_movesRK8PositionR10S_MOVELIST5Color
	.def	_Z20generate_queen_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z20generate_queen_movesRK8PositionR10S_MOVELIST5Color
_Z20generate_queen_movesRK8PositionR10S_MOVELIST5Color:
.LFB3080:
	.seh_endprologue
	jmp	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.0
	.seh_endproc
	.p2align 4
	.globl	_Z19generate_king_movesRK8PositionR10S_MOVELIST5Color
	.def	_Z19generate_king_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z19generate_king_movesRK8PositionR10S_MOVELIST5Color
_Z19generate_king_movesRK8PositionR10S_MOVELIST5Color:
.LFB3081:
	pushq	%r14
	.seh_pushreg	%r14
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
	subq	$88, %rsp
	.seh_stackalloc	88
	.seh_endprologue
	movzbl	%r8b, %eax
	movq	%rcx, %rsi
	movq	%rdx, %rdi
	movl	%r8d, %ebp
	movl	136(%rcx,%rax,4), %ebx
	cmpl	$-1, %ebx
	je	.L294
	leaq	KING_DELTAS(%rip), %r10
	leaq	32(%r10), %r11
	testb	%r8b, %r8b
	jne	.L297
	leaq	FILE_RANK_LOOKUPS(%rip), %r12
	.p2align 4
	.p2align 3
.L306:
	movl	(%r10), %edx
	addl	%ebx, %edx
	cmpl	$119, %edx
	ja	.L298
	movslq	%edx, %rax
	cmpb	$0, 240(%r12,%rax)
	je	.L298
	movzbl	(%rsi,%rax), %r8d
	testb	%r8b, %r8b
	je	.L299
	cmpb	$-1, %r8b
	je	.L298
	testb	$8, %r8b
	jne	.L364
	.p2align 4
	.p2align 3
.L298:
	addq	$4, %r10
	cmpq	%r10, %r11
	jne	.L306
	cmpl	$25, %ebx
	je	.L365
.L294:
	addq	$88, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r14
	ret
	.p2align 4,,10
	.p2align 3
.L297:
	cmpb	$1, %r8b
	leaq	FILE_RANK_LOOKUPS(%rip), %r12
	sete	%r14b
	.p2align 4
	.p2align 3
.L315:
	movl	(%r10), %edx
	addl	%ebx, %edx
	cmpl	$119, %edx
	ja	.L308
	movslq	%edx, %rax
	cmpb	$0, 240(%r12,%rax)
	je	.L308
	movzbl	(%rsi,%rax), %r8d
	testb	%r8b, %r8b
	je	.L366
	cmpb	$-1, %r8b
	je	.L367
	testb	$8, %r8b
	jne	.L308
	testb	%r14b, %r14b
	jne	.L368
	.p2align 4
	.p2align 3
.L308:
	addq	$4, %r10
	cmpq	%r10, %r11
	jne	.L315
.L370:
	cmpl	$95, %ebx
	jne	.L294
	movzbl	128(%rsi), %eax
	testb	$4, %al
	je	.L322
	cmpb	$12, 98(%rsi)
	je	.L369
.L322:
	testb	$8, %al
	je	.L294
	cmpb	$12, 91(%rsi)
	jne	.L294
	cmpb	$0, 94(%rsi)
	jne	.L294
	cmpb	$0, 93(%rsi)
	jne	.L294
	cmpb	$0, 92(%rsi)
	jne	.L294
	xorl	%r8d, %r8d
	movq	%rsi, %rdx
	movl	$95, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L294
	xorl	%r8d, %r8d
	movq	%rsi, %rdx
	movl	$94, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L294
	xorl	%r8d, %r8d
	movq	%rsi, %rdx
	movl	$93, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L294
	xorl	%eax, %eax
	movl	$1, 48(%rsp)
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movb	$0, 40(%rsp)
	movl	$93, %edx
	movl	$95, %ecx
	movl	%eax, 32(%rsp)
	jmp	.L363
	.p2align 4,,10
	.p2align 3
.L299:
	movl	$0, 48(%rsp)
	movl	%ebx, %ecx
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 72(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %rdx
	movq	72(%rsp), %r10
	leaq	32+KING_DELTAS(%rip), %r11
	movl	%eax, %ecx
	movq	%rdx, %rax
	movl	%ecx, (%rdi,%rdx,8)
	addl	$1, %eax
	movl	$0, 4(%rdi,%rdx,8)
	movl	%eax, 2048(%rdi)
	jmp	.L298
	.p2align 4,,10
	.p2align 3
.L366:
	movl	$0, 48(%rsp)
	movl	%ebx, %ecx
	xorl	%r9d, %r9d
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 72(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %rdx
	movq	72(%rsp), %r10
	leaq	32+KING_DELTAS(%rip), %r11
	movl	%eax, %ecx
	movq	%rdx, %rax
	addq	$4, %r10
	movl	%ecx, (%rdi,%rdx,8)
	addl	$1, %eax
	movl	$0, 4(%rdi,%rdx,8)
	movl	%eax, 2048(%rdi)
	cmpq	%r10, %r11
	jne	.L315
	jmp	.L370
	.p2align 4,,10
	.p2align 3
.L365:
	movzbl	128(%rsi), %eax
	testb	$1, %al
	je	.L317
	cmpb	$4, 28(%rsi)
	je	.L371
.L317:
	testb	$2, %al
	je	.L294
	cmpb	$4, 21(%rsi)
	jne	.L294
	cmpb	$0, 24(%rsi)
	jne	.L294
	cmpb	$0, 23(%rsi)
	jne	.L294
	cmpb	$0, 22(%rsi)
	jne	.L294
	movl	$1, %r8d
	movq	%rsi, %rdx
	movl	$25, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L294
	movl	$1, %r8d
	movq	%rsi, %rdx
	movl	$24, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L294
	movl	$1, %r8d
	movq	%rsi, %rdx
	movl	$23, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L294
	movl	$1, 48(%rsp)
	xorl	%ecx, %ecx
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movb	$0, 40(%rsp)
	movl	$23, %edx
	movl	%ecx, 32(%rsp)
	movl	$25, %ecx
.L363:
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %rdx
	movl	%eax, %ecx
	movq	%rdx, %rax
	movl	%ecx, (%rdi,%rdx,8)
	addl	$1, %eax
	movl	$50000, 4(%rdi,%rdx,8)
	movl	%eax, 2048(%rdi)
	jmp	.L294
	.p2align 4,,10
	.p2align 3
.L368:
	andl	$7, %r8d
.L311:
	movl	$0, 48(%rsp)
	xorl	%r9d, %r9d
	movl	%ebx, %ecx
	movb	$0, 40(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 72(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movzbl	120(%rsi), %r9d
	movslq	2048(%rdi), %r8
	leaq	32+KING_DELTAS(%rip), %r11
	movq	72(%rsp), %r10
	andl	$0, 4(%rdi,%r8,8)
	testb	%r9b, %r9b
	movq	%r8, %rdx
	movl	%eax, (%rdi,%r8,8)
	je	.L312
	movl	$1000000, %ecx
	cmpb	$1, %r9b
	je	.L312
.L313:
	andl	$127, %eax
	cmpl	$119, %eax
	jg	.L314
	movzbl	(%rsi,%rax), %eax
	leal	-1(%rax), %r9d
	cmpb	$-3, %r9b
	ja	.L314
	andl	$7, %eax
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r9
	subl	(%r9,%rax,4), %ecx
.L314:
	addl	$1, %edx
	addq	$4, %r10
	movl	%ecx, 4(%rdi,%r8,8)
	movl	%edx, 2048(%rdi)
	cmpq	%r10, %r11
	jne	.L315
	jmp	.L370
	.p2align 4,,10
	.p2align 3
.L312:
	movl	%eax, %ecx
	sarl	$14, %ecx
	andl	$15, %ecx
	je	.L332
	andl	$7, %ecx
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r9
	movl	(%r9,%rcx,4), %ecx
	leal	(%rcx,%rcx,4), %ecx
	leal	1000000(%rcx,%rcx), %ecx
	jmp	.L313
	.p2align 4,,10
	.p2align 3
.L364:
	movb	$0, 40(%rsp)
	xorl	%r9d, %r9d
	andl	$7, %r8d
	movl	%ebx, %ecx
	movl	$0, 48(%rsp)
	movl	$0, 32(%rsp)
	movq	%r10, 72(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %r8
	movzbl	120(%rsi), %edx
	movl	$8, %r9d
	movq	72(%rsp), %r10
	leaq	32+KING_DELTAS(%rip), %r11
	andl	$0, 4(%rdi,%r8,8)
	testb	%dl, %dl
	movq	%r8, %rcx
	movl	%eax, (%rdi,%r8,8)
	je	.L304
	cmpb	$1, %dl
	je	.L329
.L328:
	movl	$1000000, %edx
.L303:
	andl	$127, %eax
	cmpl	$119, %eax
	jg	.L302
	movzbl	(%rsi,%rax), %eax
	leal	-1(%rax), %r9d
	cmpb	$-3, %r9b
	ja	.L302
	andl	$7, %eax
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r9
	subl	(%r9,%rax,4), %edx
.L302:
	addl	$1, %ecx
	movl	%edx, 4(%rdi,%r8,8)
	movl	%ecx, 2048(%rdi)
	jmp	.L298
.L329:
	movl	%ebp, %r9d
.L304:
	movl	%eax, %edx
	sarl	$14, %edx
	andl	$15, %edx
	je	.L328
	orl	%r9d, %edx
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r9
	andl	$7, %edx
	movl	(%r9,%rdx,4), %edx
	leal	(%rdx,%rdx,4), %edx
	leal	1000000(%rdx,%rdx), %edx
	jmp	.L303
	.p2align 4,,10
	.p2align 3
.L367:
	xorl	%r8d, %r8d
	cmpb	$1, %bpl
	je	.L308
	jmp	.L311
.L371:
	cmpb	$0, 26(%rsi)
	jne	.L317
	cmpb	$0, 27(%rsi)
	jne	.L317
	movl	$1, %r8d
	movq	%rsi, %rdx
	movl	$25, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	je	.L318
.L361:
	movzbl	128(%rsi), %eax
	jmp	.L317
.L369:
	cmpb	$0, 96(%rsi)
	jne	.L322
	cmpb	$0, 97(%rsi)
	jne	.L322
	xorl	%r8d, %r8d
	movq	%rsi, %rdx
	movl	$95, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	je	.L323
.L362:
	movzbl	128(%rsi), %eax
	jmp	.L322
.L332:
	movl	$1000000, %ecx
	jmp	.L313
.L318:
	movl	$1, %r8d
	movq	%rsi, %rdx
	movl	$26, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L361
	movl	$1, %r8d
	movq	%rsi, %rdx
	movl	$27, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L361
	xorl	%r8d, %r8d
	movb	$0, 40(%rsp)
	movl	$27, %edx
	xorl	%r9d, %r9d
	movl	%r8d, 32(%rsp)
	movl	$25, %ecx
	xorl	%r8d, %r8d
	movl	$1, 48(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %rdx
	movl	%eax, %ecx
	movq	%rdx, %rax
	movl	%ecx, (%rdi,%rdx,8)
	addl	$1, %eax
	movl	$50000, 4(%rdi,%rdx,8)
	movl	%eax, 2048(%rdi)
	jmp	.L361
.L323:
	xorl	%r8d, %r8d
	movq	%rsi, %rdx
	movl	$96, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L362
	xorl	%r8d, %r8d
	movq	%rsi, %rdx
	movl	$97, %ecx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L362
	xorl	%edx, %edx
	movl	$1, 48(%rsp)
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	movl	%edx, 32(%rsp)
	movl	$95, %ecx
	movl	$97, %edx
	movb	$0, 40(%rsp)
	call	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	movslq	2048(%rdi), %rdx
	movl	%eax, %ecx
	movq	%rdx, %rax
	movl	%ecx, (%rdi,%rdx,8)
	addl	$1, %eax
	movl	$50000, 4(%rdi,%rdx,8)
	movl	%eax, 2048(%rdi)
	jmp	.L362
	.seh_endproc
	.p2align 4
	.globl	_Z18generate_all_movesRK8PositionR10S_MOVELIST
	.def	_Z18generate_all_movesRK8PositionR10S_MOVELIST;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z18generate_all_movesRK8PositionR10S_MOVELIST
_Z18generate_all_movesRK8PositionR10S_MOVELIST:
.LFB3074:
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$48, %rsp
	.seh_stackalloc	48
	.seh_endprologue
	movzbl	120(%rcx), %edi
	movl	%edi, %r8d
	movl	$0, 2048(%rdx)
	movq	%rcx, %rbx
	movq	%rdx, %rsi
	call	_Z19generate_pawn_movesRK8PositionR10S_MOVELIST5Color
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	call	_Z21generate_knight_movesRK8PositionR10S_MOVELIST5Color
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	leaq	_ZZ21generate_bishop_movesRK8PositionR10S_MOVELIST5ColorE10directions(%rip), %rax
	movl	$3, %r9d
	movq	%rax, 32(%rsp)
	call	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.1
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	leaq	_ZZ19generate_rook_movesRK8PositionR10S_MOVELIST5ColorE10directions(%rip), %rax
	movl	$4, %r9d
	movq	%rax, 32(%rsp)
	call	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.1
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	call	_Z22generate_sliding_movesRK8PositionR10S_MOVELIST5Color9PieceTypePKii.constprop.0
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	addq	$48, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	jmp	_Z19generate_king_movesRK8PositionR10S_MOVELIST5Color
	.seh_endproc
	.section .rdata,"dr"
.LC0:
	.ascii "vector::_M_default_append\0"
	.section	.text$_ZNSt6vectorI6S_UNDOSaIS0_EE17_M_default_appendEy,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZNSt6vectorI6S_UNDOSaIS0_EE17_M_default_appendEy
	.def	_ZNSt6vectorI6S_UNDOSaIS0_EE17_M_default_appendEy;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZNSt6vectorI6S_UNDOSaIS0_EE17_M_default_appendEy
_ZNSt6vectorI6S_UNDOSaIS0_EE17_M_default_appendEy:
.LFB3561:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$56, %rsp
	.seh_stackalloc	56
	.seh_endprologue
	movq	%rcx, %r9
	movq	%rdx, %r8
	testq	%rdx, %rdx
	je	.L373
	movq	16(%rcx), %rax
	movq	8(%rcx), %rdx
	movabsq	$3446095046736949203, %rcx
	movq	%rax, %r14
	subq	%rdx, %rax
	sarq	$3, %rax
	imulq	%rcx, %rax
	cmpq	%r8, %rax
	jnb	.L407
	movabsq	$12669467083591725, %rax
	movq	(%r9), %rbx
	movq	%rdx, %rbp
	subq	%rbx, %rbp
	movq	%rbp, %r10
	sarq	$3, %r10
	imulq	%rcx, %r10
	subq	%r10, %rax
	cmpq	%r8, %rax
	jb	.L408
	cmpq	%r10, %r8
	movq	%r10, %rax
	movq	%r9, 112(%rsp)
	movabsq	$12669467083591725, %rcx
	cmovnb	%r8, %rax
	movq	%rdx, 40(%rsp)
	movq	%r8, 120(%rsp)
	addq	%r10, %rax
	movq	%r10, 32(%rsp)
	cmpq	%rcx, %rax
	cmova	%rcx, %rax
	imulq	$728, %rax, %rax
	movq	%rax, %rcx
	movq	%rax, %r15
	call	_Znwy
	movq	120(%rsp), %r8
	movq	112(%rsp), %r9
	leaq	(%rax,%rbp), %rcx
	movq	40(%rsp), %rdx
	movq	32(%rsp), %r10
	movq	%rax, %r11
	imulq	$728, %r8, %rbp
	movq	%rcx, %rax
	addq	%rcx, %rbp
	.p2align 6
	.p2align 4
	.p2align 3
.L379:
	xorl	%esi, %esi
	movq	$0, (%rax)
	addq	$728, %rax
	movb	$0, -720(%rax)
	movw	%si, -712(%rax)
	movq	$0, -704(%rax)
	movb	$0, -696(%rax)
	cmpq	%rax, %rbp
	jne	.L379
	movq	%rbp, %rax
	subq	%rcx, %rax
	testb	$8, %al
	je	.L380
	movl	$-1, 12(%rcx)
	addq	$728, %rcx
	cmpq	%rbp, %rcx
	je	.L406
	.p2align 5
	.p2align 4
	.p2align 3
.L380:
	movl	$-1, 12(%rcx)
	addq	$1456, %rcx
	movl	$-1, -716(%rcx)
	cmpq	%rbp, %rcx
	jne	.L380
.L406:
	cmpq	%rdx, %rbx
	je	.L381
	movq	%r11, %rbp
	movq	%rbx, %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L382:
	movq	%rax, %rsi
	addq	$728, %rax
	movq	%rbp, %rdi
	movl	$91, %ecx
	rep movsq
	addq	$728, %rbp
	cmpq	%rax, %rdx
	jne	.L382
.L381:
	testq	%rbx, %rbx
	je	.L383
	movq	%r14, %rdx
	movq	%rbx, %rcx
	movq	%r11, 40(%rsp)
	subq	%rbx, %rdx
	movq	%r10, 32(%rsp)
	movq	%r8, 120(%rsp)
	movq	%r9, 112(%rsp)
	call	_ZdlPvy
	movq	40(%rsp), %r11
	movq	32(%rsp), %r10
	movq	120(%rsp), %r8
	movq	112(%rsp), %r9
.L383:
	addq	%r8, %r10
	movq	%r11, (%r9)
	imulq	$728, %r10, %r10
	addq	%r11, %r10
	addq	%r15, %r11
	movq	%r10, 8(%r9)
	movq	%r11, 16(%r9)
.L373:
	addq	$56, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L407:
	imulq	$728, %r8, %rcx
	movq	%rdx, %rax
	addq	%rdx, %rcx
	.p2align 6
	.p2align 4
	.p2align 3
.L376:
	xorl	%edi, %edi
	movq	$0, (%rax)
	addq	$728, %rax
	movb	$0, -720(%rax)
	movw	%di, -712(%rax)
	movq	$0, -704(%rax)
	movb	$0, -696(%rax)
	cmpq	%rcx, %rax
	jne	.L376
	movq	%rcx, %r10
	movq	%rdx, %rax
	subq	%rdx, %r10
	andl	$8, %r10d
	je	.L377
	leaq	728(%rdx), %rax
	movl	$-1, 12(%rdx)
	cmpq	%rcx, %rax
	je	.L405
	.p2align 5
	.p2align 4
	.p2align 3
.L377:
	movl	$-1, 12(%rax)
	addq	$1456, %rax
	movl	$-1, -716(%rax)
	cmpq	%rcx, %rax
	jne	.L377
.L405:
	imulq	$728, %r8, %r8
	leaq	(%r8,%rdx), %rax
	movq	%rax, 8(%r9)
	addq	$56, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r14
	popq	%r15
	ret
.L408:
	leaq	.LC0(%rip), %rcx
	call	_ZSt20__throw_length_errorPKc
	nop
	.seh_endproc
	.section	.text$_ZN8Position19make_move_with_undoERK6S_MOVE,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZN8Position19make_move_with_undoERK6S_MOVE
	.def	_ZN8Position19make_move_with_undoERK6S_MOVE;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN8Position19make_move_with_undoERK6S_MOVE
_ZN8Position19make_move_with_undoERK6S_MOVE:
.LFB2778:
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
	subq	$56, %rsp
	.seh_stackalloc	56
	.seh_endprologue
	movq	840(%rcx), %r8
	movq	832(%rcx), %rdi
	movslq	856(%rcx), %rax
	movq	%rcx, %rbx
	movq	%r8, %rcx
	movq	%rdx, %rsi
	movabsq	$3446095046736949203, %rdx
	subq	%rdi, %rcx
	sarq	$3, %rcx
	imulq	%rdx, %rcx
	cmpl	%ecx, %eax
	jge	.L876
.L411:
	imulq	$728, %rax, %rax
	movl	$-1, %edx
	addq	%rax, %rdi
	movq	(%rsi), %rax
	movq	%rax, (%rdi)
	movzbl	128(%rbx), %eax
	movb	%al, 8(%rdi)
	movl	124(%rbx), %eax
	movl	%eax, 12(%rdi)
	movzwl	130(%rbx), %eax
	movw	%ax, 16(%rdi)
	movq	200(%rbx), %rax
	movq	%rax, 24(%rdi)
	movl	(%rsi), %eax
	sarl	$7, %eax
	andl	$127, %eax
	cmpl	$119, %eax
	jg	.L414
	movzbl	(%rbx,%rax), %edx
.L414:
	movb	%dl, 32(%rdi)
	movq	%rbx, %rcx
	movq	%rdi, %rdx
	call	_ZN8Position18save_derived_stateER6S_UNDO
	movl	(%rsi), %eax
	movzbl	32(%rdi), %r9d
	movl	%eax, %edx
	movl	%eax, %r11d
	movl	%eax, %ecx
	andl	$127, %edx
	andl	$16777216, %r11d
	cmpl	$119, %edx
	jg	.L415
	movslq	%edx, %r12
	movzbl	(%rbx,%r12), %r10d
	leal	-1(%r10), %ebp
	movl	%r10d, %r8d
	cmpb	$-3, %bpl
	jbe	.L877
	testb	%r10b, %r10b
	jne	.L422
	testb	%r9b, %r9b
	jne	.L878
	testl	%r11d, %r11d
	je	.L442
	andb	$-13, 128(%rbx)
	movl	(%rsi), %eax
.L442:
	addw	$1, 130(%rbx)
	movl	%eax, %ecx
	testl	$262144, %eax
	je	.L879
	xorl	%r11d, %r11d
	xorl	%r8d, %r8d
	movl	$2, %r10d
	xorl	%r9d, %r9d
.L482:
	sarl	$7, %eax
	andl	$127, %eax
	addl	$10, %eax
	cmpl	$119, %eax
	ja	.L599
.L890:
	movslq	%eax, %rcx
	movzbl	(%rbx,%rcx), %r12d
	movq	%rcx, %r14
	testb	%r12b, %r12b
	je	.L857
	cmpb	$-1, %r12b
	je	.L505
	movl	%r12d, %ecx
	shrb	$3, %cl
	andl	$1, %ecx
	imulq	$28, %rcx, %rdx
	movl	780(%rbx,%rdx), %edx
	testl	%edx, %edx
	jle	.L505
	imulq	$280, %rcx, %rbp
	addq	%rbx, %rbp
	cmpl	256(%rbp), %eax
	je	.L600
	cmpl	$1, %edx
	je	.L505
	cmpl	260(%rbp), %eax
	je	.L601
	cmpl	$2, %edx
	je	.L505
	cmpl	264(%rbp), %eax
	je	.L602
	cmpl	$3, %edx
	je	.L505
	imulq	$280, %rcx, %rbp
	addq	%rbx, %rbp
	cmpl	268(%rbp), %eax
	je	.L603
	cmpl	$4, %edx
	je	.L505
	cmpl	272(%rbp), %eax
	je	.L604
	cmpl	$5, %edx
	je	.L505
	cmpl	276(%rbp), %eax
	je	.L605
	cmpl	$6, %edx
	je	.L505
	cmpl	280(%rbp), %eax
	je	.L606
	cmpl	$7, %edx
	je	.L505
	cmpl	284(%rbp), %eax
	je	.L607
	cmpl	$8, %edx
	je	.L505
	imulq	$280, %rcx, %rbp
	addq	%rbx, %rbp
	cmpl	288(%rbp), %eax
	je	.L608
	cmpl	$9, %edx
	je	.L505
	movl	$9, %r15d
	cmpl	292(%rbp), %eax
	je	.L506
.L505:
	leaq	FILE_RANK_LOOKUPS(%rip), %rbp
	cmpb	$0, 240(%rbp,%r14)
	je	.L503
	movb	$0, (%rbx,%r14)
.L503:
	movb	%r12b, 32(%rdi)
.L857:
	movl	(%rsi), %eax
	movl	%eax, %ecx
.L501:
	movl	%eax, %ebp
	movl	%eax, %edx
	sarl	$20, %ebp
	andl	$127, %edx
	movl	%ebp, %r12d
	andl	$15, %r12d
	je	.L509
	cmpb	$2, %r10b
	je	.L865
	movzbl	%r10b, %r11d
	imulq	$28, %r11, %rcx
	movl	780(%rbx,%rcx), %ecx
	testl	%ecx, %ecx
	jle	.L512
	imulq	$280, %r11, %rbp
	addq	%rbx, %rbp
	cmpl	%edx, 256(%rbp)
	je	.L609
	cmpl	$1, %ecx
	je	.L512
	cmpl	%edx, 260(%rbp)
	je	.L610
	cmpl	$2, %ecx
	je	.L512
	cmpl	%edx, 264(%rbp)
	je	.L611
	cmpl	$3, %ecx
	je	.L512
	cmpl	%edx, 268(%rbp)
	je	.L612
	cmpl	$4, %ecx
	je	.L512
	imulq	$280, %r11, %rbp
	addq	%rbx, %rbp
	cmpl	%edx, 272(%rbp)
	je	.L613
	cmpl	$5, %ecx
	je	.L512
	cmpl	%edx, 276(%rbp)
	je	.L614
	cmpl	$6, %ecx
	je	.L512
	cmpl	%edx, 280(%rbp)
	je	.L615
	cmpl	$7, %ecx
	je	.L512
	cmpl	%edx, 284(%rbp)
	je	.L616
	cmpl	$8, %ecx
	je	.L512
	cmpl	%edx, 288(%rbp)
	je	.L617
	cmpl	$9, %ecx
	je	.L512
	imulq	$280, %r11, %rbp
	cmpl	%edx, 292(%rbx,%rbp)
	je	.L880
.L512:
	sarl	$7, %eax
	movl	%eax, %edx
	andl	$127, %edx
.L574:
	movzbl	%r12b, %ecx
	leaq	(%rcx,%r11,8), %rax
	subq	%r11, %rax
	movq	%rax, %rbp
	movslq	776(%rbx,%rax,4), %rax
	cmpl	$9, %eax
	jle	.L843
	movl	(%rsi), %eax
.L514:
	sall	$3, %r10d
	orl	%r10d, %r12d
.L515:
	cmpl	$119, %edx
	jg	.L858
	leaq	FILE_RANK_LOOKUPS(%rip), %rbp
	cmpb	$0, 240(%rbp,%rdx)
	je	.L858
	movb	%r12b, (%rbx,%rdx)
.L858:
	movl	%eax, %edx
	movl	%eax, %ecx
	andl	$127, %edx
.L518:
	cmpl	$119, %edx
	jg	.L526
	leaq	FILE_RANK_LOOKUPS(%rip), %rbp
	cmpb	$0, 240(%rbp,%rdx)
	je	.L526
	movb	$0, (%rbx,%rdx)
.L526:
	testl	$16777216, %ecx
	je	.L528
	leal	-1(%r8), %eax
	andl	$16256, %ecx
	cmpb	$-3, %al
	ja	.L529
	testb	$8, %r8b
	jne	.L530
	cmpl	$3456, %ecx
	je	.L531
	movzbl	21(%rbx), %eax
	movb	$0, 21(%rbx)
	movl	$21, %edx
	movl	$24, %r11d
	movb	%al, 24(%rbx)
.L532:
	movl	792(%rbx), %eax
	testl	%eax, %eax
	jle	.L570
	cmpl	376(%rbx), %edx
	je	.L571
	xorl	%r10d, %r10d
	cmpl	$1, %eax
	je	.L859
	.p2align 4
	.p2align 3
.L572:
	imulq	$280, %r10, %rcx
	addq	%rbx, %rcx
	cmpl	%edx, 380(%rcx)
	je	.L628
	cmpl	$2, %eax
	je	.L528
	cmpl	%edx, 384(%rcx)
	je	.L629
	cmpl	$3, %eax
	je	.L528
	cmpl	%edx, 388(%rcx)
	je	.L630
	cmpl	$4, %eax
	je	.L528
	cmpl	%edx, 392(%rcx)
	je	.L631
	cmpl	$5, %eax
	je	.L528
	cmpl	%edx, 396(%rcx)
	je	.L632
	cmpl	$6, %eax
	je	.L528
	imulq	$280, %r10, %rcx
	addq	%rbx, %rcx
	cmpl	%edx, 400(%rcx)
	je	.L633
	cmpl	$7, %eax
	je	.L528
	cmpl	%edx, 404(%rcx)
	je	.L634
	cmpl	$8, %eax
	je	.L528
	cmpl	%edx, 408(%rcx)
	je	.L635
	cmpl	$9, %eax
	je	.L528
	movl	$9, %eax
	cmpl	%edx, 412(%rcx)
	je	.L536
	.p2align 4
	.p2align 3
.L528:
	movl	$-1, 124(%rbx)
	movzbl	120(%rbx), %eax
	testb	%r8b, %r8b
	jne	.L537
	movl	$1, %edx
	testb	%al, %al
	je	.L538
	movl	$2, %edx
	cmpb	$1, %al
	jne	.L538
	addw	$1, 132(%rbx)
	xorl	%edx, %edx
.L538:
	movb	%dl, 120(%rbx)
	testb	%r9b, %r9b
	jne	.L556
.L863:
	testb	$-16, 2(%rsi)
	je	.L558
	movl	$1, %r11d
	movl	$2, %eax
	xorl	%ebp, %ebp
.L559:
	subl	$1, 172(%rbx)
	movl	(%rsi), %edx
	leaq	(%rbx,%rax,4), %r10
	sarl	$20, %edx
	andl	$15, %edx
	addl	$1, 168(%rbx,%rdx,4)
	movl	208(%r10), %ecx
	subl	%ebp, %ecx
	movl	%ecx, 208(%r10)
	movl	(%rsi), %edx
	sarl	$20, %edx
	andl	$15, %edx
	je	.L568
	testb	%r11b, %r11b
	jne	.L568
	andl	$7, %edx
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r11
	addl	(%r11,%rdx,4), %ecx
.L568:
	movl	%ecx, 208(%r10)
	movl	(%rsi), %edx
	leaq	MAILBOX_MAPS(%rip), %rcx
	sarl	$7, %edx
	andl	$127, %edx
	movslq	256(%rcx,%rdx,4), %rdx
	testl	%edx, %edx
	js	.L558
	leaq	_ZL10CLEAR_MASK(%rip), %rcx
	movq	(%rcx,%rdx,8), %rdx
	andq	%rdx, 144(%rbx,%rax,8)
	andq	%rdx, 160(%rbx)
.L558:
	movl	12(%rdi), %eax
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	movl	%eax, 40(%rsp)
	movzbl	8(%rdi), %eax
	movl	%eax, 32(%rsp)
	call	_ZN8Position23update_zobrist_for_moveERK6S_MOVE5PieceS3_hi
	addl	$1, 856(%rbx)
	addq	$56, %rsp
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
.L877:
	movl	%r10d, %ebp
	andl	$7, %ebp
	cmpb	$6, %bpl
	je	.L881
	cmpb	$4, %bpl
	je	.L882
.L846:
	testb	%r9b, %r9b
	jne	.L419
	testl	%r11d, %r11d
	je	.L476
.L445:
	movzbl	128(%rbx), %ebp
	movl	%r10d, %r11d
	testb	%r10b, %r10b
	je	.L883
	cmpb	$-1, %r10b
	jne	.L471
	andl	$-13, %ebp
	movb	%bpl, 128(%rbx)
	movl	(%rsi), %eax
	jmp	.L461
	.p2align 4,,10
	.p2align 3
.L415:
	testb	%r9b, %r9b
	je	.L884
	cmpb	$-1, %r9b
	je	.L885
	movl	%r9d, %r8d
	andl	$7, %r8d
	cmpb	$4, %r8b
	je	.L430
	testl	%r11d, %r11d
	je	.L432
.L431:
	movzbl	128(%rbx), %ebp
	movslq	%edx, %r12
	cmpl	$119, %edx
	jg	.L886
.L437:
	movzbl	(%rbx,%r12), %r11d
	testb	%r11b, %r11b
	je	.L887
	cmpb	$-1, %r11b
	je	.L888
.L471:
	movl	%ebp, %edx
	andl	$-13, %ebp
	movl	%r11d, %r8d
	andl	$-4, %edx
	testb	$8, %r11b
	cmove	%edx, %ebp
	movb	%bpl, 128(%rbx)
.L476:
	movl	%r8d, %r11d
	andl	$7, %r11d
	cmpb	$1, %r11b
	je	.L490
	testb	%r9b, %r9b
	je	.L455
.L488:
	xorl	%ebp, %ebp
	movl	%r8d, %r10d
	movw	%bp, 130(%rbx)
	shrb	$3, %r10b
	andl	$1, %r10d
.L489:
	cmpb	$-1, %r9b
	je	.L839
.L485:
	movl	%r9d, %ebp
	andl	$7, %ebp
	je	.L839
.L496:
	movl	%r9d, %ecx
	movzbl	%bpl, %ebp
	shrb	$3, %cl
	andl	$1, %ecx
	leaq	0(,%rcx,8), %rdx
	movq	%rdx, %r13
	subq	%rcx, %rdx
	addq	%rbp, %rdx
	movl	776(%rbx,%rdx,4), %edx
	testl	%edx, %edx
	jle	.L839
	imulq	$280, %rcx, %r12
	leaq	0(,%rbp,4), %r14
	sarl	$7, %eax
	leaq	(%r14,%rbp), %r15
	andl	$127, %eax
	leaq	(%r12,%r15,8), %r12
	addq	%rbx, %r12
	cmpl	216(%r12), %eax
	je	.L588
	cmpl	$1, %edx
	je	.L839
	cmpl	220(%r12), %eax
	je	.L589
	cmpl	$2, %edx
	je	.L839
	cmpl	224(%r12), %eax
	je	.L590
	cmpl	$3, %edx
	je	.L839
	cmpl	228(%r12), %eax
	je	.L591
	cmpl	$4, %edx
	je	.L839
	cmpl	232(%r12), %eax
	je	.L592
	cmpl	$5, %edx
	je	.L839
	imulq	$280, %rcx, %r12
	leaq	(%r12,%r15,8), %r12
	addq	%rbx, %r12
	cmpl	236(%r12), %eax
	je	.L593
	cmpl	$6, %edx
	je	.L839
	cmpl	240(%r12), %eax
	je	.L594
	cmpl	$7, %edx
	je	.L839
	cmpl	244(%r12), %eax
	je	.L595
	cmpl	$8, %edx
	je	.L839
	cmpl	248(%r12), %eax
	je	.L596
	cmpl	$9, %edx
	je	.L839
	cmpl	252(%r12), %eax
	je	.L889
.L839:
	movl	(%rsi), %eax
	movl	%eax, %ecx
.L491:
	testl	$262144, %eax
	je	.L501
	testb	%r10b, %r10b
	jne	.L482
	sarl	$7, %eax
	andl	$127, %eax
	subl	$10, %eax
	cmpl	$119, %eax
	jbe	.L890
.L599:
	movl	$-1, %r12d
	jmp	.L503
	.p2align 4,,10
	.p2align 3
.L876:
	leal	1(%rax), %edx
	movslq	%edx, %rdx
	cmpq	%rdx, %rcx
	jb	.L891
	cmpq	%rcx, %rdx
	jnb	.L411
	imulq	$728, %rdx, %rdx
	addq	%rdi, %rdx
	cmpq	%rdx, %r8
	je	.L411
	movq	%rdx, 840(%rbx)
	jmp	.L411
	.p2align 4,,10
	.p2align 3
.L884:
	testl	%r11d, %r11d
	je	.L856
	andb	$-13, 128(%rbx)
	.p2align 4
	.p2align 3
.L856:
	movzwl	130(%rbx), %edx
	addl	$1, %edx
	movw	%dx, 130(%rbx)
.L486:
	testl	$262144, %eax
	jne	.L597
	movl	%eax, %r9d
	sarl	$20, %r9d
	andl	$15, %r9d
	je	.L892
	movl	$-1, %r8d
	xorl	%r9d, %r9d
	.p2align 4
	.p2align 3
.L865:
	movl	(%rsi), %eax
.L513:
	sarl	$7, %ecx
	xorl	%r12d, %r12d
	movl	%ecx, %edx
	andl	$127, %edx
	jmp	.L515
	.p2align 4,,10
	.p2align 3
.L878:
	cmpb	$-1, %r9b
	je	.L893
.L424:
	movl	%r9d, %r8d
	andl	$7, %r8d
	cmpb	$4, %r8b
	je	.L430
	testl	%r11d, %r11d
	jne	.L431
.L434:
	movzbl	(%rbx,%r12), %r8d
	testb	%r8b, %r8b
	jne	.L894
	xorl	%edx, %edx
	xorl	%r11d, %r11d
	movl	$2, %r10d
	movw	%dx, 130(%rbx)
	jmp	.L485
	.p2align 4,,10
	.p2align 3
.L881:
	andl	$8, %r10d
	movzbl	128(%rbx), %ebp
	je	.L895
	andl	$-13, %ebp
	movb	%bpl, 128(%rbx)
	testb	%r9b, %r9b
	jne	.L419
.L829:
	testl	%r11d, %r11d
	je	.L476
	movzbl	(%rbx,%r12), %r10d
	jmp	.L445
	.p2align 4,,10
	.p2align 3
.L422:
	testb	%r9b, %r9b
	je	.L896
.L419:
	cmpb	$-1, %r9b
	je	.L849
	movl	%r9d, %r8d
	andl	$7, %r8d
	cmpb	$4, %r8b
	je	.L430
.L462:
	testl	%r11d, %r11d
	jne	.L431
	cmpl	$119, %edx
	jle	.L897
.L432:
	xorl	%r10d, %r10d
	movw	%r10w, 130(%rbx)
.L473:
	movl	%r9d, %ebp
	andl	$7, %ebp
	je	.L898
	movl	$2, %r10d
	movl	$-1, %r8d
	xorl	%r11d, %r11d
	jmp	.L496
.L571:
	movl	%r11d, 376(%rbx)
.L859:
	movl	$-1, 124(%rbx)
	movzbl	120(%rbx), %eax
	.p2align 4
	.p2align 3
.L537:
	cmpb	$-1, %r8b
	je	.L899
.L540:
	movl	%r8d, %ecx
	andl	$7, %ecx
	cmpb	$1, %cl
	je	.L543
.L862:
	testb	%al, %al
	jne	.L544
	movb	$1, 120(%rbx)
	.p2align 4
	.p2align 3
.L545:
	movl	%r8d, %eax
	shrb	$3, %al
	andl	$1, %eax
	testb	%r9b, %r9b
	jne	.L638
	movl	(%rsi), %edx
	xorl	%r11d, %r11d
	movl	$100, %ebp
.L562:
	testl	$15728640, %edx
	je	.L567
	movzbl	%al, %eax
	jmp	.L559
	.p2align 4,,10
	.p2align 3
.L509:
	sarl	$7, %eax
	andl	$127, %eax
	cmpb	$2, %r10b
	je	.L522
	testb	%r11b, %r11b
	je	.L522
	movzbl	%r10b, %r10d
	movzbl	%r11b, %r11d
	leaq	(%r11,%r10,8), %rbp
	subq	%r10, %rbp
	movl	776(%rbx,%rbp,4), %r12d
	testl	%r12d, %r12d
	jle	.L522
	imulq	$280, %r10, %rbp
	leaq	0(,%r11,4), %r14
	leaq	(%r14,%r11), %r15
	leaq	0(%rbp,%r15,8), %rbp
	addq	%rbx, %rbp
	cmpl	%edx, 216(%rbp)
	je	.L618
	cmpl	$1, %r12d
	je	.L522
	cmpl	%edx, 220(%rbp)
	je	.L619
	cmpl	$2, %r12d
	je	.L522
	cmpl	%edx, 224(%rbp)
	je	.L620
	cmpl	$3, %r12d
	je	.L522
	imulq	$280, %r10, %rbp
	leaq	0(%rbp,%r15,8), %rbp
	addq	%rbx, %rbp
	cmpl	%edx, 228(%rbp)
	je	.L621
	cmpl	$4, %r12d
	je	.L522
	cmpl	%edx, 232(%rbp)
	je	.L622
	cmpl	$5, %r12d
	je	.L522
	cmpl	%edx, 236(%rbp)
	je	.L623
	cmpl	$6, %r12d
	je	.L522
	cmpl	%edx, 240(%rbp)
	je	.L624
	cmpl	$7, %r12d
	je	.L522
	cmpl	%edx, 244(%rbp)
	je	.L625
	cmpl	$8, %r12d
	je	.L522
	imulq	$280, %r10, %rbp
	leaq	0(%rbp,%r15,8), %rbp
	addq	%rbx, %rbp
	cmpl	%edx, 248(%rbp)
	je	.L626
	cmpl	$9, %r12d
	je	.L522
	cmpl	%edx, 252(%rbp)
	je	.L900
	.p2align 4
	.p2align 3
.L522:
	cmpl	$119, %eax
	jg	.L518
	leaq	FILE_RANK_LOOKUPS(%rip), %rbp
	cmpb	$0, 240(%rbp,%rax)
	je	.L518
	movb	%r8b, (%rbx,%rax)
	jmp	.L518
	.p2align 4,,10
	.p2align 3
.L567:
	cmpb	$1, %cl
	je	.L901
	cmpb	$6, %cl
	jne	.L558
	sarl	$7, %edx
	movzbl	%al, %eax
	andl	$127, %edx
	movl	%edx, 136(%rbx,%rax,4)
	jmp	.L558
.L894:
	cmpb	$-1, %r8b
	je	.L902
	movl	%r8d, %r11d
	andl	$7, %r11d
	cmpb	$1, %r11b
	jne	.L488
	.p2align 4
	.p2align 3
.L490:
	movl	%r8d, %r10d
	xorl	%r11d, %r11d
	shrb	$3, %r10b
	movw	%r11w, 130(%rbx)
	movl	$1, %r11d
	andl	$1, %r10d
	testb	%r9b, %r9b
	jne	.L489
	jmp	.L839
	.p2align 4,,10
	.p2align 3
.L638:
	xorl	%r11d, %r11d
	movl	$100, %ebp
.L561:
	cmpb	$-1, %r9b
	je	.L563
	movl	%r9d, %r12d
	movq	%r9, %rdx
	andl	$7, %edx
	andl	$7, %r12d
	subl	$1, 168(%rbx,%rdx,4)
	cmpb	$6, %r12b
	je	.L864
	movl	%r9d, %r10d
	leaq	_ZL15PIECE_VALUES_MG(%rip), %r15
	shrb	$3, %r10b
	movl	(%r15,%rdx,4), %edx
	andl	$1, %r10d
	subl	%edx, 208(%rbx,%r10,4)
	movl	(%rsi), %edx
	cmpb	$1, %r12b
	jne	.L562
	movl	%edx, %r12d
	leaq	MAILBOX_MAPS(%rip), %r14
	sarl	$7, %r12d
	andl	$127, %r12d
	movslq	256(%r14,%r12,4), %r12
	testl	%r12d, %r12d
	js	.L562
	leaq	_ZL10CLEAR_MASK(%rip), %r14
	movq	(%r14,%r12,8), %r12
	andq	%r12, 144(%rbx,%r10,8)
	andq	%r12, 160(%rbx)
	jmp	.L562
	.p2align 4,,10
	.p2align 3
.L563:
	subl	$1, 168(%rbx)
.L864:
	movl	(%rsi), %edx
	jmp	.L562
	.p2align 4,,10
	.p2align 3
.L529:
	cmpl	$12416, %ecx
	je	.L533
	movzbl	91(%rbx), %eax
	movb	$0, 91(%rbx)
	movb	%al, 94(%rbx)
	jmp	.L528
	.p2align 4,,10
	.p2align 3
.L899:
	testb	%al, %al
	jne	.L541
	movl	$1, %eax
.L542:
	movb	%al, 120(%rbx)
	testb	%r9b, %r9b
	je	.L863
.L556:
	movl	$1, %r11d
	xorl	%ebp, %ebp
	xorl	%ecx, %ecx
	movl	$2, %eax
	jmp	.L561
.L854:
	movl	$4, %r11d
	.p2align 4
	.p2align 3
.L455:
	movl	%r8d, %r10d
	addw	$1, 130(%rbx)
	shrb	$3, %r10b
	andl	$1, %r10d
	jmp	.L491
	.p2align 4,,10
	.p2align 3
.L901:
	movl	%edx, %ecx
	leaq	MAILBOX_MAPS(%rip), %r10
	sarl	$7, %edx
	andl	$127, %ecx
	andl	$127, %edx
	movslq	256(%r10,%rcx,4), %rcx
	movslq	256(%r10,%rdx,4), %rdx
	movl	%ecx, %r15d
	orl	%edx, %r15d
	js	.L558
	leaq	_ZL10CLEAR_MASK(%rip), %r10
	movzbl	%al, %eax
	movq	(%r10,%rcx,8), %rcx
	leaq	_ZL8BIT_MASK(%rip), %r10
	movq	(%r10,%rdx,8), %r10
	movq	144(%rbx,%rax,8), %rdx
	andq	%rcx, %rdx
	orq	%r10, %rdx
	movq	%rdx, 144(%rbx,%rax,8)
	andq	160(%rbx), %rcx
	orq	%r10, %rcx
	movq	%rcx, 160(%rbx)
	jmp	.L558
	.p2align 4,,10
	.p2align 3
.L544:
	cmpb	$1, %al
	je	.L554
	movl	$2, %eax
.L555:
	movl	%r8d, %ecx
	movb	%al, 120(%rbx)
	andl	$7, %ecx
	jmp	.L545
	.p2align 4,,10
	.p2align 3
.L843:
	imulq	$70, %r11, %r11
	leaq	(%rcx,%rcx,4), %rcx
	leaq	(%r11,%rcx,2), %rcx
	addq	%rax, %rcx
	movl	%edx, 216(%rbx,%rcx,4)
	addl	$1, 776(%rbx,%rbp,4)
	movl	(%rsi), %eax
	movl	%eax, %ebp
	movl	%eax, %edx
	sarl	$20, %ebp
	sarl	$7, %edx
	movl	%ebp, %r12d
	andl	$127, %edx
	andl	$15, %r12d
	jne	.L514
	jmp	.L515
	.p2align 4,,10
	.p2align 3
.L628:
	movl	$1, %eax
	.p2align 4
	.p2align 3
.L536:
	imulq	$70, %r10, %r10
	leaq	40(%rax,%r10), %rax
	movl	%r11d, 216(%rbx,%rax,4)
	jmp	.L528
.L600:
	xorl	%r15d, %r15d
	.p2align 4
	.p2align 3
.L506:
	leal	-1(%rdx), %eax
	imulq	$28, %rcx, %rdx
	imulq	$70, %rcx, %rcx
	movl	%eax, 780(%rbx,%rdx)
	cltq
	leaq	10(%rax,%rcx), %rax
	movl	216(%rbx,%rax,4), %ebp
	leaq	10(%rcx,%r15), %rax
	movl	%ebp, 216(%rbx,%rax,4)
	movslq	780(%rbx,%rdx), %rax
	leaq	10(%rcx,%rax), %rax
	movl	$-1, 216(%rbx,%rax,4)
	jmp	.L505
	.p2align 4,,10
	.p2align 3
.L530:
	cmpl	$12416, %ecx
	je	.L535
	movzbl	91(%rbx), %eax
	movb	$0, 91(%rbx)
	movl	$94, %r11d
	movl	$91, %edx
	movb	%al, 94(%rbx)
.L573:
	movl	820(%rbx), %eax
	testl	%eax, %eax
	jle	.L528
	cmpl	%edx, 656(%rbx)
	je	.L627
	cmpl	$1, %eax
	je	.L528
	movl	$1, %r10d
	jmp	.L572
	.p2align 4,,10
	.p2align 3
.L430:
	movl	%eax, %r8d
	andl	$16256, %r8d
	testb	$8, %r9b
	jne	.L463
	cmpl	$2688, %r8d
	je	.L903
	cmpl	$3584, %r8d
	jne	.L462
	andb	$-2, 128(%rbx)
	jmp	.L462
	.p2align 4,,10
	.p2align 3
.L543:
	movl	(%rsi), %edx
	movl	%edx, %r10d
	sarl	$7, %edx
	andl	$127, %r10d
	andl	$127, %edx
	cmpl	$119, %r10d
	jg	.L904
	leaq	FILE_RANK_LOOKUPS(%rip), %rbp
	movzbl	120(%rbp,%r10), %r12d
	cmpl	$119, %edx
	jg	.L549
.L548:
	movzbl	120(%rbp,%rdx), %r15d
	movl	%r15d, %r11d
	subl	%r12d, %r11d
	movl	%r11d, %r10d
	negl	%r10d
	cmovs	%r11d, %r10d
	cmpl	$2, %r10d
	jne	.L862
	addl	%r15d, %r12d
	movzbl	0(%rbp,%rdx), %edx
	sarl	%r12d
	leal	(%r12,%r12,4), %r10d
	leal	21(%rdx,%r10,2), %edx
.L550:
	movl	%edx, 124(%rbx)
	jmp	.L862
	.p2align 4,,10
	.p2align 3
.L891:
	subq	%rcx, %rdx
	leaq	832(%rbx), %rcx
	call	_ZNSt6vectorI6S_UNDOSaIS0_EE17_M_default_appendEy
	movslq	856(%rbx), %rax
	movq	832(%rbx), %rdi
	jmp	.L411
.L887:
	andl	$-13, %ebp
	xorl	%r13d, %r13d
	movb	%bpl, 128(%rbx)
	movw	%r13w, 130(%rbx)
	cmpb	$-1, %r9b
	je	.L478
	movl	%r9d, %ebp
	xorl	%r8d, %r8d
	movl	$2, %r10d
	andl	$7, %ebp
	jne	.L496
	movl	(%rsi), %eax
	movl	%eax, %ecx
	testl	$262144, %eax
	jne	.L482
	jmp	.L501
.L910:
	xorl	%ecx, %ecx
	movw	%cx, 130(%rbx)
.L478:
	movl	(%rsi), %eax
	xorl	%r8d, %r8d
	movl	%eax, %ecx
	testl	$262144, %eax
	jne	.L905
.L494:
	testl	$15728640, %eax
	je	.L847
	movl	$-1, %r9d
	jmp	.L513
	.p2align 4,,10
	.p2align 3
.L879:
	movl	%eax, %r8d
	sarl	$20, %r8d
	andl	$15, %r8d
	je	.L906
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	jmp	.L513
	.p2align 4,,10
	.p2align 3
.L541:
	cmpb	$1, %al
	je	.L553
	movl	$2, %eax
	jmp	.L542
	.p2align 4,,10
	.p2align 3
.L554:
	addw	$1, 132(%rbx)
	xorl	%eax, %eax
	jmp	.L555
	.p2align 4,,10
	.p2align 3
.L882:
	testb	$8, %r10b
	jne	.L443
	cmpl	$21, %edx
	je	.L907
	cmpl	$28, %edx
	jne	.L846
	movzbl	128(%rbx), %r10d
	movl	%r10d, %ebp
	andl	$-2, %ebp
	movb	%bpl, 128(%rbx)
	testb	%r9b, %r9b
	jne	.L448
	testl	%r11d, %r11d
	je	.L854
	movzbl	28(%rbx), %r11d
	testb	%r11b, %r11b
	je	.L472
	cmpb	$-1, %r11b
	jne	.L471
	andl	$-14, %r10d
	movb	%r10b, 128(%rbx)
	movl	(%rsi), %eax
	jmp	.L461
	.p2align 4,,10
	.p2align 3
.L443:
	cmpl	$91, %edx
	je	.L908
	cmpl	$98, %edx
	jne	.L846
	movzbl	128(%rbx), %r10d
	movl	%r10d, %ebp
	andl	$-5, %ebp
	movb	%bpl, 128(%rbx)
	testb	%r9b, %r9b
	jne	.L448
	testl	%r11d, %r11d
	je	.L854
	movzbl	98(%rbx), %r11d
	testb	%r11b, %r11b
	je	.L470
	cmpb	$-1, %r11b
	jne	.L471
	andl	$-13, %r10d
	movb	%r10b, 128(%rbx)
	movl	(%rsi), %eax
	jmp	.L461
	.p2align 4,,10
	.p2align 3
.L533:
	movzbl	98(%rbx), %eax
	movb	$0, 98(%rbx)
	movb	%al, 96(%rbx)
	jmp	.L528
	.p2align 4,,10
	.p2align 3
.L885:
	testl	%r11d, %r11d
	jne	.L428
	xorl	%r8d, %r8d
	movw	%r8w, 130(%rbx)
.L438:
	movl	%eax, %ecx
	movl	$-1, %r8d
	testl	$262144, %eax
	je	.L494
	xorl	%r11d, %r11d
	movl	$-1, %r8d
	movl	$2, %r10d
	movl	$-1, %r9d
	jmp	.L482
.L448:
	cmpb	$-1, %r9b
	jne	.L424
	.p2align 4
	.p2align 3
.L849:
	testl	%r11d, %r11d
	je	.L426
.L428:
	movzbl	128(%rbx), %ebp
	movslq	%edx, %r12
	cmpl	$119, %edx
	jle	.L437
	andl	$-13, %ebp
	xorl	%r15d, %r15d
	movb	%bpl, 128(%rbx)
	movl	(%rsi), %eax
	movw	%r15w, 130(%rbx)
	jmp	.L438
	.p2align 4,,10
	.p2align 3
.L463:
	cmpl	$11648, %r8d
	je	.L909
	cmpl	$12544, %r8d
	jne	.L462
	andb	$-5, 128(%rbx)
	jmp	.L462
	.p2align 4,,10
	.p2align 3
.L893:
	testl	%r11d, %r11d
	jne	.L425
.L426:
	movzbl	(%rbx,%r12), %r8d
	testb	%r8b, %r8b
	je	.L910
	cmpb	$-1, %r8b
	je	.L911
	movl	%r8d, %r11d
	andl	$7, %r11d
	cmpb	$1, %r11b
	jne	.L488
	movl	%r8d, %r10d
	xorl	%r12d, %r12d
	shrb	$3, %r10b
	movw	%r12w, 130(%rbx)
	andl	$1, %r10d
	jmp	.L839
	.p2align 4,,10
	.p2align 3
.L895:
	andl	$-4, %ebp
	movb	%bpl, 128(%rbx)
	testb	%r9b, %r9b
	jne	.L419
	jmp	.L829
	.p2align 4,,10
	.p2align 3
.L896:
	testl	%r11d, %r11d
	jne	.L445
.L461:
	addw	$1, 130(%rbx)
	movl	%eax, %ecx
	jmp	.L486
	.p2align 4,,10
	.p2align 3
.L906:
	sarl	$7, %eax
	movl	%ecx, %edx
	xorl	%r9d, %r9d
	andl	$127, %eax
	andl	$127, %edx
	jmp	.L522
	.p2align 4,,10
	.p2align 3
.L553:
	addw	$1, 132(%rbx)
	xorl	%eax, %eax
	jmp	.L542
.L597:
	xorl	%r11d, %r11d
	movl	$-1, %r8d
	movl	$2, %r10d
	xorl	%r9d, %r9d
	jmp	.L482
.L425:
	movzbl	128(%rbx), %ebp
	jmp	.L437
.L905:
	xorl	%r11d, %r11d
	xorl	%r8d, %r8d
	movl	$2, %r10d
	movl	$-1, %r9d
	jmp	.L482
.L908:
	andb	$-9, 128(%rbx)
	testb	%r9b, %r9b
	jne	.L419
	jmp	.L829
.L909:
	andb	$-9, 128(%rbx)
	jmp	.L462
.L903:
	andb	$-3, 128(%rbx)
	jmp	.L462
.L897:
	movslq	%edx, %r12
	jmp	.L434
.L898:
	movl	(%rsi), %eax
	xorl	%r11d, %r11d
	movl	$-1, %r8d
	movl	$2, %r10d
	movl	%eax, %ecx
	testl	$262144, %eax
	jne	.L482
	jmp	.L501
	.p2align 4,,10
	.p2align 3
.L531:
	movzbl	28(%rbx), %eax
	movl	$28, %edx
	movb	$0, 28(%rbx)
	movl	$26, %r11d
	movb	%al, 26(%rbx)
	jmp	.L532
.L888:
	andl	$-13, %ebp
	xorl	%r14d, %r14d
	movb	%bpl, 128(%rbx)
	movw	%r14w, 130(%rbx)
	cmpb	$-1, %r9b
	jne	.L473
	movl	(%rsi), %eax
	jmp	.L438
	.p2align 4,,10
	.p2align 3
.L904:
	cmpl	$119, %edx
	jg	.L862
	movl	$255, %r12d
	leaq	FILE_RANK_LOOKUPS(%rip), %rbp
	jmp	.L548
.L907:
	andb	$-3, 128(%rbx)
	testb	%r9b, %r9b
	jne	.L419
	jmp	.L829
.L549:
	cmpl	$253, %r12d
	jne	.L862
	movl	$2816, %edx
	jmp	.L550
.L847:
	movl	%eax, %edx
	sarl	$7, %eax
	movl	$-1, %r9d
	andl	$127, %edx
	andl	$127, %eax
	jmp	.L522
.L588:
	xorl	%r12d, %r12d
	.p2align 4
	.p2align 3
.L498:
	leal	-1(%rdx), %eax
	movq	%r13, %rdx
	subq	%rcx, %rdx
	imulq	$70, %rcx, %rcx
	addq	%rbp, %rdx
	addq	%r14, %rbp
	movl	%eax, 776(%rbx,%rdx,4)
	cltq
	leaq	(%rcx,%rbp,2), %rcx
	addq	%rcx, %rax
	addq	%rcx, %r12
	movl	216(%rbx,%rax,4), %eax
	movl	%eax, 216(%rbx,%r12,4)
	movslq	776(%rbx,%rdx,4), %rax
	addq	%rax, %rcx
	movl	$-1, 216(%rbx,%rcx,4)
	movl	(%rsi), %eax
	movl	%eax, %ecx
	jmp	.L491
.L618:
	xorl	%ecx, %ecx
	.p2align 4
	.p2align 3
.L523:
	imulq	$70, %r10, %r10
	leaq	(%r14,%r11), %rdx
	leaq	(%r10,%rdx,2), %rdx
	addq	%rcx, %rdx
	movl	%eax, 216(%rbx,%rdx,4)
	movl	(%rsi), %ecx
	movl	%ecx, %eax
	movl	%ecx, %edx
	sarl	$7, %eax
	andl	$127, %edx
	andl	$127, %eax
	jmp	.L522
.L609:
	xorl	%r12d, %r12d
	.p2align 4
	.p2align 3
.L511:
	imulq	$28, %r11, %rax
	subl	$1, %ecx
	imulq	$70, %r11, %rdx
	movl	%ecx, 780(%rbx,%rax)
	movslq	%ecx, %rcx
	leaq	10(%rcx,%rdx), %rcx
	movl	216(%rbx,%rcx,4), %ebp
	leaq	10(%r12,%rdx), %rcx
	movl	%ebp, 216(%rbx,%rcx,4)
	movslq	780(%rbx,%rax), %rax
	leaq	10(%rdx,%rax), %rax
	movl	$-1, 216(%rbx,%rax,4)
	movl	(%rsi), %eax
	movl	%eax, %ebp
	movl	%eax, %edx
	movl	%eax, %ecx
	sarl	$20, %ebp
	sarl	$7, %edx
	movl	%ebp, %r12d
	andl	$127, %edx
	andl	$15, %r12d
	jne	.L574
	jmp	.L513
	.p2align 4,,10
	.p2align 3
.L610:
	movl	$1, %r12d
	jmp	.L511
.L619:
	movl	$1, %ecx
	jmp	.L523
.L589:
	movl	$1, %r12d
	jmp	.L498
.L892:
	movl	%eax, %edx
	sarl	$7, %eax
	movl	$-1, %r8d
	andl	$127, %edx
	andl	$127, %eax
	jmp	.L522
.L627:
	movl	$1, %r10d
	xorl	%eax, %eax
	jmp	.L536
.L620:
	movl	$2, %ecx
	jmp	.L523
.L611:
	movl	$2, %r12d
	jmp	.L511
.L629:
	movl	$2, %eax
	jmp	.L536
.L590:
	movl	$2, %r12d
	jmp	.L498
.L630:
	movl	$3, %eax
	jmp	.L536
.L591:
	movl	$3, %r12d
	jmp	.L498
.L621:
	movl	$3, %ecx
	jmp	.L523
.L612:
	movl	$3, %r12d
	jmp	.L511
.L601:
	movl	$1, %r15d
	jmp	.L506
.L613:
	movl	$4, %r12d
	jmp	.L511
.L602:
	movl	$2, %r15d
	jmp	.L506
.L592:
	movl	$4, %r12d
	jmp	.L498
.L622:
	movl	$4, %ecx
	jmp	.L523
.L631:
	movl	$4, %eax
	jmp	.L536
.L632:
	movl	$5, %eax
	jmp	.L536
.L623:
	movl	$5, %ecx
	jmp	.L523
.L603:
	movl	$3, %r15d
	jmp	.L506
.L614:
	movl	$5, %r12d
	jmp	.L511
.L593:
	movl	$5, %r12d
	jmp	.L498
.L883:
	andl	$-13, %ebp
	movb	%bpl, 128(%rbx)
	movl	(%rsi), %eax
	jmp	.L442
.L594:
	movl	$6, %r12d
	jmp	.L498
.L615:
	movl	$6, %r12d
	jmp	.L511
.L604:
	movl	$4, %r15d
	jmp	.L506
.L624:
	movl	$6, %ecx
	jmp	.L523
.L633:
	movl	$6, %eax
	jmp	.L536
.L634:
	movl	$7, %eax
	jmp	.L536
.L595:
	movl	$7, %r12d
	jmp	.L498
.L616:
	movl	$7, %r12d
	jmp	.L511
.L605:
	movl	$5, %r15d
	jmp	.L506
.L625:
	movl	$7, %ecx
	jmp	.L523
.L606:
	movl	$6, %r15d
	jmp	.L506
.L635:
	movl	$8, %eax
	jmp	.L536
.L596:
	movl	$8, %r12d
	jmp	.L498
.L617:
	movl	$8, %r12d
	jmp	.L511
.L626:
	movl	$8, %ecx
	jmp	.L523
.L900:
	movl	$9, %ecx
	jmp	.L523
.L889:
	movl	$9, %r12d
	jmp	.L498
.L880:
	movl	$9, %r12d
	jmp	.L511
.L607:
	movl	$7, %r15d
	jmp	.L506
.L608:
	movl	$8, %r15d
	jmp	.L506
.L570:
	movl	$-1, 124(%rbx)
	movzbl	120(%rbx), %eax
	jmp	.L540
.L470:
	andl	$-13, %r10d
	movb	%r10b, 128(%rbx)
	movl	(%rsi), %eax
	jmp	.L442
.L472:
	andl	$-14, %r10d
	movb	%r10b, 128(%rbx)
	movl	(%rsi), %eax
	jmp	.L442
.L911:
	movw	$0, 130(%rbx)
	movl	(%rsi), %eax
	jmp	.L438
.L886:
	movw	$0, 130(%rbx)
	andl	$-13, %ebp
	movb	%bpl, 128(%rbx)
.L467:
	xorl	%r11d, %r11d
	movl	$-1, %r8d
	movl	$2, %r10d
	jmp	.L485
.L535:
	movzbl	98(%rbx), %eax
	movl	$96, %r11d
	movb	$0, 98(%rbx)
	movl	$98, %edx
	movb	%al, 96(%rbx)
	jmp	.L573
.L902:
	testb	%r9b, %r9b
	jne	.L838
	movl	(%rsi), %eax
	jmp	.L461
.L838:
	movw	$0, 130(%rbx)
	jmp	.L467
	.seh_endproc
	.text
	.p2align 4
	.globl	_Z29generate_legal_moves_enhancedRK8PositionR10S_MOVELIST
	.def	_Z29generate_legal_moves_enhancedRK8PositionR10S_MOVELIST;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z29generate_legal_moves_enhancedRK8PositionR10S_MOVELIST
_Z29generate_legal_moves_enhancedRK8PositionR10S_MOVELIST:
.LFB3082:
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
	subq	$3032, %rsp
	.seh_stackalloc	3032
	.seh_endprologue
	pxor	%xmm0, %xmm0
	movq	%rdx, %r13
	movq	%rcx, %rbx
	leaq	960(%rsp), %rbp
	leaq	3008(%rsp), %rdx
	leaq	960(%rsp), %rax
	.p2align 4
	.p2align 4
	.p2align 3
.L913:
	movups	%xmm0, (%rax)
	addq	$32, %rax
	movups	%xmm0, -16(%rax)
	cmpq	%rdx, %rax
	jne	.L913
	leaq	960(%rsp), %rdx
	movq	%rbx, %rcx
	movl	$0, 3008(%rsp)
.LEHB0:
	call	_Z18generate_all_movesRK8PositionR10S_MOVELIST
	movl	3008(%rsp), %eax
	movl	$0, 2048(%r13)
	testl	%eax, %eax
	jle	.L912
	leaq	312(%rsp), %rax
	xorl	%r12d, %r12d
	movq	%rax, 48(%rsp)
	leaq	216(%rbx), %rax
	movq	%rax, 40(%rsp)
	leaq	87(%rsp), %rax
	movq	%rax, 56(%rsp)
	.p2align 4
	.p2align 3
.L914:
	movq	112(%rbx), %rax
	movdqu	(%rbx), %xmm1
	movl	$70, %ecx
	movdqu	16(%rbx), %xmm2
	movdqu	32(%rbx), %xmm3
	movdqu	48(%rbx), %xmm4
	movdqu	64(%rbx), %xmm5
	movups	%xmm1, 96(%rsp)
	movq	%rax, 208(%rsp)
	movzbl	120(%rbx), %eax
	movdqu	80(%rbx), %xmm1
	movups	%xmm2, 112(%rsp)
	movdqu	96(%rbx), %xmm2
	movb	%al, 216(%rsp)
	movl	124(%rbx), %eax
	movups	%xmm3, 128(%rsp)
	movdqu	144(%rbx), %xmm3
	movl	%eax, 220(%rsp)
	movzbl	128(%rbx), %eax
	movups	%xmm4, 144(%rsp)
	movdqu	168(%rbx), %xmm4
	movb	%al, 224(%rsp)
	movl	130(%rbx), %eax
	movups	%xmm1, 176(%rsp)
	movl	%eax, 226(%rsp)
	movq	136(%rbx), %rax
	movups	%xmm2, 192(%rsp)
	movq	%rax, 232(%rsp)
	movq	160(%rbx), %rax
	movups	%xmm3, 240(%rsp)
	movq	%rax, 256(%rsp)
	movups	%xmm5, 160(%rsp)
	movups	%xmm4, 264(%rsp)
	movdqu	180(%rbx), %xmm5
	movq	200(%rbx), %rax
	movq	48(%rsp), %rdi
	movq	40(%rsp), %rsi
	movq	%rax, 296(%rsp)
	movq	208(%rbx), %rax
	rep movsq
	movdqu	776(%rbx), %xmm1
	movdqu	792(%rbx), %xmm2
	movups	%xmm5, 276(%rsp)
	movq	%rax, 304(%rsp)
	movdqu	808(%rbx), %xmm3
	movq	824(%rbx), %rax
	movq	832(%rbx), %rdi
	movups	%xmm1, 872(%rsp)
	movq	840(%rbx), %rsi
	movups	%xmm2, 888(%rsp)
	movq	%rax, 920(%rsp)
	movups	%xmm3, 904(%rsp)
	subq	%rdi, %rsi
	je	.L1177
	movq	%rsi, %rcx
	call	_Znwy
.LEHE0:
	movq	%rsi, %r8
	movq	%rdi, %rdx
	movq	%rax, %xmm4
	leaq	(%rax,%rsi), %r15
	movq	%rax, %rcx
	movddup	%xmm4, %xmm0
	movq	%r15, 944(%rsp)
	movups	%xmm0, 928(%rsp)
	call	memcpy
.L916:
	movl	856(%rbx), %eax
	movq	%rbp, %rdx
	leaq	96(%rsp), %rcx
	movq	%r15, 936(%rsp)
	movl	%eax, 952(%rsp)
.LEHB1:
	call	_ZN8Position19make_move_with_undoERK6S_MOVE
	movzbl	120(%rbx), %eax
	movl	$1, %r8d
	testb	%al, %al
	je	.L917
	cmpb	$1, %al
	setne	%r8b
	addl	%r8d, %r8d
.L917:
	movl	232(%rsp,%rax,4), %ecx
	leaq	96(%rsp), %rdx
	call	_Z10SqAttackediRK8Position5Color
	testb	%al, %al
	jne	.L918
	movslq	2048(%r13), %rdx
	movq	0(%rbp), %rcx
	movq	%rdx, %rax
	movq	%rcx, 0(%r13,%rdx,8)
	movl	$0, 4(%r13,%rdx,8)
	addl	$1, %eax
	movl	%eax, 2048(%r13)
.L918:
	movl	952(%rsp), %eax
	testl	%eax, %eax
	je	.L920
	subl	$1, %eax
	movq	56(%rsp), %r9
	leaq	88(%rsp), %rdx
	leaq	92(%rsp), %r8
	movl	%eax, 952(%rsp)
	cltq
	imulq	$728, %rax, %rsi
	addq	928(%rsp), %rsi
	movl	(%rsi), %ecx
	call	_ZN6S_MOVE11decode_moveEiRiS0_R9PieceType
.LEHE1:
	movl	92(%rsp), %eax
	movslq	88(%rsp), %rdx
	movzbl	87(%rsp), %r9d
	cmpl	$119, %eax
	ja	.L1178
	movslq	%eax, %r8
	movzbl	96(%rsp,%r8), %ecx
	testb	%r9b, %r9b
	je	.L924
	testb	%cl, %cl
	je	.L925
	cmpb	$-1, %cl
	je	.L926
	movl	%ecx, %r10d
	movl	%ecx, %r11d
	shrb	$3, %r10b
	andl	$1, %r10d
	andl	$7, %r11d
	je	.L927
	leaq	0(,%r10,8), %rdi
	movzbl	%r11b, %r11d
	movq	%rdi, %r9
	movq	%rdi, 72(%rsp)
	subq	%r10, %r9
	addq	%r11, %r9
	movl	872(%rsp,%r9,4), %r9d
	testl	%r9d, %r9d
	jle	.L927
	leaq	0(,%r11,4), %rdi
	leaq	(%rdi,%r11), %r15
	movq	%rdi, 64(%rsp)
	imulq	$280, %r10, %rdi
	leaq	(%rdi,%r15,8), %rdi
	cmpl	312(%rsp,%rdi), %eax
	je	.L1015
	cmpl	$1, %r9d
	je	.L927
	cmpl	316(%rsp,%rdi), %eax
	je	.L1016
	cmpl	$2, %r9d
	je	.L927
	imulq	$280, %r10, %rdi
	leaq	(%rdi,%r15,8), %rdi
	cmpl	320(%rsp,%rdi), %eax
	je	.L1017
	cmpl	$3, %r9d
	je	.L927
	cmpl	324(%rsp,%rdi), %eax
	je	.L1018
	cmpl	$4, %r9d
	je	.L927
	cmpl	328(%rsp,%rdi), %eax
	je	.L1019
	cmpl	$5, %r9d
	je	.L927
	cmpl	332(%rsp,%rdi), %eax
	je	.L1020
	cmpl	$6, %r9d
	je	.L927
	cmpl	336(%rsp,%rdi), %eax
	je	.L1021
	cmpl	$7, %r9d
	je	.L927
	imulq	$280, %r10, %rdi
	leaq	(%rdi,%r15,8), %rdi
	cmpl	340(%rsp,%rdi), %eax
	je	.L1022
	cmpl	$8, %r9d
	je	.L927
	cmpl	344(%rsp,%rdi), %eax
	je	.L1023
	cmpl	$9, %r9d
	je	.L927
	movl	$9, %r15d
	cmpl	348(%rsp,%rdi), %eax
	je	.L929
.L927:
	imulq	$28, %r10, %rdi
	andl	$8, %ecx
	movl	%ecx, %r9d
	orl	$1, %ecx
	movslq	876(%rsp,%rdi), %r11
	cmpl	$9, %r11d
	jg	.L1172
	imulq	$70, %r10, %r10
	leaq	10(%r11,%r10), %r10
	movl	%edx, 312(%rsp,%r10,4)
	addl	$1, 876(%rsp,%rdi)
.L1172:
	cmpl	$119, %edx
	ja	.L1179
.L949:
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	cmpb	$0, 240(%r9,%rdx)
	je	.L972
	movl	(%rsi), %r10d
	movb	%cl, 96(%rsp,%rdx)
	movl	%r10d, %edx
	testl	$16777216, %r10d
	je	.L966
.L973:
	movl	%ecx, %r9d
	andl	$8, %r9d
.L947:
	testb	%r9b, %r9b
	je	.L974
	cmpl	$97, %eax
	je	.L1180
	movzbl	190(%rsp), %edx
	movb	$0, 190(%rsp)
	movl	$91, %edi
	movl	$94, %r8d
	movb	%dl, 187(%rsp)
.L976:
	movl	$1, %r9d
.L977:
	imulq	$28, %r9, %rdx
	movl	888(%rsp,%rdx), %edx
	testl	%edx, %edx
	jle	.L979
	imulq	$280, %r9, %r11
	cmpl	%r8d, 472(%rsp,%r11)
	je	.L1034
	cmpl	$1, %edx
	je	.L979
	cmpl	%r8d, 476(%rsp,%r11)
	je	.L1035
	cmpl	$2, %edx
	je	.L979
	cmpl	480(%rsp,%r11), %r8d
	je	.L1036
	cmpl	$3, %edx
	je	.L979
	cmpl	484(%rsp,%r11), %r8d
	je	.L1037
	cmpl	$4, %edx
	je	.L979
	cmpl	488(%rsp,%r11), %r8d
	je	.L1038
	cmpl	$5, %edx
	je	.L979
	cmpl	492(%rsp,%r11), %r8d
	je	.L1039
	cmpl	$6, %edx
	je	.L979
	cmpl	496(%rsp,%r11), %r8d
	je	.L1040
	cmpl	$7, %edx
	je	.L979
	cmpl	500(%rsp,%r11), %r8d
	je	.L1041
	cmpl	$8, %edx
	je	.L979
	cmpl	504(%rsp,%r11), %r8d
	je	.L1042
	cmpl	$9, %edx
	je	.L979
	movl	$9, %edx
	cmpl	%r8d, 508(%rsp,%r11)
	je	.L980
.L979:
	movl	%r10d, %edx
.L966:
	andl	$262144, %edx
	je	.L984
	leal	-1(%rcx), %edx
	cmpb	$-3, %dl
	ja	.L983
.L985:
	andl	$8, %ecx
	leal	-10(%rax), %edx
	jne	.L983
.L987:
	cmpl	$119, %edx
	ja	.L991
.L1002:
	movslq	%edx, %rcx
	leaq	FILE_RANK_LOOKUPS(%rip), %rdi
	cmpb	$0, 240(%rdi,%rcx)
	je	.L991
.L989:
	movzbl	32(%rsi), %r8d
	movb	%r8b, 96(%rsp,%rcx)
.L991:
	cmpl	$119, %eax
	ja	.L990
	movslq	%eax, %r8
.L1166:
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
.L988:
	cmpb	$0, 240(%r9,%r8)
	je	.L990
	movb	$0, 96(%rsp,%r8)
.L990:
	movzbl	32(%rsi), %eax
	leal	-1(%rax), %ecx
	cmpb	$-3, %cl
	jbe	.L1181
	.p2align 4
	.p2align 3
.L920:
	movq	928(%rsp), %rcx
	testq	%rcx, %rcx
	je	.L996
	movq	944(%rsp), %rdx
	addl	$1, %r12d
	subq	%rcx, %rdx
	call	_ZdlPvy
	cmpl	3008(%rsp), %r12d
	jge	.L912
.L1163:
	addq	$8, %rbp
	jmp	.L914
.L925:
	movl	(%rsi), %r10d
	movl	%r10d, %ecx
	andl	$16777216, %ecx
	cmpl	$119, %edx
	ja	.L930
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	cmpb	$0, 240(%r9,%rdx)
	je	.L931
	movb	$0, 96(%rsp,%rdx)
	testl	%ecx, %ecx
	jne	.L932
.L1156:
	andl	$262144, %r10d
	je	.L1162
	.p2align 4
	.p2align 3
.L983:
	leal	10(%rax), %edx
	jmp	.L987
	.p2align 4,,10
	.p2align 3
.L996:
	addl	$1, %r12d
	cmpl	3008(%rsp), %r12d
	jl	.L1163
.L912:
	addq	$3032, %rsp
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
.L1177:
	movq	$0, 928(%rsp)
	xorl	%r15d, %r15d
	movq	$0, 944(%rsp)
	jmp	.L916
	.p2align 4,,10
	.p2align 3
.L1178:
	testb	%r9b, %r9b
	je	.L1182
	movl	(%rsi), %r10d
	movl	%r10d, %ecx
	andl	$16777216, %ecx
	cmpl	$119, %edx
	ja	.L1183
.L938:
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	cmpb	$0, 240(%r9,%rdx)
	je	.L941
	movb	$0, 96(%rsp,%rdx)
	testl	%ecx, %ecx
	je	.L943
.L932:
	cmpl	$97, %eax
	je	.L1184
.L939:
	movzbl	190(%rsp), %edx
	movb	$0, 190(%rsp)
	xorl	%ecx, %ecx
	movb	%dl, 187(%rsp)
.L1185:
	movl	%r10d, %edx
	jmp	.L966
.L1183:
	testl	%ecx, %ecx
	jne	.L939
.L943:
	andl	$262144, %r10d
	jne	.L983
	.p2align 4
	.p2align 3
.L984:
	movzbl	32(%rsi), %edx
	cmpl	$119, %eax
	ja	.L995
	movslq	%eax, %r8
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
.L982:
	cmpb	$0, 240(%r9,%r8)
	je	.L995
	movb	%dl, 96(%rsp,%r8)
.L1167:
	movzbl	32(%rsi), %edx
.L995:
	leal	-1(%rdx), %ecx
	cmpb	$-3, %cl
	ja	.L920
	movl	%edx, %r8d
	andl	$7, %r8d
	je	.L920
	shrb	$3, %dl
	movzbl	%r8b, %r8d
	andl	$1, %edx
	leaq	0(,%rdx,8), %rcx
	subq	%rdx, %rcx
	addq	%r8, %rcx
	movslq	872(%rsp,%rcx,4), %r9
	cmpl	$9, %r9d
	jg	.L920
	imulq	$70, %rdx, %rdx
	leaq	(%r8,%r8,4), %r8
	leaq	(%rdx,%r8,2), %rdx
	addq	%r9, %rdx
	movl	%eax, 312(%rsp,%rdx,4)
	addl	$1, 872(%rsp,%rcx,4)
	jmp	.L920
	.p2align 4,,10
	.p2align 3
.L924:
	testb	%cl, %cl
	je	.L951
	cmpb	$-1, %cl
	je	.L952
	movl	%ecx, %r10d
	andl	$7, %r10d
	je	.L954
	movl	%ecx, %r9d
	movzbl	%r10b, %r10d
	shrb	$3, %r9b
	andl	$1, %r9d
	leaq	(%r10,%r9,8), %r11
	subq	%r9, %r11
	movl	872(%rsp,%r11,4), %r11d
	testl	%r11d, %r11d
	jle	.L954
	leaq	0(,%r10,4), %rdi
	leaq	(%rdi,%r10), %r15
	movq	%rdi, 64(%rsp)
	imulq	$280, %r9, %rdi
	leaq	(%rdi,%r15,8), %rdi
	cmpl	312(%rsp,%rdi), %eax
	je	.L1024
	cmpl	$1, %r11d
	je	.L954
	cmpl	316(%rsp,%rdi), %eax
	je	.L1025
	cmpl	$2, %r11d
	je	.L954
	imulq	$280, %r9, %rdi
	leaq	(%rdi,%r15,8), %rdi
	cmpl	320(%rsp,%rdi), %eax
	je	.L1026
	cmpl	$3, %r11d
	je	.L954
	cmpl	324(%rsp,%rdi), %eax
	je	.L1027
	cmpl	$4, %r11d
	je	.L954
	cmpl	328(%rsp,%rdi), %eax
	je	.L1028
	cmpl	$5, %r11d
	je	.L954
	cmpl	332(%rsp,%rdi), %eax
	je	.L1029
	cmpl	$6, %r11d
	je	.L954
	cmpl	336(%rsp,%rdi), %eax
	je	.L1030
	cmpl	$7, %r11d
	je	.L954
	imulq	$280, %r9, %rdi
	leaq	(%rdi,%r15,8), %rdi
	cmpl	340(%rsp,%rdi), %eax
	je	.L1031
	cmpl	$8, %r11d
	je	.L954
	cmpl	344(%rsp,%rdi), %eax
	je	.L1032
	cmpl	$9, %r11d
	je	.L954
	movl	$9, %r11d
	cmpl	348(%rsp,%rdi), %eax
	je	.L955
.L954:
	cmpl	$119, %edx
	jbe	.L949
.L971:
	movl	(%rsi), %r10d
	testl	$16777216, %r10d
	jne	.L973
	andl	$262144, %r10d
	je	.L1001
	andl	$8, %ecx
	jne	.L983
.L1174:
	leal	-10(%rax), %edx
	cmpl	$119, %edx
	jbe	.L1002
	jmp	.L1166
	.p2align 4,,10
	.p2align 3
.L1181:
	shrb	$3, %al
	andl	$1, %eax
	imulq	$28, %rax, %r8
	movslq	876(%rsp,%r8), %rcx
	cmpl	$9, %ecx
	jg	.L920
	imulq	$70, %rax, %rax
	leaq	10(%rcx,%rax), %rax
	movl	%edx, 312(%rsp,%rax,4)
	addl	$1, 876(%rsp,%r8)
	jmp	.L920
.L972:
	movl	(%rsi), %r10d
	testl	$16777216, %r10d
	jne	.L973
	andl	$262144, %r10d
	jne	.L985
.L1162:
	movzbl	32(%rsi), %edx
	jmp	.L982
.L1182:
	cmpl	$119, %edx
	ja	.L962
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	cmpb	$0, 240(%r9,%rdx)
	jne	.L963
	movl	(%rsi), %r10d
	testl	$16777216, %r10d
	je	.L943
.L957:
	movzbl	190(%rsp), %edx
	movb	$0, 190(%rsp)
	movb	%dl, 187(%rsp)
.L958:
	movl	$-1, %ecx
	jmp	.L979
.L951:
	cmpl	$119, %edx
	ja	.L959
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	cmpb	$0, 240(%r9,%rdx)
	jne	.L960
	movl	(%rsi), %r10d
	testl	$16777216, %r10d
	je	.L1156
	cmpl	$97, %eax
	jne	.L939
.L1184:
	movzbl	192(%rsp), %edx
	xorl	%ecx, %ecx
	movb	$0, 192(%rsp)
	movb	%dl, 194(%rsp)
	jmp	.L1185
.L974:
	xorl	%r9d, %r9d
	movl	$28, %edi
	movl	$26, %r8d
	cmpl	$27, %eax
	je	.L977
	movl	$21, %edi
	movl	$24, %r8d
	jmp	.L977
.L962:
	movl	(%rsi), %r10d
	testl	$16777216, %r10d
	jne	.L957
	andl	$262144, %r10d
	je	.L1167
	leal	10(%rax), %edx
	cmpl	$119, %edx
	ja	.L990
	movslq	%edx, %rcx
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	cmpb	$0, 240(%r9,%rcx)
	jne	.L989
	jmp	.L990
	.p2align 4,,10
	.p2align 3
.L930:
	testl	%ecx, %ecx
	jne	.L932
	andl	$262144, %r10d
	je	.L1001
	leal	10(%rax), %edx
	cmpl	$119, %edx
	jg	.L1166
	movslq	%edx, %rcx
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	cmpb	$0, 240(%r9,%rcx)
	jne	.L989
	jmp	.L988
	.p2align 4,,10
	.p2align 3
.L959:
	movl	(%rsi), %r10d
	testl	$16777216, %r10d
	jne	.L932
.L1170:
	andl	$262144, %r10d
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	je	.L1162
.L1011:
	leal	10(%rax), %edx
	cmpl	$119, %edx
	jg	.L988
	movslq	%edx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	jne	.L989
	jmp	.L988
.L1015:
	xorl	%r15d, %r15d
.L929:
	movq	72(%rsp), %rdi
	subl	$1, %r9d
	subq	%r10, %rdi
	addq	%r11, %rdi
	movl	%r9d, 872(%rsp,%rdi,4)
	movslq	%r9d, %r9
	movq	%r9, 72(%rsp)
	movq	64(%rsp), %r9
	addq	%r11, %r9
	imulq	$70, %r10, %r11
	leaq	(%r11,%r9,2), %r11
	movq	72(%rsp), %r9
	addq	%r11, %r15
	addq	%r11, %r9
	movl	312(%rsp,%r9,4), %r9d
	movl	%r9d, 312(%rsp,%r15,4)
	movslq	872(%rsp,%rdi,4), %r9
	addq	%r9, %r11
	movl	$-1, 312(%rsp,%r11,4)
	jmp	.L927
.L1024:
	xorl	%r11d, %r11d
.L955:
	imulq	$70, %r9, %r9
	movq	64(%rsp), %rdi
	addq	%rdi, %r10
	leaq	(%r9,%r10,2), %r9
	addq	%r11, %r9
	movl	%edx, 312(%rsp,%r9,4)
	cmpl	$119, %edx
	jbe	.L949
	jmp	.L971
.L1034:
	xorl	%edx, %edx
.L980:
	imulq	$70, %r9, %r9
	leaq	40(%rdx,%r9), %rdx
	movl	%edi, 312(%rsp,%rdx,4)
	movl	(%rsi), %edx
	jmp	.L966
.L1179:
	movl	(%rsi), %r10d
	testl	$16777216, %r10d
	jne	.L947
	andl	$262144, %r10d
	je	.L1001
	testb	%r9b, %r9b
	jne	.L983
	jmp	.L1174
.L931:
	testl	%ecx, %ecx
	jne	.L932
	andl	$262144, %r10d
	jne	.L1011
	jmp	.L1162
.L960:
	movl	(%rsi), %r10d
	movb	$0, 96(%rsp,%rdx)
	movl	%r10d, %edx
	testl	$16777216, %r10d
	jne	.L932
	jmp	.L966
.L1180:
	movzbl	192(%rsp), %edx
	movl	$98, %edi
	movb	$0, 192(%rsp)
	movl	$96, %r8d
	movb	%dl, 194(%rsp)
	jmp	.L976
.L1001:
	movzbl	32(%rsi), %edx
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	jmp	.L982
.L926:
	movl	(%rsi), %r10d
	movl	%r10d, %ecx
	andl	$16777216, %ecx
	cmpl	$119, %edx
	jbe	.L938
.L941:
	testl	%ecx, %ecx
	jne	.L932
	jmp	.L943
.L952:
	cmpl	$119, %edx
	ja	.L1186
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	cmpb	$0, 240(%r9,%rdx)
	je	.L964
	movl	(%rsi), %r10d
	movb	$-1, 96(%rsp,%rdx)
	movl	%r10d, %edx
	testl	$16777216, %r10d
	je	.L966
.L965:
	cmpl	$97, %eax
	jne	.L957
	movzbl	192(%rsp), %edx
	movb	$0, 192(%rsp)
	movb	%dl, 194(%rsp)
	jmp	.L958
.L1035:
	movl	$1, %edx
	jmp	.L980
.L1036:
	movl	$2, %edx
	jmp	.L980
.L1028:
	movl	$4, %r11d
	jmp	.L955
.L1186:
	movl	(%rsi), %r10d
	testl	$16777216, %r10d
	jne	.L965
	jmp	.L1170
.L1037:
	movl	$3, %edx
	jmp	.L980
.L1039:
	movl	$5, %edx
	jmp	.L980
.L1038:
	movl	$4, %edx
	jmp	.L980
.L1026:
	movl	$2, %r11d
	jmp	.L955
.L1025:
	movl	$1, %r11d
	jmp	.L955
.L1016:
	movl	$1, %r15d
	jmp	.L929
.L1041:
	movl	$7, %edx
	jmp	.L980
.L1040:
	movl	$6, %edx
	jmp	.L980
.L1027:
	movl	$3, %r11d
	jmp	.L955
.L1042:
	movl	$8, %edx
	jmp	.L980
.L1029:
	movl	$5, %r11d
	jmp	.L955
.L1030:
	movl	$6, %r11d
	jmp	.L955
.L1021:
	movl	$6, %r15d
	jmp	.L929
.L1022:
	movl	$7, %r15d
	jmp	.L929
.L1023:
	movl	$8, %r15d
	jmp	.L929
.L1031:
	movl	$7, %r11d
	jmp	.L955
.L1017:
	movl	$2, %r15d
	jmp	.L929
.L1018:
	movl	$3, %r15d
	jmp	.L929
.L1019:
	movl	$4, %r15d
	jmp	.L929
.L1020:
	movl	$5, %r15d
	jmp	.L929
.L963:
	movb	$-1, 96(%rsp,%rdx)
	movl	(%rsi), %edx
	movl	$-1, %ecx
	movl	%edx, %r10d
	testl	$16777216, %edx
	jne	.L965
	jmp	.L966
.L1032:
	movl	$8, %r11d
	jmp	.L955
.L964:
	movl	(%rsi), %r10d
	testl	$16777216, %r10d
	jne	.L965
	jmp	.L943
.L1043:
	movq	928(%rsp), %rcx
	movq	%rax, %rbx
	testq	%rcx, %rcx
	je	.L1000
	movq	944(%rsp), %rdx
	subq	%rcx, %rdx
	call	_ZdlPvy
.L1000:
	movq	%rbx, %rcx
.LEHB2:
	call	_Unwind_Resume
	nop
.LEHE2:
	.seh_handler	__gxx_personality_seh0, @unwind, @except
	.seh_handlerdata
.LLSDA3082:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE3082-.LLSDACSB3082
.LLSDACSB3082:
	.uleb128 .LEHB0-.LFB3082
	.uleb128 .LEHE0-.LEHB0
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB1-.LFB3082
	.uleb128 .LEHE1-.LEHB1
	.uleb128 .L1043-.LFB3082
	.uleb128 0
	.uleb128 .LEHB2-.LFB3082
	.uleb128 .LEHE2-.LEHB2
	.uleb128 0
	.uleb128 0
.LLSDACSE3082:
	.text
	.seh_endproc
	.section .rdata,"dr"
	.align 32
_ZZ20generate_queen_movesRK8PositionR10S_MOVELIST5ColorE10directions:
	.long	10
	.long	-10
	.long	1
	.long	-1
	.long	11
	.long	9
	.long	-9
	.long	-11
	.align 16
_ZZ19generate_rook_movesRK8PositionR10S_MOVELIST5ColorE10directions:
	.long	10
	.long	-10
	.long	1
	.long	-1
	.align 16
_ZZ21generate_bishop_movesRK8PositionR10S_MOVELIST5ColorE10directions:
	.long	11
	.long	9
	.long	-9
	.long	-11
	.align 32
_ZL10CLEAR_MASK:
	.quad	-2
	.quad	-3
	.quad	-5
	.quad	-9
	.quad	-17
	.quad	-33
	.quad	-65
	.quad	-129
	.quad	-257
	.quad	-513
	.quad	-1025
	.quad	-2049
	.quad	-4097
	.quad	-8193
	.quad	-16385
	.quad	-32769
	.quad	-65537
	.quad	-131073
	.quad	-262145
	.quad	-524289
	.quad	-1048577
	.quad	-2097153
	.quad	-4194305
	.quad	-8388609
	.quad	-16777217
	.quad	-33554433
	.quad	-67108865
	.quad	-134217729
	.quad	-268435457
	.quad	-536870913
	.quad	-1073741825
	.quad	-2147483649
	.quad	-4294967297
	.quad	-8589934593
	.quad	-17179869185
	.quad	-34359738369
	.quad	-68719476737
	.quad	-137438953473
	.quad	-274877906945
	.quad	-549755813889
	.quad	-1099511627777
	.quad	-2199023255553
	.quad	-4398046511105
	.quad	-8796093022209
	.quad	-17592186044417
	.quad	-35184372088833
	.quad	-70368744177665
	.quad	-140737488355329
	.quad	-281474976710657
	.quad	-562949953421313
	.quad	-1125899906842625
	.quad	-2251799813685249
	.quad	-4503599627370497
	.quad	-9007199254740993
	.quad	-18014398509481985
	.quad	-36028797018963969
	.quad	-72057594037927937
	.quad	-144115188075855873
	.quad	-288230376151711745
	.quad	-576460752303423489
	.quad	-1152921504606846977
	.quad	-2305843009213693953
	.quad	-4611686018427387905
	.quad	9223372036854775807
	.align 32
_ZL8BIT_MASK:
	.quad	1
	.quad	2
	.quad	4
	.quad	8
	.quad	16
	.quad	32
	.quad	64
	.quad	128
	.quad	256
	.quad	512
	.quad	1024
	.quad	2048
	.quad	4096
	.quad	8192
	.quad	16384
	.quad	32768
	.quad	65536
	.quad	131072
	.quad	262144
	.quad	524288
	.quad	1048576
	.quad	2097152
	.quad	4194304
	.quad	8388608
	.quad	16777216
	.quad	33554432
	.quad	67108864
	.quad	134217728
	.quad	268435456
	.quad	536870912
	.quad	1073741824
	.quad	2147483648
	.quad	4294967296
	.quad	8589934592
	.quad	17179869184
	.quad	34359738368
	.quad	68719476736
	.quad	137438953472
	.quad	274877906944
	.quad	549755813888
	.quad	1099511627776
	.quad	2199023255552
	.quad	4398046511104
	.quad	8796093022208
	.quad	17592186044416
	.quad	35184372088832
	.quad	70368744177664
	.quad	140737488355328
	.quad	281474976710656
	.quad	562949953421312
	.quad	1125899906842624
	.quad	2251799813685248
	.quad	4503599627370496
	.quad	9007199254740992
	.quad	18014398509481984
	.quad	36028797018963968
	.quad	72057594037927936
	.quad	144115188075855872
	.quad	288230376151711744
	.quad	576460752303423488
	.quad	1152921504606846976
	.quad	2305843009213693952
	.quad	4611686018427387904
	.quad	-9223372036854775808
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
	.globl	KING_DELTAS
	.section	.rdata$KING_DELTAS,"dr"
	.linkonce same_size
	.align 32
KING_DELTAS:
	.long	10
	.long	-10
	.long	1
	.long	-1
	.long	11
	.long	9
	.long	-9
	.long	-11
	.globl	KNIGHT_DELTAS
	.section	.rdata$KNIGHT_DELTAS,"dr"
	.linkonce same_size
	.align 32
KNIGHT_DELTAS:
	.long	21
	.long	19
	.long	12
	.long	8
	.long	-8
	.long	-12
	.long	-19
	.long	-21
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
	.def	__gxx_personality_seh0;	.scl	2;	.type	32;	.endef
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.1.0"
	.def	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b;	.scl	2;	.type	32;	.endef
	.def	_Z10SqAttackediRK8Position5Color;	.scl	2;	.type	32;	.endef
	.def	_Znwy;	.scl	2;	.type	32;	.endef
	.def	_ZdlPvy;	.scl	2;	.type	32;	.endef
	.def	_ZSt20__throw_length_errorPKc;	.scl	2;	.type	32;	.endef
	.def	_ZN8Position18save_derived_stateER6S_UNDO;	.scl	2;	.type	32;	.endef
	.def	_ZN8Position23update_zobrist_for_moveERK6S_MOVE5PieceS3_hi;	.scl	2;	.type	32;	.endef
	.def	memcpy;	.scl	2;	.type	32;	.endef
	.def	_ZN6S_MOVE11decode_moveEiRiS0_R9PieceType;	.scl	2;	.type	32;	.endef
	.def	_Unwind_Resume;	.scl	2;	.type	32;	.endef
