# ワークスペース作業ガイド

## 対象

この文書は、c-modernization-kit の統合ワークスペースに固有の作業手順を示します。  
個別 framework や app の詳細は、それぞれの AGENTS.md と README.md を参照してください。

## Git の扱い

ユーザーから個別に指示されるまで、ステージング、コミット、アンステージ、スタッシュを行いません。  
サブモジュールを変更するときは、サブモジュール内の差分と、親リポジトリから見たサブモジュール状態を分けて確認します。

## 主要コマンド

```bash
make
make test
make doxy
make docs
make skills
make sync-app-env
```

app を追加または削除した場合は、`.vscode`、GitHub Actions、Jenkins の環境変数を手編集せず、`make sync-app-env` を実行します。  
詳細は [VS Code 環境変数](../../general/docs/vscode-variables.md) を参照してください。

## ビルドとテスト

変更した app が一つなら、最初にその app 直下で `make test` を実行します。  
依存ファイルの内容変更だけで `make clean` を実行する必要はありません。

複数 app、framework、または共有ライブラリ層を変更した場合は、全体テストの所要時間と必要性をユーザーへ説明してから実行します。

ビルド後は、対象範囲で内容がある `.warn` ファイルを確認します。

```bash
find . -type f -name '*.warn' -size +0 -print
find . -type f -name '*.warn' -size +0 -print0 | xargs -0 -r sed -n '1,200p'
```

`.warn` ファイルは結果であり、直接編集しません。  
警告原因を修正し、同じビルドを再実行します。

## 文書の変更

Markdown を変更した場合は、対象ごとに次の順で確認します。

```bash
python framework/docsfw/bin/text_style_jp.py <対象ファイル> --dry-run
python framework/docsfw/bin/text_style_jp.py <対象ファイル> --in-place
```

全 Markdown を対象にする場合は、[Markdown 一括スタイル確認](markdown-style-bulk-check.md) に従います。

## ソースの変更

新規ソースは `clang-format`、既存ソースの変更は `git clang-format` で確認します。  
整形後は、Doxygen コメントの字下げも確認します。

C/C++ の規範は [コーディング規範](../../general/docs/coding-guideline.md)、テストの規範は [テスト方法](../../../framework/testfw/docs/how-to-test.md) を参照してください。
