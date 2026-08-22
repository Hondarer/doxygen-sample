# AGENTS.md

## 対象

本書は `app/struct-meta/` 以下の変更に適用します。  
作業前にルートの `AGENTS.md`、本書、`README.md`、作業内容に該当するスキル、[アーキテクチャー](docs/architecture.md) の順に確認してください。

## 責務と依存方向

- `meta` は記述子と汎用属性を定義し、他のカテゴリへ依存しません。
- `access` は `meta` のみに依存し、ポインター演算を集約します。
- `json`、`patch`、`print` は `meta` と `access` を利用します。
- JSON ファイル入出力は cJSON 変換を利用します。
- `struct-meta-gen` は生成時だけ JSON 用 Doxygen 属性を解釈します。
- 公開入口では、構造体の内容へアクセスする前に記述子を検査してください。

生成器はフル C パーサーではありません。  
対応範囲を変更するときは [アーキテクチャー](docs/architecture.md) も更新してください。  
`prod/src/cmd/makelocal.mk` の順序は、`struct-meta-gen`、`struct-meta-sample` のまま維持してください。

## 局所確認

```sh
make clean
make
make test
make doxy
```

ビルド後は `app/struct-meta/` 以下の `.warn` ファイルを確認してください。  
サンプルを手動確認する場合は `./prod/cbin/struct-meta-sample` を実行してください。
