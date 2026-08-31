# AGENTS.md

## 対象

本書は `app/struct-meta/` 以下の変更に適用します。  
作業前にルートの `AGENTS.md`、本書、`README.md`、作業内容に該当するスキル、[アーキテクチャー](docs/architecture.md) の順に確認してください。

## 責務と依存方向

- `layout` は対応する型の表と x86_64 のレイアウト規則を定義し、他のカテゴリへ依存しません。
- `parse` は C ヘッダーを構文解析して AST を作り、他のカテゴリへ依存しません。
- `catalog` は `layout` と `parse` と `meta` を使い、記述子の構築、索引、寿命を担います。
- `meta` は記述子、汎用属性、記述子の索引を定義し、他のカテゴリへ依存しません。
- `access` は `meta` のみに依存し、ポインター演算を集約します。
- `json`、`patch`、`print` は `meta` と `access` を利用します。
- JSON ファイル入出力は cJSON 変換を利用します。
- `parse` は汎用 Doxygen 属性を解析し、属性名の意味は解釈しません。
- 公開入口では、構造体の内容へアクセスする前に記述子を検査してください。
- Doxygen は公開 API 用と内部用の 2 系統で生成します。`prod/Doxyfile.part.public` は `prod/include/` だけを、`prod/Doxyfile.part.internal` は `prod/` 全体を対象とします。
- `docs/doxybook2_public/` と `docs/doxybook2_internal/` は自動生成物です。手作業で変更せず、Doxygen コメントを変更してから `make doxy` で再生成してください。

記述子を得る経路は、生成 C ソースを組み込む事前組み込み型と、実行時に構文解析する事後解析型の 2 系統です。  
どちらも `struct_meta_catalog` を返すため、利用側は経路を意識しません。  
新しい機能は、片方だけで使える形にせず、カタログ API の上に載せてください。

解析器はフル C パーサーではありません。  
対応範囲を変更するときは [アーキテクチャー](docs/architecture.md) も更新してください。  
受け付ける型を増やすときは、LP64 と LLP64 で幅が一致することを必ず確認してください。  
型の表と大きさ・アラインメントの正本は `struct_meta_internal_layout_find_type()` の 1 箇所だけです。  
生成物は x86_64 の Linux と Windows の間でバイト互換であることを契約としています。  
`long` と `unsigned long` は幅が異なるため、解析器が拒否します。

レイアウトの計算は `layout` に集約し、生成コードへ `_Static_assert` を出力してコンパイラの実レイアウトと照合します。  
この照合を外すと、事後解析型の正しさを検証する手段が無くなります。  
`layout` の規則や表を変えたときは、`make` が通ることを必ず確認してください。

`parse` はライブラリの一部です。プロセスを終了させず、`struct_meta_diagnostic` へ書いて結果コードを返してください。  
flex と bison は再入可能な構成 (`reentrant` と `api.pure`) を維持し、状態をグローバル変数へ戻さないでください。

`prod/src/cmd/makelocal.mk` の順序は、`struct-meta-gen`、`struct-meta-sample` のまま維持してください。

## 局所確認

```sh
make clean
make
make test
make doxy
```

ビルド後は `app/struct-meta/` 以下の `.warn` ファイルを確認してください。  
サンプルを手動確認する場合は、`init` で 2 系統の経路をそれぞれ選んでください。

```sh
./prod/cbin/struct-meta-sample
```

```text
init sample_types                                       事前組み込み型
init prod/src/cmd/struct-meta-sample/sample_types.h     事後解析型
```

同じ構造体一覧が出て、`dump` までの結果が一致することを確認してください。  
1 プロセス内で `init` を繰り返して対象を切り替えられること、および `init` が失敗したときに
直前の対象がそのまま残ることもあわせて確認してください。
