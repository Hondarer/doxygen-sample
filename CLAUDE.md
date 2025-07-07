# CLAUDE.md

このファイルは、このリポジトリでコードを扱う際のClaude Code (claude.ai/code) への指針を提供します。

## プロジェクト概要

これは、Cソースコードからドキュメントを生成する方法を実証するDoxygenドキュメント学習プロジェクトです。プロジェクトには以下が含まれます：

- C言語のシンプルな計算機関数
- ドキュメント生成用のDoxygen設定
- マークダウンドキュメント用のDoxybook2統合
- 日本語ドキュメント出力用のカスタムテンプレート

## ビルドとドキュメント生成コマンド

### ドキュメント生成
```bash
make docs
```
このコマンドは：
1. `xml/`にDoxygen XMLファイルと`docs/doxygen/`にHTMLファイルを出力
2. XMLファイルに対してプリプロセッシングスクリプトを実行
3. Doxybook2を実行してXMLを`docs-src/doxybook/`のマークダウンに変換
4. `doxyconfig/templates/`のカスタム日本語テンプレートを使用
5. `!include`ディレクティブを処理するポストプロセッシングスクリプトを実行

### ビルド成果物のクリーンアップ
```bash
make clean
```
計算機バイナリ、オブジェクトファイル、`docs/html`と`docs-src/doxybook/`内の生成されたドキュメントを削除します。

## アーキテクチャ

### ソースコード構造
- `src/calculator.h` - 関数宣言とUserInfo構造体を含むヘッダー
- `src/calculator.c` - `@ingroup public_api`を使用したDoxygenコメント付きの実装

### ドキュメント生成パイプライン
1. **Doxygen**: Cソースファイルを解析し、`Doxyfile`設定に基づいてXMLファイルとHTMLドキュメントを生成
2. **プリプロセッシング**: `preprocess.sh`スクリプトが変換前にXMLファイルを処理
3. **Doxybook2**: `doxybook-config.json`とカスタムテンプレートを使用してDoxygen XMLをマークダウンに変換
4. **ポストプロセッシング**: `postprocess.sh`スクリプトが`!include`ディレクティブを処理して関連コンテンツを統合
5. **テンプレート**: `doxyconfig/templates/`に日本語カスタマイズ版が配置

### 主要設定ファイル
- `doxyconfig/Doxyfile` - Doxygen設定 (UTF-8エンコーディング、全要素抽出)
- `doxyconfig/doxybook-config.json` - Doxybook2設定 (グループフィルタなし、全コンテンツ処理)
- `doxyconfig/templates/` - 日本語フォーマット用カスタムJinja2テンプレート

### テンプレート構造
テンプレートシステムは以下を使用：
- `nonclass_members_details.tmpl` - 出力セクションを組織するメインテンプレート
- `member_details.tmpl` - 個別要素の書式設定 (関数、構造体など)
- `details.tmpl` - パラメータリスト、戻り値、警告などの共通テンプレート
- `preprocess.sh` - 変換前にXMLファイルを処理するプリプロセッシングスクリプト
- `postprocess.sh` - `!include {filename}`ディレクティブを処理してファイル内容を置換するポストプロセッシングスクリプト

### ポストプロセッシング機能
- **インクルードディレクティブ**: テンプレートで`!include filename.md`を使用して他の生成ファイルからコンテンツを統合
- **ファイルクリーンアップ**: 処理後に`struct*.md`、`index_classes.md`、`index_namespaces.md`などの不要なファイルを削除
- **エラー処理**: 1つのファイルの処理が失敗しても他のファイルの処理を継続
- **統計レポート**: 処理されたファイル数と成功したインクルード数を表示

### テンプレート開発のメモ
- テンプレートで`!include`ディレクティブを使用して関連コンテンツを統合 (例：構造体の詳細をグループページに統合)
- ポストプロセッシングスクリプトは、インクルードファイルの相対パスと絶対パスの両方をサポート
- インクルードされたコンテンツは`!include`行をそのまま置換
- `postprocess.sh`内の`set -x`のコメントアウトを解除してデバッグ出力を有効化可能

## 開発メモ

プロジェクトはDoxygen `@ingroup`の使用法を実証し、APIドキュメントを論理的なグループに整理します。すべてのパブリック関数は、フィルタリングされたドキュメント生成のために`@ingroup public_api`でタグ付けされています。