@parent = page.html
@title = Home

# 6502 Toolchain

<center>
	<video controls="true">
		<source src="demo.webm" type="video/webm">
	</video>
</center>

This project aims to create a fast, powerful, and easy to use toolchain for writing, debugging,
and running programs for the 6502 processor. It features a graphical and CLI debugger, disassembler,
and emulator.


## Features

### 8 bit color

That's 16 times more colors than <a href="http://6502asm.com/" target="_blank">the competition</a>!

### Graphical debugger

Easily step through, run, and debug your programs using a fast graphical debugger. A traditional
CLI debugger is also available.

### Fully multithreaded

The graphical debugger, cli debugger, and screen are all fully asynchronous. That means your
debugger stays fast, even when the emulator is running at full capacity.
It will soon be possible to even debug an already running instance of the emulator!

### *Fast*

This emulator is incredibly fast. So fast that programs written for other emulators don't work
properly half the time because of how fast it is! At some point I will add an option to slow
the emulator down so it is comparable to other emulators.

<br>

So what are you waiting for? Download the emulator now, or build it from source.