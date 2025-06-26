#!/bin/bash

echo "Doxygenドキュメントを生成中..."

# 古いドキュメントを削除
rm -rf docs/html docs/latex

# 新しいドキュメントを生成
doxygen

# 生成完了メッセージ
if [ $? -eq 0 ]; then
    echo "ドキュメント生成が完了しました。"
    echo "docs/html/index.htmlを開いてドキュメントを確認してください。"
else
    echo "ドキュメント生成でエラーが発生しました。"
fi
