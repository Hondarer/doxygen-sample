TEST_SRCS := \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/parse/doc.c

ADD_SRCS := \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/parse/ast.c \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/parse/diagnostic.c

# doc.c はモジュール私有ヘッダー (doc.h) を持つため、元ディレクトリを探索対象へ加える。
# see: app/general/docs/coding-guideline.md の「モジュール私有ヘッダー」
INCDIR += \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/parse

LIBS += cplat mock_libc
