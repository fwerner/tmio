CFLAGS       := -std=c99 -O2 -Wall -Wextra -W -pedantic -march=native -fPIC
INCDIRS      := src
SUBMAKEFILES := src/Rules.mk src/Rules-shared.mk examples/Rules.mk
