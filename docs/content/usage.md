@parent = page.html
@title = Usage

# Basic Usage

The `6502` command takes some arguments that control how it functions. Each flag is documented
here. Note that only UNIX style flags are supported, not GNU style. This uses the standard
`getopt()` function, so flags work the same as any UNIX command.

#### `-g`

Opens a GUI debugger window.

#### `-s`

Opens a window that shows the emulators screen. Cannot be used in conjunction with `-g`.

#### `-H`

Keep the emulator running after the CPU halts (after an interrupt is triggered). Useful
for debugging short programs. Does nothing when used with `-D`.

#### `-d`

Disassemble the input file, printing the whole disassembly to `stdout`. You probably want
to use this with `-n`

#### `-r`

Run the input file. Can be used in conjunction with `-s` to run and display the output.

#### `-D`

Open CLI debugger. Can be used with `-s` to view screen while debugging. 

#### `-i input`

Read `input` into the CPUs memory.

#### `-n number_of_instructions`

Disassemble only `number_of_instructions` instructions.

#### `-h, -?`

Print a help message.
