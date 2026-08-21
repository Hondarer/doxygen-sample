# AGENTS.md

## 対象

この app は、flex/bison でヘッダー中の構造体宣言を解析してメタデータを生成し、cJSON と組み合わせて構造体と JSON の相互変換・ファイル入出力・対話形式での値パッチを行う PoC サンプルです。

## 必須参照

- [README.md](README.md)
- [設計思想](docs/architecture.md)
- [進捗](docs/progress.md)
- [makeparts](../../framework/makefw/docs/makeparts.md) (`.l`/`.y` の自動コンパイル、`gen/` の扱い)

## 構成

- `prod/include/struct_json/` - 公開ヘッダー (記述子データ モデル、公開 API)。
- `prod/libsrc/struct_json/` - メタデータ駆動の汎用エンジン (struct⇔JSON 変換、ファイル入出力、対話パッチ)。将来的に `com_util` へ移すことを想定し、命名・戻り値は `com_util/base/result.h` の規約に合わせています。
- `prod/src/cmd/structgen/` - ヘッダー解析ツール本体 (flex/bison 文法、メタデータ C ソース生成)。
- `prod/src/cmd/struct-json-sample/` - 動作確認コマンド。解析対象のヘッダーは `makepart.mk` の `STRUCTGEN_HEADERS` で宣言します。
- `test/` - `struct_json` エンジンの単体テスト。

## 注意点

- 構造体の正本は `prod/src/cmd/struct-json-sample/sample_types.h` です。JSON 変換用に構造体を二重定義しないでください。
- `structgen` はフル C パーサーではありません。対応スコープは `docs/architecture.md` を参照してください。
- 生成される `.gen.c` は `$(GENDIR)` (`gen/`) に置かれ、`git` 管理対象外です。ラッパー `.c` や `pre-build` フックではなく、`makepart.mk` の明示ルールで生成します (`docs/architecture.md` 参照)。
- `prod/src/cmd/makelocal.mk` の `SUBDIRS` 宣言順 (`structgen` → `struct-json-sample`) を変更しないでください。ビルド順序の保証に必要です。
- 作業の節目ごとに [docs/progress.md](docs/progress.md) を更新してください。

## 確認コマンド

- `make` (ビルド)
- `make test` (単体テスト)
- `./prod/cbin/struct-json-sample` (動作確認。対話で `init` / `load <path>` / `patch` / `save <path>` / `dump` / `help` / `exit`。空行は `help`。詳細は README.md)
