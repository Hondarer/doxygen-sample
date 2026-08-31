# テスト対象のソース ファイル
TEST_SRCS := \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/catalog/build.c

ADD_SRCS += \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/catalog/arena.c \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/layout/layout.c \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/parse/ast.c \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/parse/diagnostic.c

LIBS += cplat mock_libc
