Orange System
========

This repository tracks all the documentation and code for my attempt at creating a custom CPU out of logic chips,
and writing the software tool-chain to use it.  The repository is structured in several modules, documented below.
It has 16 bit addresses and 16 bit data, with more memory being accessible through the memory management unit (MMU).

The final hardware design, an assembly language and a higher level language are still to-do, as well as any code that will
run on the system.

The documentation(not much) is currently at this repository's [wiki](https://github.com/ScratchOs/starfish/wiki), however the plan
is to move it to the `./docs/` folder, so it can be hosted in gh-pages.

# Modules
## Block
Block diagrams explaining the system.

## MicroASM
Current Version: v0.0.2 alpha
A program written in c to generate a virtual machine and assembler from an instruction set architecture description language, in src.
Current features: generate simple virtual machine from the description language, assembler not written yet.
Possible eventual goals include a programming language that can target the described ISA, Some ISAs for already existing processors
and defining the virtual machine in the description, rather than in MicroASM's source.


Building: `CMake`, if on windows, use MinGW GCC. MSVC does not work, unsure if clang would work, but hopefully would.
Running: `microasm --help`, also see example code in `src/emulator/*.uasm`