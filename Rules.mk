CFLAGS       := -std=c99 -O2 -Wall -Wextra -W -pedantic -march=native -fPIC
INCDIRS      := src
SUBMAKEFILES := src/Rules.mk examples/Rules.mk

.PHONY: submodules
submodules:
	git submodule update --init --recursive

all: submodules
