	.file	"assembly_comparison_test.cpp"
	.text
	.p2align 4
	.globl	_Z28generate_all_moves_old_clearRK8PositionR10S_MOVELIST
	.def	_Z28generate_all_moves_old_clearRK8PositionR10S_MOVELIST;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z28generate_all_moves_old_clearRK8PositionR10S_MOVELIST
_Z28generate_all_moves_old_clearRK8PositionR10S_MOVELIST:
.LFB3074:
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$32, %rsp
	.seh_stackalloc	32
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
	call	_Z21generate_bishop_movesRK8PositionR10S_MOVELIST5Color
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	call	_Z19generate_rook_movesRK8PositionR10S_MOVELIST5Color
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	call	_Z20generate_queen_movesRK8PositionR10S_MOVELIST5Color
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	addq	$32, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	jmp	_Z19generate_king_movesRK8PositionR10S_MOVELIST5Color
	.seh_endproc
	.p2align 4
	.globl	_Z29generate_all_moves_new_directRK8PositionR10S_MOVELIST
	.def	_Z29generate_all_moves_new_directRK8PositionR10S_MOVELIST;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z29generate_all_moves_new_directRK8PositionR10S_MOVELIST
_Z29generate_all_moves_new_directRK8PositionR10S_MOVELIST:
.LFB3922:
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$32, %rsp
	.seh_stackalloc	32
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
	call	_Z21generate_bishop_movesRK8PositionR10S_MOVELIST5Color
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	call	_Z19generate_rook_movesRK8PositionR10S_MOVELIST5Color
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	call	_Z20generate_queen_movesRK8PositionR10S_MOVELIST5Color
	movl	%edi, %r8d
	movq	%rsi, %rdx
	movq	%rbx, %rcx
	addq	$32, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	jmp	_Z19generate_king_movesRK8PositionR10S_MOVELIST5Color
	.seh_endproc
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.1.0"
	.def	_Z19generate_pawn_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.def	_Z21generate_knight_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.def	_Z21generate_bishop_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.def	_Z19generate_rook_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.def	_Z20generate_queen_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
	.def	_Z19generate_king_movesRK8PositionR10S_MOVELIST5Color;	.scl	2;	.type	32;	.endef
