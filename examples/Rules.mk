TGT_PREREQS  := lib/libtmio.a
TGT_LDFLAGS  := -L${TARGET_DIR}/lib
TGT_LDLIBS   := -ltmio -lbufio -lm
SUBMAKEFILES += $(shell ls ${DIR}/tmio_*.mk | sed s,^${DIR}/,,g)
