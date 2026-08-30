TEST_SRCS := \
	$(MYAPP_DIR)/prod/src/cmd/struct-meta-gen/struct_meta_gen_emit_array.c

INCDIR += \
	$(MYAPP_DIR)/prod/src/cmd/struct-meta-gen

LIBS += cplat mock_libc
