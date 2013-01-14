
# Makefile for building the PSAS st32F407 x-compilers
# Depends on ct-ng (crosstool-ng.org)
#   see create-ctng.bash in this directory

unexport LD_LIBRARY_PATH

CWD            := $(sh pwd)
VERSION        := 1.17.0
CTNGDIR        := /opt/crosstool-ng
CTNGEXE        := ct-ng
NUMCPUS        := 9

%:
	export PATH="${PATH}:$(CTNGDIR)/bin"
	mkdir -p .build
	mkdir -p tarballs
	$(CTNGDIR)/bin/$(CTNGEXE) arm-psas-eabi
	$(CTNGDIR)/bin/$(CTNGEXE) build.$(NUMCPUS)


