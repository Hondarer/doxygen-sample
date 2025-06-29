#!/bin/bash

# postprocess.sh - Markdownファイル内の !include ディレクティブを処理するスクリプト
# 使用方法: ./postprocess.sh <markdown_directory>
# 例: ./postprocess.sh docs-src/doxybook2

# set -x  # デバッグ時のみ有効にする

# 引数チェック
if [ $# -ne 1 ]; then
    echo "使用方法: $0 <markdown_directory>"
    echo "例: $0 docs-src/doxybook2"
    exit 1
fi

MARKDOWN_DIR="$1"

# ディレクトリの存在チェック
if [ ! -d "$MARKDOWN_DIR" ]; then
    echo "エラー: ディレクトリが存在しません: $MARKDOWN_DIR"
    exit 1
fi

#echo "Markdownファイルのポストプロセッシング: $MARKDOWN_DIR"

# 一時ディレクトリを作成
TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

# インクルードファイルを処理する関数
process_includes() {
    local file="$1"
    local temp_file
    temp_file=$(mktemp "$TEMP_DIR/$(basename "$file").XXXXXX") || {
        echo "エラー: 一時ファイルの作成に失敗しました: $file"
        return 1
    }
    local processed=false
    
    # ファイルを行ごと処理
    while IFS= read -r line; do
        # !include {filename} パターンをチェック
        if [[ "$line" =~ ^[[:space:]]*\!include[[:space:]]+([^[:space:]]+) ]]; then
            local include_file="${BASH_REMATCH[1]}"
            local include_path
            
            # 相対パスとして解決 (markdownディレクトリを基準)
            if [[ "$include_file" == /* ]]; then
                # 絶対パス
                include_path="$include_file"
            else
                # 相対パス
                include_path="$MARKDOWN_DIR/$include_file"
            fi
            
            # インクルードファイルの存在チェック
            if [ -f "$include_path" ]; then
                #echo "  -> インクルード: $include_file"
                # ファイルの内容を出力
                cat "$include_path"
                processed=true
            else
                echo "  -> 警告: インクルードファイルが見つかりません: $include_file"
                # 元の行をそのまま出力
                echo "$line"
            fi
        else
            # 通常の行をそのまま出力
            echo "$line"
        fi
    done < "$file" > "$temp_file"
    
    # 変更があった場合のみファイルを更新
    if [ "$processed" = true ]; then
        if mv "$temp_file" "$file" 2>/dev/null; then
            : #echo "  -> 更新完了: $(basename "$file")"
        else
            #echo "  -> エラー: ファイル更新に失敗: $(basename "$file")"
            rm -f "$temp_file"
            return 1
        fi
    else
        rm -f "$temp_file"
    fi
}

# YAMLフロントマター内の空行をすべて取り除く関数
remove_frontmatter_empty_lines() {
    local file="$1"
    local temp_file
    temp_file=$(mktemp "$TEMP_DIR/$(basename "$file").frontmatter.XXXXXX") || {
        echo "エラー: 一時ファイルの作成に失敗しました: $file"
        return 1
    }
    
    # awkを使用してYAMLフロントマター内の空行を除去
    awk '
    BEGIN { 
        in_frontmatter = 0
        frontmatter_started = 0
        line_count = 0
    }
    
    # 最初の行が --- で始まる場合、フロントマターの開始
    line_count == 0 && /^---[[:space:]]*$/ {
        frontmatter_started = 1
        in_frontmatter = 1
        print $0
        line_count++
        next
    }
    
    # フロントマター内で --- が見つかった場合、フロントマターの終了
    in_frontmatter && /^---[[:space:]]*$/ {
        in_frontmatter = 0
        print $0
        line_count++
        next
    }
    
    # フロントマター内の処理
    in_frontmatter {
        # 空行または空白のみの行をスキップ
        if (/^[[:space:]]*$/) {
            line_count++
            next
        }
        # 非空行はそのまま出力
        print $0
        line_count++
        next
    }
    
    # フロントマター外の処理 (通常の行)
    {
        print $0
        line_count++
    }
    ' "$file" > "$temp_file"
    
    # ファイルを更新
    if mv "$temp_file" "$file" 2>/dev/null; then
        return 0
    else
        rm -f "$temp_file"
        return 1
    fi
}

# 連続した空行を単一の空行に置換する関数
remove_consecutive_empty_lines() {
    local file="$1"
    local temp_file
    temp_file=$(mktemp "$TEMP_DIR/$(basename "$file").empty_lines.XXXXXX") || {
        echo "エラー: 一時ファイルの作成に失敗しました: $file"
        return 1
    }
    
    # awkを使用して処理
    # - 空白文字のみの行 (空行含む) を空行として扱う
    # - 連続する空行を1つの空行に置換
    # - 文末の複数の空行も1つの空行に置換
    awk '
    BEGIN { blank_count = 0 }
    
    # 空白文字のみの行 (空行含む) をチェック
    /^[[:space:]]*$/ {
        blank_count++
        # 最初の空行のみ保持
        if (blank_count == 1) {
            blank_line = ""  # 完全な空行として保存
        }
        next
    }
    
    # 非空行の場合
    {
        # 前に空行があった場合、1つだけ出力
        if (blank_count > 0) {
            print blank_line
            blank_count = 0
        }
        # 現在の行を出力
        print $0
    }
    ' "$file" > "$temp_file"
    
    # ファイルを更新
    if mv "$temp_file" "$file" 2>/dev/null; then
        return 0
    else
        rm -f "$temp_file"
        return 1
    fi
}

# 行末の連続した空白を取り除く関数
remove_trailing_spaces() {
    local file="$1"
    local temp_file
    temp_file=$(mktemp "$TEMP_DIR/$(basename "$file").trailing_spaces.XXXXXX") || {
        echo "エラー: 一時ファイルの作成に失敗しました: $file"
        return 1
    }
    
    # sedを使用して行末の空白文字を削除
    sed 's/[[:space:]]*$//' "$file" > "$temp_file"
    
    # ファイルを更新
    if mv "$temp_file" "$file" 2>/dev/null; then
        return 0
    else
        rm -f "$temp_file"
        return 1
    fi
}

# .mdファイルを配列に収集
mapfile -t md_files < <(find "$MARKDOWN_DIR" -name "*.md" -type f)

# 処理対象ファイル数をカウント
total_files=${#md_files[@]}
processed_files=0

#echo "見つかったMarkdownファイル: $total_files 個"

# 各ファイルを処理
for file in "${md_files[@]}"; do
    #echo "処理中: $(basename "$file")"
    
    # インクルードディレクティブが含まれているかチェック
    if grep -q "^[[:space:]]*!include[[:space:]]" "$file"; then
        if process_includes "$file"; then
            ((processed_files++))
        else
            : #echo "  -> 処理に失敗: $(basename "$file")"
        fi
    else
        : #echo "  -> インクルードディレクティブなし"
    fi

    # YAMLフロントマター内の空行を除去
    remove_frontmatter_empty_lines "$file"

    # 行末の空白を除去
    remove_trailing_spaces "$file"

    # 連続した空行を単一の空行に置換
    remove_consecutive_empty_lines "$file"
done

#echo ""
#echo "ポストプロセッシング完了"
#echo "  - 対象ファイル数: $total_files"
#echo "  - 処理済みファイル数: $processed_files"

# 不要ファイルの削除
# 構造体ファイルは、それぞれに include するので処理後は不要
rm -f $MARKDOWN_DIR/struct*.md
# クラスインデックスは不要
rm -f $MARKDOWN_DIR/index_classes.md
# 名前空間インデックスは不要
rm -f $MARKDOWN_DIR/index_namespaces.md

# 以下、現段階で使用していない
# Pages インデックスは不要
rm -f $MARKDOWN_DIR/index_pages.md
# Exanples インデックスは不要
rm -f $MARKDOWN_DIR/index_examples.md
# ディレクトリページは不要
rm -f $MARKDOWN_DIR/dir_*.md

# ファイルインデックスの編集
# テンプレートでは正しく置換できなかったため、シェルで加工する
#
# オリジナル
# * **dir [src]**
#     * **file [src/calculator.c](calculator_8c.md#file-calculator.c)** <br/>計算機の実装ファイル
#     * **file [src/calculator.h](calculator_8h.md#file-calculator.h)** <br/>簡単な計算機のヘッダーファイル
#
# 編集後
# * **dir [src]**
#     * **file [calculator.c](calculator_8c.md)** <br/>計算機の実装ファイル
#     * **file [calculator.h](calculator_8h.md)** <br/>簡単な計算機のヘッダーファイル
#
cat $MARKDOWN_DIR/index_files.md | \
sed 's/\(\*\* *file \[\)[^/]*\/\([^]]*\]\)/\1\2/g' | \
sed 's/\(\.md\)#[^)]*/\1/g' > $MARKDOWN_DIR/index_files_new.md
mv $MARKDOWN_DIR/index_files_new.md $MARKDOWN_DIR/index_files.md

# Todo リストの編集
# テンプレートでは正しく置換できなかったため、シェルで加工する
#
# オリジナル
# Global [add](group__public__api.md#function-add)  (int a, int b)
#
# 編集後
# Global [add](group__public__api.md)  (int a, int b)
#
#cat $MARKDOWN_DIR/todo.md | \
#sed 's/\(\.md\)#[^)]*/\1/g' > $MARKDOWN_DIR/todo_new.md
#mv $MARKDOWN_DIR/todo_new.md $MARKDOWN_DIR/todo.md
# 不要なので削除する
rm -f $MARKDOWN_DIR/todo.md

# Deprecated リストの編集
# テンプレートでは正しく置換できなかったため、シェルで加工する
#
# オリジナル
# Global [add](group__public__api.md#function-add)  (int a, int b)
#
# 編集後
# Global [add](group__public__api.md)  (int a, int b)
#
#cat $MARKDOWN_DIR/deprecated.md | \
#sed 's/\(\.md\)#[^)]*/\1/g' > $MARKDOWN_DIR/deprecated_new.md
#mv $MARKDOWN_DIR/deprecated_new.md $MARKDOWN_DIR/deprecated.md
# 不要なので削除する
rm -f $MARKDOWN_DIR/deprecated.md

# 処理終了
exit 0
