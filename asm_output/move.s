	.file	"move.cpp"
	.text
	.align 2
	.p2align 4
	.globl	_ZN6S_MOVE11decode_moveEiRiS0_R9PieceType
	.def	_ZN6S_MOVE11decode_moveEiRiS0_R9PieceType;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN6S_MOVE11decode_moveEiRiS0_R9PieceType
_ZN6S_MOVE11decode_moveEiRiS0_R9PieceType:
.LFB2319:
	.seh_endprologue
	movl	%ecx, %eax
	andl	$127, %eax
	movl	%eax, (%rdx)
	movl	%ecx, %eax
	sarl	$20, %ecx
	sarl	$7, %eax
	andl	$15, %ecx
	andl	$127, %eax
	movl	%eax, (%r8)
	movb	%cl, (%r9)
	ret
	.seh_endproc
	.align 2
	.p2align 4
	.globl	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
	.def	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b
_ZN6S_MOVE11encode_moveEii9PieceTypebbS0_b:
.LFB2320:
	.seh_endprologue
	movzbl	48(%rsp), %eax
	movzbl	40(%rsp), %r10d
	sall	$20, %eax
	sall	$19, %r10d
	andl	$15728640, %eax
	orl	%r10d, %eax
	movzbl	56(%rsp), %r10d
	movzbl	%r9b, %r9d
	andl	$127, %ecx
	sall	$18, %r9d
	sall	$7, %edx
	sall	$24, %r10d
	andl	$16256, %edx
	sall	$14, %r8d
	orl	%r10d, %eax
	andl	$245760, %r8d
	orl	%r9d, %eax
	orl	%ecx, %eax
	orl	%edx, %eax
	orl	%r8d, %eax
	ret
	.seh_endproc
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.1.0"
