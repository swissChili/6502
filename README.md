# [6502 Toolchain](https://6502.swisschili.sh)

[![Screenshot](screenshot.png)](https://6502.swisschili.sh)

This project aims to create a portable toolchain for developing,
testing and debugging programs for the 6502 processor. An assembler
may be implemented in the future but no work has been done on that
yet.

The `instructions.h` header is generated from `6502.csv` and contains
definitions of every 6502 opcode, its mnemonic and addressing mode.
It is built automatically by cmake. 

## Dependencies

- POSIX Threads
- POSIX Messsage Queues
- SDL2
- OpenGL 3
- GLEW
- GNU Readline
- Nuklear (included)

## License

```
Copyright 2020 swissChili

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and 
the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
and the following disclaimer in the documentation and/or other materials provided with the 
distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse 
or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY 
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

---

```
 ____________________________________
/ On the subject of C program        \
| indentation: "In My Egotistical    |
| Opinion, most people's C programs  |
| should be indented                 |
| six feet downward and covered with |
| dirt."                             |
|                                    |
\ -- Blair P. Houghton               /
 ------------------------------------
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||
```
