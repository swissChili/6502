start:
	lda #$32 					; Store $32 in a
	tax							; Transfer a to x
	stx $200					; Store x at $200
	jmp ($FFAA)					; Jump to the address at $FFAA
