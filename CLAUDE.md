# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

このファイルは、このリポジトリでコードを扱う際の Claude Code (claude.ai/code) への指針を提供します。

<important_rules>
性能改善やリファクタリングを要求された際は、オリジナルのソース解説コメントを維持してください。

**編集に関するルール**

編集を行う際は、編集の目的が精度向上であることを念頭において作業してください。
目的があいまいな場合は、以下のどの編集行為を行うとしているのか、ユーザーに問い合わせてください。

以下が一般的な編集です。

+ Polish: 仕上げの微調整、表現をなめらかにする
+ Refine: 不要部分を削り、精度を高める
+ Edit: 全般的な編集、誤字修正から大幅改変まで幅広い
+ Revise: 内容や構成を再検討して修正

以下の編集は、明確な書き直しや書き換えの指示がない限り、実施しないでください。

+ Rewrite: 大きく書き直す（意味も変わることあり）
+ Rephrase: 同じ意味を別の言い回しで表す
+ Reword: 特定の単語や表現を置き換える
+ Paraphrase: 意味を保ったまま言い換える

**単語の強制**

以下に示す単語は、同義語、同じ意味の他言語の単語に優先して使用すること。

+ Markdown

**表記に関するルール**

このルールは、**ソースコード内のコメントでも有効** です。

+ 本文の文末を `：` で終わらせないでください。きちんと文章として完結させてください。
+ 全角括弧 `（` `）` や全角コロン `：` は使用せず、半角で記述してください。
+ 日本語と英単語の間は、半角スペースを挿入してください。
+ 見出し (#, ##, ...) と本文の間、および、本文とコードブロックの間は、空行を挿入してください。
+ 見出し (#, ##, ...) と見出し (#, ##, ...) の間に水平線 (`----`) を挿入しないでください。
+ 見出し (#, ##, ...) に続く文字列に番号 (1. など) を付与しないでください。
+ 絵文字の使用は最低限度の使用にしてください。シンプルな意味論を持つ絵文字として ✅, ❌, 🟢, 🟡, 🔴 などは許容します。
+ 特に言語の指定のないコードブロックの場合でも、'```text' のように形式を明示してください。

**図に関するルール**

ユーザーに図を提示する際は、原則として、PlantUML 形式としてください。  
plantuml コードブロックへのファイル名付与は不要です。  
PlantUML による記法では、`@startuml` と `caption` に同じタイトル文字列を付与してください。 `title` は使用しないでください。以下に例を示します。

```plantuml
@startuml この図のキャプション
    caption この図のキャプション
    Alice->Bob : Hello
@enduml
```

PlantUML にてフローを説明する際は、アクティビティ図を優先してください。  
シーケンスに着目すべき内容の説明、および、シーケンス図を要求された場合は、シーケンス図としてください。  
適切であると判断された場合は、他の PlantUML 形式も活用してください。

PlantUML 形式での表現が困難な場合は、marmaid 形式としてください。  
marmaid による記法では、コードブロックの caption にタイトル文字列を付与してください。以下に例を示します。

```{.mermaid caption="Mermaid の図キャプション"}
sequenceDiagram
    Alice->>John: Hello John, how are you?
```

指示があった場合は、draw.io にインポート可能な xml の提示をしてください。  
</important_rules>

## プロジェクト概要

これは Doxygen を使用したドキュメント生成のサンプルプロジェクトです。C 言語のサンプルソースコードから高品質な日本語ドキュメントを生成するための設定とコードの例を提供します。

Doxygen フレームワーク機能はサブモジュール `doxyfw` として統合されています。

### 主要コンポーネント

- `prod/src/` - サンプル C ソースコード (calculator 関数群、構造体定義)
- `doxyfw/` - Doxygen ドキュメント生成フレームワーク (git サブモジュール)
- `Doxyfile.part` - プロジェクト固有の Doxygen 設定

## サブモジュール: doxyfw

本プロジェクトは、Doxygen ドキュメント生成機能を `doxyfw` サブモジュールとして利用しています。

### サブモジュールの機能

`doxyfw` サブモジュールは以下の機能を提供します:

- Doxygen 基本設定ファイル
- Markdown 変換用の Doxybook2 設定・テンプレート
- XML 前処理・後処理スクリプト
- 日本語ドキュメント出力用のカスタムテンプレート

詳細な使用方法や設定については、`doxyfw/CLAUDE.md` を参照してください。

## 主要コマンド

### ドキュメント生成

```bash
cd doxyfw && make docs
```

### クリーンアップ

```bash
cd doxyfw && make clean
```

## プロジェクト構造

```text
doxygen-sample/                    # このプロジェクト
├── doxyfw/                     # Doxygen フレームワーク (git submodule)
│   ├── Doxyfile               # Doxygen 基本設定
│   ├── doxybook-config.json   # Doxybook2 設定
│   ├── templates/             # カスタム日本語テンプレート群
│   ├── docs-src/              # フレームワーク技術ドキュメント
│   ├── Makefile               # ドキュメント生成用 Makefile
│   └── CLAUDE.md              # フレームワーク詳細ドキュメント
├── Doxyfile.part               # Doxygen プロジェクト固有設定
├── prod/src/                   # サンプル C ソースコード
│   ├── calculator.h           # 関数宣言とマクロ定義
│   ├── calculator.c           # 計算関数の実装
│   └── samplestruct.h         # サンプル構造体定義
├── docs/doxygen/               # 生成される HTML ドキュメント
├── docs-src/doxybook/          # 生成される Markdown ドキュメント
└── xml/                        # Doxygen XML 中間ファイル
```

## サンプルソースコード

### calculator モジュール

基本的な数値計算を行う関数群のサンプルです。

- `prod/src/calculator.h:calculator.h` - 関数宣言と `@ingroup public_api` による API 分類、ZERO_DEVIDE 定数定義
- `prod/src/calculator.c:calculator.c` - `@ingroup public_api` を使用した Doxygen コメント付きの実装

### サンプル構造体

構造体のドキュメント化の例を提供します。

- `prod/src/samplestruct.h:samplestruct.h` - UserInfo 構造体とその他のサンプル定義
