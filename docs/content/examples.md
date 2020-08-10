@parent = page.html
@title = Examples

# Example Programs

Download the compiled programs and try them out in the emulator!

### Colors

Draws every supported color on the screen.
<a href="colors.dat" download>Download</a>

```
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
```

### Disco

Epilepsy warning: lots of flashing colors. Due to how much faster this emulator is
than the one this program was written for, it's more of just flashing colors than
what it originally looked like.
<a href="disco.dat" download>Download</a>

```
; Taken from 6502asm.com

start:
	inx
	txa
	sta $200, y
	sta $300, y
	sta $400, y
	sta $500, y
	iny
	tya
	cmp 16
	bne do
	iny
	jmp start
do:
	iny
	iny
	iny
	iny
	jmp start
```
