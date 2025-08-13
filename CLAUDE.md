# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

このファイルは、このリポジトリでコードを扱う際のClaude Code (claude.ai/code) への指針を提供します。

## プロジェクト概要

これは、Cソースコードからドキュメントを生成する方法を実証するDoxygenドキュメント学習プロジェクトです。プロジェクトには以下が含まれます：

- C言語のシンプルな計算機関数
- ドキュメント生成用のDoxygen設定
- マークダウンドキュメント用のDoxybook2統合
- 日本語ドキュメント出力用のカスタムテンプレート

## 主要コマンド

### ドキュメント生成
```bash
cd doxyfw && make docs
```
このコマンドは以下の処理を順次実行します：
1. `../Doxyfile.part`が存在する場合、基本設定ファイルと結合して一時ファイルを作成し使用（設定のオーバーライド対応）
2. `../prod`ディレクトリからCソースファイルを解析し、`../xml/`にDoxygen XMLファイルと`../docs/doxygen/html/`にHTMLファイルを生成
3. `preprocess.sh`でXMLファイルを前処理（PlantUMLタグ、パラメータdirection属性、linebreakタグを変換）
4. Doxybook2でXMLを`../docs-src/doxybook/`のマークダウンに変換（カスタム日本語テンプレート使用）
5. `postprocess.sh`で`!include`ディレクティブを処理して関連コンテンツを統合

### クリーンアップ
```bash
cd doxyfw && make clean
```
生成されたドキュメント（`../docs/doxygen`、`../docs-src/doxybook`、`../xml`）を削除します。

## アーキテクチャ

### プロジェクト構造
```
study-doxygen/
├── doxyfw/                    # ドキュメント生成環境
│   ├── makefile              # ドキュメント生成用Makefile
│   └── config/               # 設定ファイル群
│       ├── Doxyfile          # Doxygen設定
│       ├── doxybook-config.json  # Doxybook2設定
│       └── templates/        # カスタム日本語テンプレート群
├── prod/src/                 # 実際のCソースコード
│   ├── calculator.h/c        # 計算機API (@ingroup public_api)
│   ├── samplestruct.h        # UserInfo構造体定義
│   └── *.c                   # その他のサンプルコード
├── docs/doxygen/html/        # Doxygen生成HTML出力
└── docs-src/doxybook/        # Doxybook2生成マークダウン出力
```

### ソースコード構造
- `prod/src/calculator.h` - 関数宣言と`@ingroup public_api`によるAPI分類、ZERO_DEVIDE定数定義
- `prod/src/calculator.c` - `@ingroup public_api`を使用したDoxygenコメント付きの実装
- `prod/src/samplestruct.h` - UserInfo構造体とその他のサンプル定義

### ドキュメント生成パイプライン
1. **Doxygen**: Cソースファイルを解析し、`Doxyfile`設定に基づいてXMLファイルとHTMLドキュメントを生成
2. **プリプロセッシング**: `preprocess.sh`スクリプトが変換前にXMLファイルを処理
3. **Doxybook2**: `doxyfw/config/doxybook-config.json`とカスタムテンプレートを使用してDoxygen XMLをマークダウンに変換
4. **ポストプロセッシング**: `postprocess.sh`スクリプトが`!include`ディレクティブを処理して関連コンテンツを統合、不要ファイルを削除

### 主要設定ファイル
- `doxyfw/config/Doxyfile` - Doxygen基本設定 (UTF-8エンコーディング、全要素抽出、PlantUML対応)
- `Doxyfile.part` - プロジェクト固有の設定オーバーライド (存在時は基本設定に追加結合)
- `doxyfw/config/doxybook-config.json` - Doxybook2設定 (ソートあり、フォルダ使用なし、.md拡張子)
- `doxyfw/config/templates/` - 日本語フォーマット用カスタムJinja2テンプレート群

### 処理スクリプト詳細

#### preprocess.sh（前処理）
XMLファイル変換前の前処理を実行：
- PlantUMLタグ（`<plantuml>`→`\`\`\`plantuml\n@startuml`、`</plantuml>`→`@enduml\n\`\`\``）
- パラメータdirection属性（`direction="in"`→`[in]`プレフィックス追加）
- linebreakタグ（`<linebreak/>`→改行文字への変換準備）

#### postprocess.sh（後処理）
Markdownファイル生成後の後処理を実行：
- **インクルードディレクティブ**: `!include filename.md`でファイル内容を統合
- **ファイルクリーンアップ**: 不要な`struct*.md`、`index_classes.md`等を削除
- **エラー継続**: 単一ファイル処理失敗時も他ファイル処理継続

### カスタムテンプレート
- `nonclass_members_details.tmpl` - API出力セクション組織化のメインテンプレート
- `member_details.tmpl` - 関数・構造体等の個別要素書式設定
- `details.tmpl` - パラメータ・戻り値・警告等の共通書式

## 開発ガイド

### Doxygen仕様とベストプラクティス
- 全パブリック関数に`@ingroup public_api`を付与してAPIをグループ化
- PlantUMLサポート（前提：`/usr/local/bin/plantuml.jar`が存在）
- UTF-8エンコーディングで日本語コメント対応
- 関数宣言はヘッダーで簡潔に、詳細実装コメントは.cファイルに記述
- プロジェクト固有設定は`Doxyfile.part`で上書き（PROJECT_NAME等）

### テンプレート開発時の注意点
- `!include`ディレクティブで関連コンテンツ統合（例：構造体詳細をグループページに統合）
- 相対パス・絶対パス両方をサポート
- デバッグ時は`postprocess.sh`内の`set -x`のコメントアウト解除