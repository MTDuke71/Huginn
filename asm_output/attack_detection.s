	.file	"attack_detection.cpp"
	.text
	.p2align 4
	.globl	_Z10SqAttackediRK8Position5Color
	.def	_Z10SqAttackediRK8Position5Color;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z10SqAttackediRK8Position5Color
_Z10SqAttackediRK8Position5Color:
.LFB2795:
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
	movl	%ecx, %eax
	movl	%r8d, %ebx
	xorl	%ecx, %ecx
	cmpl	$119, %eax
	ja	.L1
	movslq	%eax, %r10
	leaq	FILE_RANK_LOOKUPS(%rip), %r9
	movzbl	240(%r9,%r10), %ecx
	testb	%cl, %cl
	je	.L1
	movzbl	%r8b, %r11d
	imulq	$28, %r11, %r8
	addq	%rdx, %r8
	movl	776(%r8), %r14d
	testl	%r14d, %r14d
	jg	.L357
	movl	780(%r8), %r13d
	testl	%r13d, %r13d
	jg	.L358
	movl	784(%r8), %r12d
	testl	%r12d, %r12d
	jg	.L5
	movl	788(%r8), %ebp
	testl	%ebp, %ebp
	jg	.L5
	movl	792(%r8), %edi
	testl	%edi, %edi
	jg	.L5
	movl	796(%r8), %esi
	testl	%esi, %esi
	jg	.L5
	movl	800(%r8), %r8d
	testl	%r8d, %r8d
	jg	.L5
	movl	$1, %r10d
	leaq	239(%r9), %rsi
	jmp	.L9
	.p2align 4,,10
	.p2align 3
.L360:
	movl	%r11d, %r8d
	movl	$2, %edi
	shrb	$3, %r8b
	andl	$1, %r8d
	cmpb	$-1, %r11b
	cmove	%edi, %r8d
	cmpb	%r8b, %bl
	sete	%r8b
	setne	%dil
.L7:
	cmpq	$120, %r10
	setne	%r11b
	addq	$1, %r10
	andb	%dil, %r11b
	je	.L359
.L9:
	movzbl	(%rsi,%r10), %r8d
	movl	%ecx, %edi
	testb	%r8b, %r8b
	je	.L7
	movzbl	-1(%rdx,%r10), %r11d
	testb	%r11b, %r11b
	jne	.L360
	movl	%r8d, %edi
	xorl	%r8d, %r8d
	jmp	.L7
	.p2align 4,,10
	.p2align 3
.L10:
	leal	9(%rax), %ecx
	cmpl	$119, %ecx
	jg	.L17
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L18
	cmpb	$9, (%rdx,%rcx)
	je	.L13
.L18:
	leal	11(%rax), %ecx
	cmpl	$119, %ecx
	jg	.L17
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L16
	cmpb	$9, (%rdx,%rcx)
	je	.L13
.L16:
	leal	21(%rax), %ecx
	cmpl	$119, %ecx
	jg	.L361
.L15:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L24
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L362
.L24:
	leal	19(%rax), %ecx
.L354:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L316
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L363
.L316:
	leal	12(%rax), %ecx
.L21:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L34
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L364
.L34:
	leal	8(%rax), %ecx
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L35
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L28
.L35:
	leal	-8(%rax), %ecx
	cmpl	$119, %ecx
	ja	.L36
.L29:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L38
.L30:
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L365
.L38:
	leal	-12(%rax), %ecx
	cmpl	$119, %ecx
	ja	.L36
.L31:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L40
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L366
.L40:
	leal	-19(%rax), %ecx
	cmpl	$119, %ecx
	ja	.L36
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L46
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L367
.L46:
	leal	-21(%rax), %ecx
	cmpl	$119, %ecx
	ja	.L47
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L48
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L49
.L48:
	leal	10(%rax), %ecx
	cmpl	$119, %ecx
	jg	.L50
.L341:
	movslq	%ecx, %rcx
	leal	-10(%rax), %r10d
	cmpb	$0, 240(%r9,%rcx)
	je	.L45
.L44:
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L368
.L52:
	leal	-10(%rax), %r10d
	leal	1(%rax), %ecx
	cmpl	$119, %r10d
	jbe	.L45
.L55:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L59
.L141:
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L369
.L59:
	leal	-1(%rax), %ecx
	cmpl	$119, %ecx
	ja	.L153
.L58:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L61
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L370
.L61:
	leal	11(%rax), %ecx
	cmpl	$119, %ecx
	jg	.L371
.L60:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L63
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L372
.L63:
	leal	9(%rax), %ecx
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L69
.L66:
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L373
.L69:
	leal	-9(%rax), %ecx
	cmpl	$119, %ecx
	ja	.L68
.L67:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L70
	movzbl	(%rdx,%rcx), %r10d
	leal	-1(%r10), %ecx
	cmpb	$-3, %cl
	jbe	.L374
.L70:
	leal	-11(%rax), %ecx
	cmpl	$119, %ecx
	ja	.L68
.L65:
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L68
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L375
.L68:
	movdqu	.LC0(%rip), %xmm0
	movl	%r8d, %r14d
	leaq	16(%rsp), %rsi
	movl	%eax, %r8d
	leaq	32(%rsp), %r10
	movups	%xmm0, 16(%rsp)
.L73:
	movl	(%rsi), %edi
	leal	(%r8,%rdi), %ecx
	cmpl	$119, %ecx
	ja	.L71
	movslq	%ecx, %rax
	cmpb	$0, 240(%r9,%rax)
	je	.L71
	movzbl	(%rdx,%rax), %eax
	testb	%al, %al
	jne	.L72
	addl	%edi, %ecx
	cmpl	$119, %ecx
	ja	.L71
	movslq	%ecx, %rax
	cmpb	$0, 240(%r9,%rax)
	je	.L71
	movzbl	(%rdx,%rax), %eax
	testb	%al, %al
	jne	.L72
	addl	%edi, %ecx
	cmpl	$119, %ecx
	ja	.L71
	movslq	%ecx, %rax
	cmpb	$0, 240(%r9,%rax)
	je	.L71
	movzbl	(%rdx,%rax), %eax
	testb	%al, %al
	jne	.L72
	addl	%edi, %ecx
	cmpl	$119, %ecx
	ja	.L71
	movslq	%ecx, %rax
	cmpb	$0, 240(%r9,%rax)
	je	.L71
	movzbl	(%rdx,%rax), %eax
	testb	%al, %al
	jne	.L72
	addl	%edi, %ecx
	cmpl	$119, %ecx
	ja	.L71
	movslq	%ecx, %rax
	cmpb	$0, 240(%r9,%rax)
	je	.L71
	movzbl	(%rdx,%rax), %eax
	testb	%al, %al
	jne	.L72
	addl	%edi, %ecx
	cmpl	$119, %ecx
	ja	.L71
	movslq	%ecx, %rax
	cmpb	$0, 240(%r9,%rax)
	je	.L71
	movzbl	(%rdx,%rax), %eax
	testb	%al, %al
	jne	.L72
	addl	%ecx, %edi
	cmpl	$119, %edi
	ja	.L71
	movslq	%edi, %rdi
	cmpb	$0, 240(%r9,%rdi)
	je	.L71
	movzbl	(%rdx,%rdi), %eax
	testb	%al, %al
	jne	.L72
	.p2align 4
	.p2align 3
.L71:
	addq	$4, %rsi
	cmpq	%rsi, %r10
	jne	.L73
	movdqu	.LC1(%rip), %xmm0
	movl	%r8d, %eax
	movl	%r14d, %r8d
	movups	%xmm0, 32(%rsp)
.L76:
	movl	(%r10), %esi
	leal	(%rax,%rsi), %ecx
	cmpl	$119, %ecx
	ja	.L74
	movslq	%ecx, %rdi
	cmpb	$0, 240(%r9,%rdi)
	je	.L74
	movzbl	(%rdx,%rdi), %edi
	testb	%dil, %dil
	jne	.L75
	addl	%esi, %ecx
	cmpl	$119, %ecx
	ja	.L74
	movslq	%ecx, %rdi
	cmpb	$0, 240(%r9,%rdi)
	je	.L74
	movzbl	(%rdx,%rdi), %edi
	testb	%dil, %dil
	jne	.L75
	addl	%esi, %ecx
	cmpl	$119, %ecx
	ja	.L74
	movslq	%ecx, %rdi
	cmpb	$0, 240(%r9,%rdi)
	je	.L74
	movzbl	(%rdx,%rdi), %edi
	testb	%dil, %dil
	jne	.L75
	addl	%esi, %ecx
	cmpl	$119, %ecx
	ja	.L74
	movslq	%ecx, %rdi
	cmpb	$0, 240(%r9,%rdi)
	je	.L74
	movzbl	(%rdx,%rdi), %edi
	testb	%dil, %dil
	jne	.L75
	addl	%esi, %ecx
	cmpl	$119, %ecx
	ja	.L74
	movslq	%ecx, %rdi
	cmpb	$0, 240(%r9,%rdi)
	je	.L74
	movzbl	(%rdx,%rdi), %edi
	testb	%dil, %dil
	jne	.L75
	addl	%esi, %ecx
	cmpl	$119, %ecx
	ja	.L74
	movslq	%ecx, %rdi
	cmpb	$0, 240(%r9,%rdi)
	je	.L74
	movzbl	(%rdx,%rdi), %edi
	testb	%dil, %dil
	jne	.L75
	addl	%ecx, %esi
	cmpl	$119, %esi
	ja	.L74
	movslq	%esi, %rsi
	cmpb	$0, 240(%r9,%rsi)
	je	.L74
	movzbl	(%rdx,%rsi), %edi
	testb	%dil, %dil
	jne	.L75
	.p2align 4
	.p2align 3
.L74:
	addq	$4, %r10
	leaq	48(%rsp), %rsi
	cmpq	%r10, %rsi
	jne	.L76
.L77:
	movl	%r11d, %ecx
.L1:
	movl	%ecx, %eax
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
.L359:
	xorl	%ecx, %ecx
	testb	%r8b, %r8b
	je	.L1
	testb	%bl, %bl
	jne	.L10
	leal	-9(%rax), %ecx
	cmpl	$119, %ecx
	ja	.L11
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L12
	cmpb	$1, (%rdx,%rcx)
	je	.L13
.L12:
	leal	-11(%rax), %ecx
	cmpl	$119, %ecx
	ja	.L11
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L16
	cmpb	$1, (%rdx,%rcx)
	jne	.L16
.L13:
	movl	%r8d, %r11d
	jmp	.L77
.L357:
	movl	780(%r8), %r8d
	testl	%r8d, %r8d
	jle	.L5
.L4:
	testb	%bl, %bl
	je	.L78
	imulq	$280, %r11, %rbx
	addq	%rdx, %rbx
	movl	256(%rbx), %esi
	leal	-9(%rsi), %edi
	cmpl	%edi, %eax
	je	.L1
	subl	$11, %esi
	cmpl	%esi, %eax
	je	.L1
	cmpl	$1, %r8d
	jle	.L5
	movl	260(%rbx), %esi
	leal	-9(%rsi), %edi
	cmpl	%edi, %eax
	je	.L1
	subl	$11, %esi
	cmpl	%esi, %eax
	je	.L1
	cmpl	$2, %r8d
	je	.L5
	movl	264(%rbx), %ebx
	leal	-9(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	subl	$11, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$3, %r8d
	je	.L5
	imulq	$280, %r11, %rbx
	addq	%rdx, %rbx
	movl	268(%rbx), %esi
	leal	-9(%rsi), %edi
	cmpl	%edi, %eax
	je	.L1
	subl	$11, %esi
	cmpl	%esi, %eax
	je	.L1
	cmpl	$4, %r8d
	je	.L5
	movl	272(%rbx), %esi
	leal	-9(%rsi), %edi
	cmpl	%edi, %eax
	je	.L1
	subl	$11, %esi
	cmpl	%esi, %eax
	je	.L1
	cmpl	$5, %r8d
	je	.L5
	movl	276(%rbx), %esi
	leal	-9(%rsi), %edi
	cmpl	%edi, %eax
	je	.L1
	subl	$11, %esi
	cmpl	%esi, %eax
	je	.L1
	cmpl	$6, %r8d
	je	.L5
	movl	280(%rbx), %ebx
	leal	-9(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	subl	$11, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$7, %r8d
	je	.L5
	imulq	$280, %r11, %rbx
	addq	%rdx, %rbx
	movl	284(%rbx), %esi
	leal	-9(%rsi), %edi
	cmpl	%edi, %eax
	je	.L1
	subl	$11, %esi
	cmpl	%esi, %eax
	je	.L1
	cmpl	$8, %r8d
	je	.L5
	movl	288(%rbx), %esi
	leal	-9(%rsi), %edi
	cmpl	%edi, %eax
	je	.L1
	subl	$11, %esi
	cmpl	%esi, %eax
	je	.L1
	cmpl	$9, %r8d
	je	.L5
	movl	292(%rbx), %r8d
	leal	-9(%r8), %ebx
	cmpl	%ebx, %eax
	je	.L1
	subl	$11, %r8d
	cmpl	%r8d, %eax
	jne	.L5
	jmp	.L1
	.p2align 4,,10
	.p2align 3
.L78:
	movl	256(%rdx), %ebx
	leal	11(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	addl	$9, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$1, %r8d
	jle	.L5
	movl	260(%rdx), %ebx
	leal	11(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	addl	$9, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$2, %r8d
	je	.L5
	movl	264(%rdx), %ebx
	leal	11(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	addl	$9, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$3, %r8d
	je	.L5
	movl	268(%rdx), %ebx
	leal	11(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	addl	$9, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$4, %r8d
	je	.L5
	movl	272(%rdx), %ebx
	leal	11(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	addl	$9, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$5, %r8d
	je	.L5
	movl	276(%rdx), %ebx
	leal	11(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	addl	$9, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$6, %r8d
	je	.L5
	movl	280(%rdx), %ebx
	leal	11(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	addl	$9, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$7, %r8d
	je	.L5
	movl	284(%rdx), %ebx
	leal	11(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	addl	$9, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$8, %r8d
	je	.L5
	movl	288(%rdx), %ebx
	leal	11(%rbx), %esi
	cmpl	%esi, %eax
	je	.L1
	addl	$9, %ebx
	cmpl	%ebx, %eax
	je	.L1
	cmpl	$9, %r8d
	je	.L5
	movl	292(%rdx), %r8d
	leal	11(%r8), %ebx
	cmpl	%ebx, %eax
	je	.L1
	addl	$9, %r8d
	cmpl	%r8d, %eax
	je	.L1
	.p2align 4
	.p2align 3
.L5:
	imulq	$28, %r11, %r8
	movslq	784(%rdx,%r8), %r8
	testl	%r8d, %r8d
	jle	.L79
	imulq	$70, %r11, %rsi
	movq	%rdx, 136(%rsp)
	imulq	$280, %r11, %rbx
	addq	%rsi, %r8
	addq	%rdx, %rbx
	leaq	(%rdx,%r8,4), %rdx
	jmp	.L80
	.p2align 4,,10
	.p2align 3
.L377:
	leal	19(%r8), %esi
	cmpl	%esi, %eax
	je	.L1
	leal	12(%r8), %esi
	cmpl	%esi, %eax
	je	.L1
	leal	8(%r8), %esi
	cmpl	%esi, %eax
	je	.L1
	leal	-8(%r8), %esi
	cmpl	%esi, %eax
	je	.L1
	leal	-12(%r8), %esi
	cmpl	%esi, %eax
	je	.L1
	leal	-19(%r8), %esi
	cmpl	%esi, %eax
	je	.L1
	subl	$21, %r8d
	cmpl	%r8d, %eax
	je	.L1
	addq	$4, %rbx
	cmpq	%rbx, %rdx
	je	.L376
.L80:
	movl	296(%rbx), %r8d
	leal	21(%r8), %esi
	cmpl	%esi, %eax
	jne	.L377
	jmp	.L1
.L361:
	leal	19(%rax), %ecx
	cmpl	$119, %ecx
	jle	.L354
.L19:
	leal	12(%rax), %ecx
	cmpl	$119, %ecx
	jle	.L21
	.p2align 4
	.p2align 3
.L17:
	leal	8(%rax), %ecx
	cmpl	$119, %ecx
	jg	.L26
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L26
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L28
	leal	-8(%rax), %ecx
	jmp	.L29
	.p2align 4,,10
	.p2align 3
.L11:
	leal	21(%rax), %ecx
	jmp	.L15
	.p2align 4,,10
	.p2align 3
.L45:
	movslq	%r10d, %r10
	cmpb	$0, 240(%r9,%r10)
	je	.L378
	movzbl	(%rdx,%r10), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L54
	leal	1(%rax), %ecx
	jmp	.L55
	.p2align 4,,10
	.p2align 3
.L36:
	leal	10(%rax), %ecx
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L52
	jmp	.L44
	.p2align 4,,10
	.p2align 3
.L26:
	leal	-8(%rax), %ecx
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	jne	.L30
	leal	-12(%rax), %ecx
	jmp	.L31
.L50:
	leal	-10(%rax), %ecx
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	jne	.L145
	leal	1(%rax), %ecx
	cmpl	$119, %eax
	je	.L57
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	jne	.L141
.L337:
	leal	-1(%rax), %ecx
	jmp	.L58
	.p2align 4,,10
	.p2align 3
.L47:
	leal	10(%rax), %ecx
	jmp	.L341
	.p2align 4,,10
	.p2align 3
.L153:
	movl	$11, %ecx
	jmp	.L60
.L145:
	movzbl	(%rdx,%rcx), %ecx
	leal	-1(%rcx), %r10d
	cmpb	$-3, %r10b
	jbe	.L54
.L56:
	leal	1(%rax), %ecx
	cmpl	$119, %eax
	jne	.L55
.L57:
	leal	-9(%rax), %ecx
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	je	.L379
	movzbl	(%rdx,%rcx), %r10d
	leal	-11(%rax), %ecx
	leal	-1(%r10), %esi
	cmpb	$-3, %sil
	ja	.L65
	movl	%r10d, %esi
	andl	$7, %esi
	cmpb	$6, %sil
	jne	.L65
.L137:
	movl	%r10d, %ecx
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L70
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L378:
	leal	1(%rax), %ecx
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	jne	.L141
	jmp	.L337
.L358:
	movl	780(%r8), %r8d
	jmp	.L4
.L72:
	cmpb	$-1, %al
	je	.L71
	movl	%eax, %ecx
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L71
	andl	$7, %eax
	subl	$4, %eax
	cmpb	$1, %al
	ja	.L71
	movl	%r14d, %r8d
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L75:
	cmpb	$-1, %dil
	je	.L74
	movl	%edi, %ecx
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L74
	andl	$7, %edi
	leal	-3(%rdi), %ecx
	andl	$253, %ecx
	jne	.L74
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L28:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$2, %r10b
	jne	.L35
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L35
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L362:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$2, %r10b
	jne	.L24
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L24
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L363:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$2, %r10b
	jne	.L19
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L19
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L364:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$2, %r10b
	jne	.L34
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L34
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L366:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$2, %r10b
	jne	.L40
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L40
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L365:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$2, %r10b
	jne	.L38
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%bl, %cl
	jne	.L38
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L367:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$2, %r10b
	jne	.L46
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L46
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L49:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$2, %r10b
	jne	.L48
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L48
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L379:
	leal	-11(%rax), %ecx
	jmp	.L65
.L371:
	leal	9(%rax), %ecx
	cmpl	$119, %ecx
	jg	.L57
	movslq	%ecx, %rcx
	cmpb	$0, 240(%r9,%rcx)
	jne	.L66
	leal	-9(%rax), %ecx
	jmp	.L67
	.p2align 4,,10
	.p2align 3
.L54:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$6, %r10b
	jne	.L56
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L56
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L373:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$6, %r10b
	jne	.L69
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L69
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L369:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$6, %r10b
	jne	.L59
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L59
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L372:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$6, %r10b
	jne	.L63
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L63
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L368:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$6, %r10b
	jne	.L52
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L52
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L370:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$6, %r10b
	jne	.L61
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L61
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L375:
	movl	%ecx, %r10d
	andl	$7, %r10d
	cmpb	$6, %r10b
	jne	.L68
	shrb	$3, %cl
	andl	$1, %ecx
	cmpb	%cl, %bl
	jne	.L68
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L374:
	movl	%r10d, %ecx
	andl	$7, %ecx
	cmpb	$6, %cl
	jne	.L70
	jmp	.L137
.L376:
	movq	136(%rsp), %rdx
.L79:
	movl	136(%rdx,%r11,4), %r8d
	testl	%r8d, %r8d
	js	.L81
	leal	10(%r8), %ebx
	cmpl	%ebx, %eax
	je	.L1
	leal	-10(%r8), %ebx
	cmpl	%ebx, %eax
	je	.L1
	leal	1(%r8), %ebx
	cmpl	%ebx, %eax
	je	.L1
	leal	-1(%r8), %ebx
	cmpl	%ebx, %eax
	je	.L1
	leal	11(%r8), %ebx
	cmpl	%ebx, %eax
	je	.L1
	leal	9(%r8), %ebx
	cmpl	%ebx, %eax
	je	.L1
	leal	-9(%r8), %ebx
	cmpl	%ebx, %eax
	je	.L1
	subl	$11, %r8d
	cmpl	%r8d, %eax
	je	.L1
.L81:
	imulq	$28, %r11, %r8
	movslq	792(%rdx,%r8), %r8
	testl	%r8d, %r8d
	jle	.L82
	imulq	$70, %r11, %rsi
	movzbl	(%r9,%r10), %edi
	movq	%r10, %rbp
	movl	%ecx, %r15d
	imulq	$280, %r11, %rbx
	movzbl	120(%r9,%r10), %r14d
	movq	%rdx, %rcx
	movq	%r11, %r13
	addq	%rsi, %r8
	addq	%rdx, %rbx
	leaq	(%rdx,%r8,4), %r10
	movl	%eax, %edx
	movl	%edi, %eax
	.p2align 4
	.p2align 3
.L97:
	movl	376(%rbx), %r8d
	cmpl	$119, %r8d
	ja	.L380
	movslq	%r8d, %rsi
	movzbl	(%r9,%rsi), %r11d
	movzbl	120(%r9,%rsi), %esi
	cmpb	%al, %r11b
	je	.L172
	cmpb	%r14b, %sil
	jne	.L84
.L172:
	cmpb	%al, %r11b
	je	.L381
	jnb	.L87
	addl	$1, %r8d
	cmpl	%r8d, %edx
	je	.L332
	movslq	%r8d, %r8
.L95:
	cmpl	$119, %r8d
	ja	.L84
	leaq	240+FILE_RANK_LOOKUPS(%rip), %rsi
	movzbl	(%rsi,%r8), %esi
	testb	%sil, %sil
	je	.L84
	cmpb	$0, (%rcx,%r8)
	jne	.L84
	addq	$1, %r8
	cmpl	%r8d, %edx
	jne	.L95
	movl	%esi, %ecx
	jmp	.L1
	.p2align 4,,10
	.p2align 3
.L84:
	addq	$4, %rbx
	cmpq	%rbx, %r10
	jne	.L97
	movl	%edx, %eax
	movq	%rbp, %r10
	movq	%rcx, %rdx
	movq	%r13, %r11
	movl	%r15d, %ecx
.L82:
	imulq	$28, %r11, %r8
	movl	788(%rdx,%r8), %ebx
	movl	%ebx, %r12d
	testl	%ebx, %ebx
	jle	.L98
	movzbl	(%r9,%r10), %ebx
	leal	-255(%rbx), %r8d
	movl	%ebx, %r13d
	movl	%r8d, %ebx
	negl	%ebx
	cmovns	%ebx, %r8d
	movzbl	120(%r9,%r10), %ebx
	movl	%r8d, 12(%rsp)
	imulq	$280, %r11, %r8
	movl	%ebx, 4(%rsp)
	leaq	(%r8,%rdx), %rsi
	leal	-255(%rbx), %r8d
	movl	%r8d, %ebx
	negl	%ebx
	cmovns	%ebx, %r8d
	movl	%r8d, 8(%rsp)
	movl	336(%rsi), %r8d
	cmpl	$119, %r8d
	ja	.L382
.L99:
	movslq	%r8d, %rbx
	movzbl	(%r9,%rbx), %edi
	movzbl	120(%r9,%rbx), %ebx
	movl	%edi, %r14d
	movl	%r13d, %edi
	subl	%r14d, %edi
	movl	%ebx, %r14d
	movl	4(%rsp), %ebx
	movl	%edi, %ebp
	subl	%r14d, %ebx
	movl	%edi, %r14d
	negl	%r14d
	cmovns	%r14d, %edi
	movl	%ebx, %r14d
	negl	%r14d
	cmovs	%ebx, %r14d
	cmpl	%r14d, %edi
	jne	.L100
	testl	%ebp, %ebp
	setg	3(%rsp)
	testl	%ebx, %ebx
	setg	%r14b
	cmpb	$0, 3(%rsp)
	je	.L173
	movl	$11, %r15d
	testb	%r14b, %r14b
	je	.L173
.L101:
	addl	%r15d, %r8d
	cmpl	%r8d, %eax
	je	.L1
	movslq	%r15d, %rdi
	movslq	%r8d, %r8
.L107:
	cmpl	$119, %r8d
	ja	.L100
	leaq	240+FILE_RANK_LOOKUPS(%rip), %rbx
	movzbl	(%rbx,%r8), %ebx
	testb	%bl, %bl
	je	.L100
	cmpb	$0, (%rdx,%r8)
	jne	.L100
	addq	%rdi, %r8
	cmpl	%r8d, %eax
	jne	.L107
	movl	%ebx, %ecx
	jmp	.L1
.L382:
	movl	12(%rsp), %edi
	cmpl	%edi, 8(%rsp)
	je	.L159
.L100:
	imulq	$70, %r11, %r8
	movslq	%r12d, %rbx
	addq	%rbx, %r8
	leaq	(%rdx,%r8,4), %rbx
.L104:
	addq	$4, %rsi
	cmpq	%rsi, %rbx
	je	.L98
	movl	336(%rsi), %r8d
	cmpl	$119, %r8d
	jbe	.L99
	movl	12(%rsp), %r14d
	cmpl	%r14d, 8(%rsp)
	jne	.L104
.L159:
	movl	$-11, %r15d
	jmp	.L101
.L380:
	cmpb	$-1, %al
	je	.L86
	cmpb	$-1, %r14b
	jne	.L84
	cmpb	$-1, %al
	je	.L86
.L87:
	subl	$1, %r8d
	cmpl	%r8d, %edx
	je	.L332
	movl	$-1, %esi
.L92:
	movslq	%esi, %rdi
	movslq	%r8d, %r8
.L94:
	cmpl	$119, %r8d
	ja	.L84
	leaq	240+FILE_RANK_LOOKUPS(%rip), %rsi
	movzbl	(%rsi,%r8), %esi
	testb	%sil, %sil
	je	.L84
	cmpb	$0, (%rcx,%r8)
	jne	.L84
	addq	%rdi, %r8
	cmpl	%r8d, %edx
	jne	.L94
	movl	%esi, %ecx
	jmp	.L1
	.p2align 4,,10
	.p2align 3
.L381:
	cmpb	%r14b, %sil
	jnb	.L86
	addl	$10, %r8d
	cmpl	%r8d, %edx
	je	.L332
	movl	$10, %esi
	jmp	.L92
.L86:
	subl	$10, %r8d
	cmpl	%r8d, %edx
	je	.L332
	movl	$-10, %esi
	jmp	.L92
.L173:
	testl	%ebp, %ebp
	jns	.L174
	movl	$9, %r15d
	testb	%r14b, %r14b
	jne	.L101
.L174:
	testl	%ebx, %ebx
	jns	.L159
	cmpb	$0, 3(%rsp)
	je	.L159
	movl	$-9, %r15d
	jmp	.L101
.L98:
	imulq	$28, %r11, %r8
	movslq	796(%rdx,%r8), %rbx
	testl	%ebx, %ebx
	jle	.L161
	imulq	$280, %r11, %r8
	movzbl	120(%r9,%r10), %r12d
	movzbl	(%r9,%r10), %r13d
	movl	%ecx, %r15d
	movq	%rdx, %rcx
	movl	%r12d, %edi
	movb	%r12b, 3(%rsp)
	movl	%r13d, %r14d
	leaq	(%r8,%rdx), %rsi
	imulq	$70, %r11, %r8
	addq	%rbx, %r8
	cmpb	$-1, %r13b
	setne	%r10b
	addb	$1, %dil
	leaq	(%rdx,%r8,4), %rbp
	setne	%r8b
	andl	%r8d, %r10d
	movb	%r10b, 4(%rsp)
.L133:
	movl	416(%rsi), %r8d
	cmpl	$119, %r8d
	ja	.L383
	movslq	%r8d, %r10
	movzbl	(%r9,%r10), %r11d
	movzbl	120(%r9,%r10), %ebx
	cmpb	%r14b, %r11b
	je	.L175
	cmpb	3(%rsp), %bl
	jne	.L134
.L175:
	cmpb	%r14b, %r11b
	je	.L384
	jnb	.L135
	leal	1(%r8), %r10d
	cmpl	%r10d, %eax
	je	.L332
	movslq	%r10d, %r10
.L123:
	cmpl	$119, %r10d
	ja	.L134
	leaq	240+FILE_RANK_LOOKUPS(%rip), %rdx
	movzbl	(%rdx,%r10), %edx
	testb	%dl, %dl
	je	.L134
	cmpb	$0, (%rcx,%r10)
	jne	.L134
	addq	$1, %r10
	cmpl	%r10d, %eax
	jne	.L123
	movl	%edx, %ecx
	jmp	.L1
	.p2align 4,,10
	.p2align 3
.L134:
	movl	%r13d, %r10d
	subl	%r11d, %r10d
.L126:
	movzbl	%bl, %edx
	movl	%r12d, %ebx
	movl	%r10d, %r11d
	subl	%edx, %ebx
	negl	%r11d
	movl	%ebx, %edx
	cmovs	%r10d, %r11d
	negl	%edx
	cmovs	%ebx, %edx
	cmpl	%edx, %r11d
	jne	.L127
	testl	%r10d, %r10d
	setg	%r11b
	testl	%ebx, %ebx
	setg	%dil
	testb	%r11b, %r11b
	je	.L176
	movl	$11, %edx
	testb	%dil, %dil
	je	.L176
.L128:
	addl	%edx, %r8d
	cmpl	%r8d, %eax
	je	.L332
	movslq	%edx, %r10
	movslq	%r8d, %r8
.L131:
	cmpl	$119, %r8d
	ja	.L127
	leaq	240+FILE_RANK_LOOKUPS(%rip), %rbx
	movzbl	(%rbx,%r8), %ebx
	testb	%bl, %bl
	je	.L127
	cmpb	$0, (%rcx,%r8)
	jne	.L127
	addq	%r10, %r8
	cmpl	%r8d, %eax
	jne	.L131
	movl	%ebx, %ecx
	jmp	.L1
	.p2align 4,,10
	.p2align 3
.L127:
	addq	$4, %rsi
	cmpq	%rsi, %rbp
	jne	.L133
.L161:
	xorl	%ecx, %ecx
	jmp	.L1
.L176:
	testl	%r10d, %r10d
	jns	.L177
	movl	$9, %edx
	testb	%dil, %dil
	jne	.L128
.L177:
	testl	%ebx, %ebx
	jns	.L167
	testb	%r11b, %r11b
	je	.L167
	movl	$-9, %edx
	jmp	.L128
.L135:
	leal	-1(%r8), %r10d
	cmpl	%r10d, %eax
	je	.L332
	movl	$-1, %edx
.L119:
	movslq	%edx, %rdi
	movslq	%r10d, %r10
.L121:
	cmpl	$119, %r10d
	ja	.L124
	leaq	240+FILE_RANK_LOOKUPS(%rip), %rdx
	movzbl	(%rdx,%r10), %edx
	testb	%dl, %dl
	je	.L124
	cmpb	$0, (%rcx,%r10)
	jne	.L124
	addq	%rdi, %r10
	cmpl	%r10d, %eax
	jne	.L121
	movl	%edx, %ecx
	jmp	.L1
	.p2align 4,,10
	.p2align 3
.L383:
	cmpb	$0, 4(%rsp)
	jne	.L385
	movl	$-1, %ebx
	movl	$255, %r11d
	cmpb	$-1, %r14b
	jne	.L135
.L347:
	leal	-10(%r8), %r10d
	cmpl	%r10d, %eax
	je	.L332
	movl	$-10, %edx
	jmp	.L119
.L384:
	cmpb	3(%rsp), %bl
	jnb	.L347
	leal	10(%r8), %r10d
	cmpl	%r10d, %eax
	je	.L332
	movl	$10, %edx
	jmp	.L119
.L124:
	movl	%r13d, %r10d
	cmpl	$119, %r8d
	jbe	.L134
.L112:
	subl	%r11d, %r10d
	jmp	.L126
.L332:
	movl	%r15d, %ecx
	jmp	.L1
.L167:
	movl	$-11, %edx
	jmp	.L128
.L385:
	movzbl	%r14b, %r10d
	movl	$-1, %ebx
	movl	$255, %r11d
	jmp	.L112
	.seh_endproc
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
.LC0:
	.long	10
	.long	-10
	.long	1
	.long	-1
	.align 16
.LC1:
	.long	11
	.long	9
	.long	-9
	.long	-11
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.1.0"
