;;; Shows some nice colors on the screen

	LDY #$0
loop:
	TYA
	STA $200, Y
	STA $300, Y
	STA $400, Y
	STA $500, Y
	INY
	CMP #$ff
	BCC loop
	BRK
