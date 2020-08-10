@parent = page.html
@title = Building

# Building from source

You need the following libraries installed to build the emulator:

`pthread` `rt` `m` `SDL2` `GL` `GLEW` `GLU` `readline`

The first three will be included on any POSIX compliant operating system (OS X, Linux, BSD, etc).
To install SDL2, visit its <a href="https://www.libsdl.org/">website</a> or use your distributions
package manager.

<br>
You also need `awk` installed if you want to modify the `6502.csv` file from which parts of the
emulator are generated. If you don't want to do this, run cmake with `-DGEN_INSTRUCTIONS_HEADER=OFF`

<br>
You may be able to build this on Windows using Cygwin or MinGW, but I haven't tested that.

<br>
Run the usual commands to build with cmake:
```
$ mkdir build
$ cd build
$ cmake ..
$ make -j
$ ./6502 # you built it, nice
```
