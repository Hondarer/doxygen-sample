# struct-json-sample

flex/bison でプログラム本体が実際に使う C ヘッダーを解析し、構造体のメタデータ (型・フィールド名・ネスト・配列構成) を自動導出します。そのメタデータと cJSON を組み合わせて、構造体と JSON の相互変換、JSON ファイルの読み書き、対話形式での値パッチを行う PoC サンプル app です。

## 入口

- [作業規則](AGENTS.md)
- [設計思想](docs/architecture.md)
- [進捗](docs/progress.md)

## 構成

| ディレクトリ | 役割 |
| --- | --- |
| `prod/include/struct_json/` | 公開ヘッダー (記述子データ モデル、公開 API) |
| `prod/libsrc/struct_json/` | メタデータ駆動の汎用エンジン (struct⇔JSON 変換、ファイル入出力、対話パッチ) |
| `prod/src/cmd/structgen/` | ヘッダー解析ツール (flex/bison 文法、メタデータ C ソース生成) |
| `prod/src/cmd/struct-json-sample/` | 動作確認コマンド、解析対象の構造体定義 (`sample_types.h`) |
| `test/` | 単体テスト |

## ビルドと実行

```sh
make
./prod/cbin/struct-json-sample
make test
```

起動後は対話でサブコマンドを発行します。`load` と `save` はファイル名を引数に取ります。

| 入力 | 動作 |
| --- | --- |
| `init` | 記述子のサイズ分をゼロで初期化する |
| `load <path>` | JSON ファイルを読み込む |
| `patch` | 対話でフィールドを編集する |
| `save <path>` | JSON ファイルへ書き出す |
| `dump` | メモリ上の内容を表示する |
| `help` | コマンド一覧を表示する |
| `exit` | 終了する |
| 空行 | `help` と同じ |

例: `init` → `save out.json` で新規保存、`load out.json` → `patch` → `save out.json` で編集です。
