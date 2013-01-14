
Thu 10 January 2013 14:21:04 (PST)

This directory is for development of the psas
cross compiler(s)

A configuration PSAS use of stm32f407 is provided in
the samples directory and is used by the makefile and
ct-ng tool.

Currently the crosstool-ng cross compiler build
tool is used. ("ct-ng") (see: crosstool-ng.org)

Thu 10 January 2013 14:24:58 (PST)

A makefile is provided. Once the crosstool-ng tools
have been installed, 'make all' should build the
cross compilers.

Change the NUMCPUS to something reasonable for your
system

