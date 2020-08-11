@parent = page.html
@title = Debugger

# Debugger commands

These are the commands that the debugger CLI supports. Due to the asynchronous nature
of the debugger and emulator, commands can be run even while the emulator is actively
executing the program. For instance, try running the `disco.asm` <a href="/examples.html">
example</a> and typing `set A #$0` into the debugger while it's running with a screen
attached, you will be able to modify the program in real time!

<br>
Or, run the emulator on an empty file, and run `set $200 #$FF` to write to the frame
buffer. You can even draw a picture by hand using just the debugger!

<br>
The GUI debugger actually just sends commands like these to the emulator using a
message queue, and you can do the same. Look in `/dev/mqueue` for the message queue
the emulator is using, and try sending a debug command as a null-terminated string
to the queue, you should be able to interact with the emulator from a different
process in real time.

#### `step`, `s`

Step through one CPU instruction

#### `show [$addr]`, `print [$addr]`

Show the value at `$addr` in memory, or, if no address given, show value of all registers.

#### `set $addr #$val`, `set reg #$val`

Set either `$addr` or register `reg` to `#$val`.

#### `run`

Run the emulator.

#### `pause`

Stop running the emulator.

#### `quit`, `exit`

Quit the debugger, emulator, and the screen (if running).
