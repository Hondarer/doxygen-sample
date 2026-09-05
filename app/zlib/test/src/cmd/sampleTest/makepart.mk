TEST_SRCS := $(MYAPP_DIR)/prod/src/cmd/zlib_sample/zlib_sample.c
USE_WRAP_MAIN := 1
LIBS += mock_zlib mock_libc
