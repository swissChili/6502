;;; This file is meant to test the parsing capabilities of the assembler.
;;; It should contain every address mode to its parsing, as well as a
;;; range of instructions to test.

;;; When the assembler supports macros, those will be tested here as well.

start:
	lda #$32 					; Store $32 in a
	tax							; Transfer a to x
	stx $200					; Store x at $200
	jmp ($FFAA)					; Jump to the address at $FFAA
second_label:
	lda $30, X
	inc
	adc #$3
	bne start
	jsr another_subroutine
	tax
	hlt

another_subroutine:
	ret
