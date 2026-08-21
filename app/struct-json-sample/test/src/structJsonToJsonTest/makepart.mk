# テスト対象のソース ファイル
TEST_SRCS := \
	$(MYAPP_DIR)/prod/libsrc/struct_json/struct_json_to_json.c

# cJSON API を直接使用するため実体をリンクする (モック不要)。
LIBS += cjson com_util
