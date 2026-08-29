# AGENTS.md

## 対象

本書は `app/struct-meta/` 以下の変更に適用します。  
作業前にルートの `AGENTS.md`、本書、`README.md`、作業内容に該当するスキル、[アーキテクチャー](docs/architecture.md) の順に確認してください。

## 責務と依存方向

- `meta` は記述子、汎用属性、記述子の索引を定義し、他のカテゴリへ依存しません。
- `access` は `meta` のみに依存し、ポインター演算を集約します。
- `json`、`patch`、`print` は `meta` と `access` を利用します。
- JSON ファイル入出力は cJSON 変換を利用します。
- `struct-meta-gen` は生成時だけ汎用 Doxygen 属性を解析し、属性名の意味は解釈しません。
- 公開入口では、構造体の内容へアクセスする前に記述子を検査してください。
- Doxygen は公開 API 用と内部用の 2 系統で生成します。`prod/Doxyfile.part.public` は `prod/include/` だけを、`prod/Doxyfile.part.internal` は `prod/` 全体を対象とします。
- `docs/doxybook2_public/` と `docs/doxybook2_internal/` は自動生成物です。手作業で変更せず、Doxygen コメントを変更してから `make doxy` で再生成してください。

生成器はフル C パーサーではありません。  
対応範囲を変更するときは [アーキテクチャー](docs/architecture.md) も更新してください。  
生成器が受け付ける型を増やすときは、LP64 と LLP64 で幅が一致することを必ず確認してください。  
生成物は x86_64 の Linux と Windows の間でバイト互換であることを契約としています。  
`long` と `unsigned long` は幅が異なるため、生成器が拒否します。  
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
