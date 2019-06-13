# MicroASM
A microcode assembler

## running
```
microasm <filename>
```

## testing
Using a debug build:
```
cd microasm
microasm test ./test
```

## building
```
cd microasm
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug # or "Release"
make
```