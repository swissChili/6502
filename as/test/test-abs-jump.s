;;; This program should test for a bug involving endianness.

start:
	lda #$FF
	sta $200					; Put something on the screen, just to see
	jmp second-pixel
	brk

second-pixel:
	lda #$E0					; Red
	sta $201
	sta $220
	sta $221					; Draw a box around the white pixel

loop:							; Infinite loop
	jmp loop
