# doxygen-sample

Doxygen を使用したドキュメント生成のサンプルプロジェクト

## 概要

C 言語のサンプルソースコードから高品質な日本語ドキュメントを生成するための設定とコードの例を提供しています。

## 特徴

- **Doxygen フレームワーク**: サブモジュール `doxyfw` として統合された包括的なドキュメント生成システム
- **日本語対応**: 日本語コメントに最適化されたカスタムテンプレート
- **複数形式出力**: HTML と Markdown の両方でドキュメント生成
- **サンプルコード**: 実際の C プロジェクトでの使用例

## クイックスタート

### サブモジュール初期化

```bash
git submodule update --init --recursive
```

### ドキュメント生成

```bash
cd doxyfw && make docs
```

### 生成されたドキュメントの確認

- HTML版: `docs/doxygen/index.html`
- Markdown版: `docs-src/doxybook/`

### クリーンアップ

```bash
cd doxyfw && make clean
```

## サンプルコード

- `prod/src/calculator.h` / `calculator.c` - 計算機能のサンプル実装
- `prod/src/samplestruct.h` - 構造体定義のサンプル

## 詳細ドキュメント

プロジェクト構造、設定方法については [CLAUDE.md](./CLAUDE.md) をご覧ください。

## サブモジュール

このプロジェクトは `doxyfw` サブモジュールを使用しています。サブモジュールの詳細については [doxyfw/CLAUDE.md](./doxyfw/CLAUDE.md) を参照してください。

## ライセンス

[LICENSE](./LICENSE) を参照してください。
