	.file	"board120.cpp"
	.text
	.p2align 4
	.globl	_Z14from_algebraicSt17basic_string_viewIcSt11char_traitsIcEE
	.def	_Z14from_algebraicSt17basic_string_viewIcSt11char_traitsIcEE;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z14from_algebraicSt17basic_string_viewIcSt11char_traitsIcEE
_Z14from_algebraicSt17basic_string_viewIcSt11char_traitsIcEE:
.LFB823:
	.seh_endprologue
	movq	8(%rcx), %rdx
	cmpq	$2, (%rcx)
	jne	.L6
	movzbl	(%rdx), %ecx
	leal	-65(%rcx), %eax
	cmpb	$7, %al
	ja	.L8
.L3:
	movzbl	1(%rdx), %edx
	subl	$49, %edx
	cmpb	$7, %dl
	ja	.L6
	addl	$21, %eax
	movzbl	%dl, %edx
	leal	(%rdx,%rdx,4), %edx
	movzbl	%al, %eax
	leal	(%rax,%rdx,2), %eax
	ret
	.p2align 4,,10
	.p2align 3
.L8:
	leal	-97(%rcx), %eax
	cmpb	$7, %al
	jbe	.L3
.L6:
	movl	$-1, %eax
	ret
	.seh_endproc
	.p2align 4
	.globl	_Z12to_algebraiciPc
	.def	_Z12to_algebraiciPc;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z12to_algebraiciPc
_Z12to_algebraiciPc:
.LFB826:
	.seh_endprologue
	movq	%rdx, %rax
	cmpl	$119, %ecx
	ja	.L12
	movslq	%ecx, %rcx
	leaq	FILE_RANK_LOOKUPS(%rip), %rdx
	cmpb	$0, 240(%rdx,%rcx)
	je	.L12
	movzbl	(%rdx,%rcx), %r9d
	movzbl	120(%rdx,%rcx), %edx
	movb	$0, 2(%rax)
	leal	97(%r9), %r8d
	addl	$49, %edx
	movb	%r8b, (%rax)
	movb	%dl, 1(%rax)
	ret
	.p2align 4,,10
	.p2align 3
.L12:
	movl	$63, %r8d
	movl	$63, %edx
	movb	$0, 2(%rax)
	movb	%r8b, (%rax)
	movb	%dl, 1(%rax)
	ret
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
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.1.0"
