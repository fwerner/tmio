LDFLAGS := -L${TARGET_DIR}/lib
LDLIBS  := -ltmio -lbufio
PREREQS := lib/libtmio.a lib/libbufio.a

SUBMAKEFILES := $(shell ls ${DIR}/tmio_*.mk | sed s,^${DIR}/,,g)
