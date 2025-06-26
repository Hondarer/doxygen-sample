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
	doxygen

# クリーンアップ
clean:
	rm -rf $(BUILD_DIR) calculator docs/html docs/latex

.PHONY: all clean docs
