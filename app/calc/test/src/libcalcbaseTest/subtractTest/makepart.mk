# テスト対象のソース ファイル
TEST_SRCS := \
	$(MYAPP_DIR)/prod/libsrc/calcbase/calcbase_subtract.c

# ライブラリの指定
# subtractTest では、calcbase_add 関数のモックを使ってテストを行う
LIBS += mock_calcbase
