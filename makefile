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
	doxygen doxyconfig/Doxyfile
	mkdir -p docs-src/doxybook
# プリプロセッシング
	doxyconfig/templates/preprocess.sh xml
# xml -> md 変換
	# TODO: パスチェック (インストール済であれば、それを使う)
	bin/doxybook2/doxybook2 \
		-i xml/ \
		-o docs-src/doxybook/ \
		--config doxyconfig/doxybook-config.json \
		--templates doxyconfig/templates
# ポストプロセッシング
	doxyconfig/templates/postprocess.sh docs-src/doxybook

# クリーンアップ
clean:
	rm -rf $(BUILD_DIR) calculator docs/html docs-src/doxybook

.PHONY: all clean docs
