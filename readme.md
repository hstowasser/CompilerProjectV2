# The GeeCC compiler project. 
        ( As in "gee, I didn't expect that to work")

## Build
        Compiled using g++ v9.3.0 though any version should work.
        run the command `make` and the geecc executable should be generated.

## Usage
        The GeeCC compiler requires that you have a linux system and `llvm-as` installed.
        Example usage:
        `./geecc mycode.src`
        If successful this generates a mycode.src.ll file with llvm assembly
        and an executable with the name of your program in all caps.