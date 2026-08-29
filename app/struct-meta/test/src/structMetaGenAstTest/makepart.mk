TEST_SRCS := \
	$(MYAPP_DIR)/prod/src/cmd/struct-meta-gen/struct_meta_gen_ast.c

INCDIR += \
	$(MYAPP_DIR)/prod/src/cmd/struct-meta-gen

LIBS += mock_libc
