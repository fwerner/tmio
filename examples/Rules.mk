TGT_PREREQS  := lib/libtmio.a
TGT_LDFLAGS  := -L${TARGET_DIR}/lib
TGT_LDLIBS   := -l:libtmio.a -l:libbufio.a -lm
SUBMAKEFILES += $(shell ls ${DIR}/tmio_*.mk | sed s,^${DIR}/,,g)
