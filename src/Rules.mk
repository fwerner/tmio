TARGET      := lib/libtmio.a
SOURCES     := tmio.c
SRC_INCDIRS := ../externals/bufio/src

define BUILD_DEPENDENCIES

	@echo Building dependency bufio
	@cd $(DIR)/../externals/bufio && $(MAKE)
	@cp -a $(DIR)/../externals/bufio/lib/libbufio.a $(TARGET_DIR)/lib
endef

define MOVE_HEADER

	@mkdir -p $(TARGET_DIR)/include
	@cp -a $(DIR)/tmio.h $(TARGET_DIR)/include
endef

define REMOVE_HEADER

  @rm -f $(TARGET_DIR)/include/tmio.h
	@rm -f $(TARGET_DIR)/lib/libbufio.a
endef

TGT_POSTMAKE := ${MOVE_HEADER}
TGT_POSTMAKE += ${BUILD_DEPENDENCIES}

TGT_POSTCLEAN := ${REMOVE_HEADER}
