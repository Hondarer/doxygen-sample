# struct-json-sample

flex/bison でプログラム本体が実際に使う C ヘッダーを解析し、構造体のメタデータ (型・フィールド名・ネスト・配列構成) を自動導出します。そのメタデータと cJSON を組み合わせて、構造体と JSON の相互変換、JSON ファイルの読み書き、対話形式での値パッチを行う PoC サンプル app です。

## 入口

- [作業規則](AGENTS.md)
- [設計思想](docs/architecture.md)
- [進捗](docs/progress.md)

## 構成

| ディレクトリ | 役割 |
| --- | --- |
| `prod/include/struct_json/` | 公開ヘッダー (記述子データモデル、公開 API) |
| `prod/libsrc/struct_json/` | メタデータ駆動の汎用エンジン (struct⇔JSON 変換、ファイル入出力、対話パッチ) |
| `prod/src/cmd/structgen/` | ヘッダー解析ツール (flex/bison 文法、メタデータ C ソース生成) |
| `prod/src/cmd/struct-json-sample/` | 動作確認コマンド、解析対象の構造体定義 (`sample_types.h`) |
| `test/` | 単体テスト |

## ビルドと実行

```sh
make
./prod/cbin/struct-json-sample --save out.json
./prod/cbin/struct-json-sample --load out.json --dump
./prod/cbin/struct-json-sample --patch out.json
make test
```
