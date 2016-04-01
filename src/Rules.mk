TARGET  := lib/libtmio.a
SOURCES := tmio.c

INCDIRS := ${TARGET_DIR}/include
LDFLAGS := -L${TARGET_DIR}/lib
LDLIBS  := -lbufio
PREREQS := lib/libbufio.a

define MOVE_HEADER
	@mkdir -p $(TARGET_DIR)/include
	@cp -a $(DIR)/tmio.h $(TARGET_DIR)/include
endef

TGT_POSTMAKE := ${MOVE_HEADER}
