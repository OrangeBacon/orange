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

## HWVM  (WIP)
A virtual machine/emulator for the system, written in C#, that tries to be as accurate to what the real hardware will be as possible.
This makes it not that fast, although useful for testing.  This module is also known as "Starfish VM".