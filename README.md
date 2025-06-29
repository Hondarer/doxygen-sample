# study-doxygen

from [https://claude.ai/public/artifacts/11b194e0-9d53-42e1-bb30-6a81f95bc5b0](https://claude.ai/public/artifacts/11b194e0-9d53-42e1-bb30-6a81f95bc5b0)

## 概要

Doxygen は C/C++、Java、Python 等のソースコードから自動的にドキュメントを生成するツールです。本ガイドでは、Linux で Doxygen を使用して C 言語のソースコードをドキュメント化する手順を詳しく説明します。

## 前提

+ `/usr/local/bin/plantuml.jar` が存在すること。
  上記でない場合は、`Doxyfile` の `PLANTUML_JAR_PATH` を適切に設定する。
  doxygen の仕様上、ファイル名は `plantuml.jar` 固定のため、注意。
+ `doxybook2` に PATH が通っていること。
  現段階では、プロジェクトフォルダの `bin/doxybook2/bin` に `doxybook2` が存在することを前提にしているが、最終的には PATH を検索する予定。

## プロジェクトディレクトリの準備

プロジェクトディレクトリを作成し、サンプルの C 言語ファイルを用意します。

```bash
mkdir src docs
```

## サンプル C 言語ソースファイルの作成

### src/calculator.h

```c
/**
 * @file calculator.h
 * @brief 簡単な計算機のヘッダーファイル
 * @author あなたの名前
 * @date 2025-06-27
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

/**
 * @brief 二つの整数を加算する関数
 * @param a 第一オペランド
 * @param b 第二オペランド
 * @return 加算結果
 */
int add(int a, int b);

/**
 * @brief 二つの整数を減算する関数
 * @param a 被減数
 * @param b 減数
 * @return 減算結果
 */
int subtract(int a, int b);

/**
 * @brief 二つの整数を乗算する関数
 * @param a 第一因数
 * @param b 第二因数
 * @return 乗算結果
 */
int multiply(int a, int b);

/**
 * @brief 二つの整数を除算する関数
 * @param a 被除数
 * @param b 除数
 * @return 除算結果
 * @warning b が 0 の場合、結果は未定義です
 */
double divide(int a, int b);

#endif // CALCULATOR_H
```

### src/calculator.c

```c
/**
 * @file calculator.c
 * @brief 計算機の実装ファイル
 */

#include "calculator.h"

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

double divide(int a, int b) {
    if (b == 0) {
        return 0.0; // エラー処理は簡略化
    }
    return (double)a / b;
}
```

## Doxyfile 設定ファイルの生成

プロジェクトのルートディレクトリで以下のコマンドを実行します。

```bash
doxygen -g
```

これにより `Doxyfile` という設定ファイルが生成されます。

## Doxyfile の編集

生成された Doxyfile を編集して、プロジェクトに適した設定を行います。

### 主要な設定項目

以下の項目を変更します：

```text
# プロジェクト名
PROJECT_NAME           = "My Calculator Project"

# プロジェクトのバージョン
PROJECT_NUMBER         = "1.0.0"

# 入力ディレクトリ
INPUT                  = src/

# ファイル拡張子
FILE_PATTERNS          = *.c *.h

# 再帰的にサブディレクトリを検索
RECURSIVE              = YES

# 出力ディレクトリ
OUTPUT_DIRECTORY       = docs/

# HTML ドキュメントを生成
GENERATE_HTML          = YES

# LaTeX ドキュメントを生成 (通常は NO)
GENERATE_LATEX         = NO

# XML ドキュメントを生成
GENERATE_XML           = YES

# ソースコードを含める
SOURCE_BROWSER         = YES

# 関数やクラスの呼び出し関係図を生成
CALL_GRAPH             = YES
CALLER_GRAPH           = YES

# UML スタイルの関係図を生成
UML_LOOK               = YES

# グラフ生成に dot を使用
HAVE_DOT               = YES

# すべての関数/変数を文書化
EXTRACT_ALL            = YES
```

## ドキュメントの生成

設定が完了したら、以下のコマンドでドキュメントを生成します。

```bash
doxygen
```

## 生成されたドキュメントの確認

ドキュメントが正常に生成されると、`docs/html/` ディレクトリに HTML ファイルが作成されます。

```bash
# ブラウザでドキュメントを開く
firefox docs/html/index.html
# または
google-chrome docs/html/index.html
# または
xdg-open docs/html/index.html
```

## より詳細なドキュメント化

### コメントの書き方

Doxygen では以下のスタイルでコメントを記述できます：

#### JavaDoc スタイル

```c
/**
 * @brief 関数の簡潔な説明
 * @param param_name パラメータの説明
 * @return 戻り値の説明
 * @see 関連する関数や型
 * @note 特記事項
 * @warning 警告事項
 */
```

#### Qt/KDE スタイル

```c
/*!
 * \brief 関数の簡潔な説明
 * \param param_name パラメータの説明
 * \return 戻り値の説明
 */
```

### 主要な Doxygen タグ

- `@brief` - 簡潔な説明
- `@param` - パラメータの説明
- `@return` - 戻り値の説明
- `@see` - 関連項目への参照
- `@note` - 注意事項
- `@warning` - 警告
- `@example` - 使用例
- `@code @endcode` - コード例の記述
- `@todo` - 未実装項目

## 自動化スクリプトの作成

ドキュメント生成を自動化するためのスクリプトを作成します。

### generate_docs.sh

```bash
#!/bin/bash

echo "Doxygen ドキュメントを生成中..."

# 古いドキュメントを削除
rm -rf docs/html docs/latex

# 新しいドキュメントを生成
doxygen

# 生成完了メッセージ
if [ $? -eq 0 ]; then
    echo "ドキュメント生成が完了しました。"
    echo "docs/html/index.html を開いてドキュメントを確認してください。"
else
    echo "ドキュメント生成でエラーが発生しました。"
fi
```

スクリプトを実行可能にします：

```bash
chmod +x generate_docs.sh
./generate_docs.sh
```

## Makefile との統合

Makefile にドキュメント生成のターゲットを追加できます。

### Makefile

```makefile
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
```

使用方法：

```bash
make docs
```

## トラブルシューティング

### よくある問題と解決方法

1. **日本語文字化け**

   Doxyfile でエンコーディングを設定：
   ```
   DOXYFILE_ENCODING      = UTF-8
   ```

2. **ファイルが見つからない**

   INPUT パスと FILE_PATTERNS の設定を確認

3. **グラフが生成されない**

   HAVE_DOT を YES に設定し、graphviz がインストールされていることを確認

## まとめ

このガイドに従うことで、Linux で Doxygen を使用して C 言語のソースコードから高品質なドキュメントを生成できます。継続的にソースコードにコメントを追加し、定期的にドキュメントを更新することで、プロジェクトの保守性と可読性を大幅に向上させることができます。
