# struct-meta

`struct-meta` は、C 構造体をメタデータで記述し、その内容を共通の方法で参照、JSON 変換、対話編集、表示するための実験的な基盤です。  
公開機能は `libstruct_meta` にまとめ、ヘッダーと実装を責務別に分けています。

## 構成

| パス | 責務 |
|---|---|
| `prod/include/struct_meta/meta/` | 記述子、汎用属性、記述子検査 |
| `prod/include/struct_meta/access/` | フィールド、配列要素、文字列パスによるアクセス |
| `prod/include/struct_meta/json/` | cJSON および JSON ファイルとの相互変換 |
| `prod/include/struct_meta/patch/` | 対話形式の編集 |
| `prod/include/struct_meta/print/` | テキスト表示 |
| `prod/src/cmd/struct-meta-gen/` | C ヘッダーから記述子を生成する PoC |
| `prod/src/cmd/struct-meta-sample/` | 生成結果とライブラリを使う動作確認コマンド |

生成ファイルは `gen/` に置かれ、Git では管理しません。  
生成器は Doxygen コメントの `@struct_meta{key}` と `@struct_meta{key=value}` を解析し、構造体またはフィールドの汎用 key/value 属性として保持します。  
属性の記述方法と制約は [アーキテクチャー](docs/architecture.md#doxygen-属性の書式) を参照してください。
文字列とバイト配列の判定、および `meta.kind` と `meta.format` は
[文字列とバイト配列](docs/architecture.md#文字列とバイト配列) を参照してください。

## 実行

```sh
make
make test
./prod/cbin/struct-meta-sample
./prod/cbin/struct-meta-sample --help
```

サンプルでは `init`、`load <path>`、`patch`、`patch <field-path>`、`save <path>`、`cat <path>`、`dump`、`help`、`exit` を使用できます。  
`patch` はメニューを順に辿り、`patch addresses[0].city` は指定したパスの値を直接編集します。  
メニューには現在位置と各候補の完全パスが表示されるため、そのパスを次回の `patch <field-path>` に利用できます。  
設計と依存方向は [アーキテクチャー](docs/architecture.md) を参照してください。
