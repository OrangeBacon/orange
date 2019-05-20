HWVM (StarfishVM)
========

A virtual machine/emulator for the system, written in C#, that tries to be as accurate to what the real hardware will be as possible.
This makes it not that fast, although useful for testing.  This module is also known as "Starfish VM" in places.

There are two main components, the virtual machine/emulator library, `./VMLib/` and a WPF GUI to drive it at `./Interface/`.  There
is also a console-based GUI at `./ConsoleUI/`, for testing.

When .net Core 3 comes out, this should be migrated to it, or by that time it might have been re-written in c
(as command-line only) for better performance.  Ideally this would be cross-platform, but WPF isn't.
