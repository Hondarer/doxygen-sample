# 進捗

計画は [flex-bison-app-cjson-json-makefw-gen-fle-sparkling-prism.md](../../../../.claude/plans/flex-bison-app-cjson-json-makefw-gen-fle-sparkling-prism.md) を参照してください (計画は作業端末のローカル パスであり、恒久的な参照先ではありません。設計の正本は [architecture.md](architecture.md) です)。

## Phase 0: 足場

- 状態: 完了
- `app/empty-lib` を土台に app 骨格を作成。`appdeps.mk` を `com_util cjson` に設定。
- `docs/architecture.md`/`docs/progress.md` を作成。

## Phase 1: makefw 拡張 + フラット構造体の JSON 変換

- 状態: 完了
- スコープ: `structgen` はネスト・配列非対応 (`int`/`unsigned`/`double`/`float`/`char[N]` のフラット構造体のみ)。ただし `struct_json` エンジン自体はネスト・配列を含めて汎用実装済み (単体テストで検証)。
- 実施内容:
    - `_flex_bison_compile.mk` 新設 (`.l`/`.y` → `gen/` → `obj/` の自動変換・コンパイル。`GENDIR_EXTRA_C` によるアプリ独自コード生成の合流点も兼ねる)。
    - `_collect_srcs.mk` に `SRCS_L`/`SRCS_Y` 収集を追加。
    - `makesrc_c_cpp.mk`/`makelibsrc_c_cpp.mk` に `_flex_bison_compile.mk` の include と `OBJS += $(GENDIR_OBJS)` を追加。
    - `makeparts.md` に「例 6: flex/bison (.l/.y) を埋め込む」を追記。
    - `structgen` (flex/bison ヘッダー解析ツール) を実装し、`sample_types.h` から `person` 構造体のメタデータ C ソースを生成することを確認。
    - `struct_json_to_json.c`/`from_json.c`/`fileio.c`、`struct-json-sample` コマンド (`--save`/`--load --dump`) を実装し、実際に JSON 往復変換を確認。
    - 単体テスト 2 系統 (`structJsonToJsonTest`/`structJsonFromJsonTest`、計 6 ケース) が PASS。
- 実装時の設計修正 (計画からの乖離):
    - Linux のリンクは `$(OBJS)` を直接使わず `filter_existing_source_objs.sh` (SRCS_C 由来のみ検出) が対象を決めるため、`OBJS += $(GENDIR_OBJS)` (ビルド トリガー用) に加えて `MAKEFW_EXTRA_OBJS += $(GENDIR_OBJS)` (リンク コマンドへの明示追加) の両方が必要だった。詳細は [architecture.md](architecture.md) を参照。
    - `gen/*.c` のコンパイルには `-I$(GENDIR)` に加え `-I.` (呼び出し元ディレクトリ) も必要だった (`structgen_ast.h` 等の解決のため)。
    - flex/bison 生成コード (`.tab.c`/`.lex.c`) は上流ツールの出力であり改変できないため、cJSON 取り込みと同様に生成コード由来の警告のみ例外的に抑制した (`MAKEFW_FLEXBISON_WARN_SUPPRESS`)。アプリ独自生成の `gen/*.c` (`GENDIR_EXTRA_C`) はこの抑制の対象外。

## Phase 2: ネスト・配列対応

- 状態: 完了
- 実施内容:
    - `structgen.y`: `type_spec` に `IDENT` (他の `typedef struct` 名) を追加し、ネスト メンバーをパース可能にした。配列 (`field[N]`) の制限 (char[] のみ) を撤廃し、任意の型 (プリミティブ・ネスト構造体) に固定長配列を許可した。
    - `structgen_ast.h`/`.c`: `sg_field` に `is_struct_type` を追加。`sg_typespec` (型スペリング + ネスト判定フラグ) を新設。
    - `structgen_emit.c`: ネスト構造体を参照する場合、依存先の記述子を先に (`static const`) 出力してから、要求された対象の記述子を (`const`、外部リンケージ) 出力する再帰実装に変更。
    - `sample_types.h`: `address` (ネスト用構造体) を追加し、`person` に `home` (ネスト スカラー)、`addresses[2]` (ネスト配列)、`scores[3]` (プリミティブ配列) を追加。
    - `struct-json-sample.c`: 追加フィールドの設定・表示に対応。
    - 動作確認: `--save`/`--load --dump` でネスト・配列を含む JSON の往復変換を確認。単体テスト (6 ケース) は引き続き PASS。

## Phase 3: 対話パッチ

- 状態: 完了
- 実施内容:
    - `struct_json_patch.c`: `com_util_prompt` を使い、フィールド一覧 → 配列要素選択 → ネスト構造体への降下 → スカラー値編集、をメニュー形式で辿る対話ナビゲーションを実装。空行で 1 階層戻る (最上位では対話セッション終了)。
    - `sj_patch_interactive()` を `struct_json.h` に公開 API として追加。ファイル保存は行わず、呼び出し元が `sj_save_file` を別途呼ぶ設計 (関心の分離)。
    - `struct-json-sample.c` に `--patch <path>` サブコマンドを追加 (読み込み → 対話編集 → 保存)。
    - 動作確認: 標準入力へ固定シーケンスを流し込み (TTY でない場合 `com_util_prompt` は `fgets` にフォールバック)、`person.age` の直接編集、`person.addresses[0].city` (配列要素中のネスト構造体のフィールド) の編集を確認。いずれも JSON ファイルへ正しく反映された。
    - 対話パッチの自動テストは追加していない (計画どおり、手動確認に留める)。

## ヘッダー単位の型一覧

- 状態: 完了
- 実施内容:
    - `structgen` の `--struct` を廃止し、ヘッダー内の `typedef struct` をすべて変換する。
    - 生成物を `gen/<stem>_meta.c` と `gen/<stem>_meta.h` にした。`.h` に宣言順の enum と取得関数を置く。
    - 個別の記述子は `static` とし、利用側は `sample_types_desc(SAMPLE_TYPES_PERSON)` のように一覧のキーで取得する。
    - `makepart.mk` の宣言を `STRUCTGEN_HEADERS` に変更した。
    - 名前からの解決関数は未実装 (次段階)。

## Doxygen コメントの brief

- 状態: 完了
- 実施内容:
    - `sj_field_desc` / `sj_struct_desc` に `brief` を追加した。
    - `structgen` は前置 `@brief` と後置 Doxygen コメントから `brief` を取り、記述子へ埋め込む。`@file` コメントは構造体へ付けない。
    - 対話パッチは `brief` があるときだけ名前の横へ表示する。
    - `sample_types.h` のメンバーへ後置コメントを追加し、構造体の前置 `@brief` と両方の経路を見せる。

## 対話サブコマンドのデモコマンド

- 状態: 完了
- 実施内容:
    - `struct-json-sample` の起動引数を廃止し、対話で `init` / `load <path>` / `patch` / `save <path>` / `dump` / `help` / `exit` を発行する。ルートの空行は `help` と同じで、終了は `exit`。
    - `--save` / `--load` / `--patch` は使わない。
    - 1 つの `person` インスタンスを持ち回り、`init` のあと `patch` して `save` できるようにした。
    - コマンドは `SAMPLE_TYPES_PERSON` の記述子だけを使う。領域は `desc->size` で確保し、`init` はゼロ初期化、`dump` は `sj_print` で記述子を歩く。

## JSON アノテーション

- 状態: 完了
- 実施内容:
    - `@json_name{...}` / `@json_ignore` / `@json_required` を Doxygen コメントから取り、記述子の JSON 層へ載せた。
    - JSON キー欠落は既定で許し、`@json_required` のときだけエラーにする。
    - `@json_ignore` は JSON の読み書きだけを外し、dump / patch の対象には残す。
    - `prod/Doxyfile.part` の `ALIASES` で、タグを `@par` 段落として見えるようにした。

## Phase 4: com_util への切り出し検討

- 状態: 未着手
