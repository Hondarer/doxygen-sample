# 基本的な Makefile 例
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC_DIR = src
BUILD_DIR = build
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# メインターゲット
all: calculator

calculator: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ドキュメント生成
docs:
	mkdir -p docs/doxygen
	doxygen
	mkdir -p docs-src/doxybook2
# プリプロセッシング
	doxybook-templates/grouping_api_doc/preprocess.sh docs/doxygen/xml
# xml -> md 変換
	# TODO: パスチェック (インストール済であれば、それを使う)
	bin/doxybook2/bin/doxybook2 \
		-i docs/doxygen/xml/ \
		-o docs-src/doxybook2/ \
		--config doxybook-config.json \
		--templates doxybook-templates/grouping_api_doc
# ポストプロセッシング
	doxybook-templates/grouping_api_doc/postprocess.sh docs-src/doxybook2

# クリーンアップ
clean:
	rm -rf $(BUILD_DIR) calculator docs/html docs-src/doxybook2

.PHONY: all clean docs
