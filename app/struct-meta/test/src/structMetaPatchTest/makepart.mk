# テスト対象のソース ファイル
TEST_SRCS := \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/patch/patch.c

ADD_SRCS += \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/access/access.c \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/access/path.c \
	$(MYAPP_DIR)/prod/libsrc/struct_meta/meta/validate.c

LIBS += mock_com_util mock_libc
