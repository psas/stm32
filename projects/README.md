The makefile here makes all projects, it's a good idea to run it once before
you push to verify nothing broke. make clean will clean all projects.

Projects intended for use in flight should have the prefix flight-
Line endings should be unix
Don't commit binaries. 
Keep Board files in common/boards, projects shouldn't have individual ones.


build for flight turns off all debug symbols, sets Ofast, all warnings are errors, disables usb, serial

Build for debug turns on all(?) debug symbols, sets Og, user specified warnings

TODO: Common hostside program/framework thingy
