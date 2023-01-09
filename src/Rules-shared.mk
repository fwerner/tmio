TARGET      := lib/libtmio.so
SOURCES     := tmio.c
SRC_INCDIRS := ../externals/bufio/src
TGT_LDFLAGS := -shared -Llib
TGT_LDLIBS  := -lbufio
TGT_PREREQS := lib/libtmio.a