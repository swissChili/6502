; Taken from 6502asm.com


start:
	inx							; 600
	txa							; 601
	sta $200, y					; 602
	sta $300, y					; 605
	sta $400, y					; 608
	sta $500, y					; 60b
	iny							; 60e
	tya							; 60f
	cmp 16						; 610
	bne do						; 612
	iny							; 614
	jmp start					; 615
do:
	iny							; 618
	iny							; 619
	iny							; 61a
	iny							; 61b
	jmp start					; 61c
