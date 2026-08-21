# 設計思想

## 背景

C 構造体には実行時リフレクションがありません。設定値の対話編集や JSON との相互変換を行うには、構造体の型・フィールド名・ネスト・配列構成をプログラムに伝える「メタデータ」が別途必要です。このメタデータを手書きで構造体ごとに二重管理すると、実ヘッダーとの乖離 (更新漏れ) が避けられません。

本 app は、**プログラム本体が実際に使うヘッダーそのものを flex/bison で解析してメタデータを自動導出する** ことで、この二重管理を避けます。

## 主要な設計判断

### レイアウト計算はコンパイラに任せる

`structgen` (ヘッダー解析ツール) は、ソース テキストから「型のスペリング」「フィールド名」「配列次元の整数リテラル」、および Doxygen コメントの短い説明 (`brief`) を抽出します。  
`brief` は前置コメントの `@brief` 本文、または後置コメント (`/**< ... */` など) の本体です。  
JSON 入出力向けの拡張として、同じコメントに `@json_name{...}` / `@json_ignore` / `@json_required` を書けます。dump / patch は C のメンバー名を使い、JSON の読み書きだけがこれらのタグを見ます。  
フィールドのオフセットやサイズ (`offsetof`/`sizeof`) は自分で計算せず、生成する C コードに `offsetof(Type, field)` のような式をそのまま埋め込み、**実ヘッダーを `#include` した状態でコンパイラに計算させます**。

理由: パディング・アラインメント・ビット幅は、コンパイラ・ターゲット (32/64bit)・OS (Linux/Windows) によって異なります。`structgen` 側でこれらを再現しようとすると、コンパイラの挙動を二重に実装することになり、ズレが生じたときに検出できません。コンパイラに計算させれば、実際にビルドされる構造体と必ず一致します。

### structgen はフル C パーサーではない

対象は `typedef struct [tag] { field-decl* } Name;` という形の宣言に限定した寛容なスキャナー/パーサーです。

- 対応: `int` / `unsigned` / `char` 配列 (`char name[N]`、JSON 文字列として扱う) / `float` / `double`、同一ヘッダー内の `typedef struct` ネスト メンバー、固定長 1 次元配列。
- 非対応 (検出時は行番号付きで診断しエラー終了): ポインター、共用体、ビット フィールド、関数ポインター、多次元配列。

フル C 文法を実装しないのは、対象が「JSON で表現できる値を持つ設定的な構造体」に限られる PoC であるためです。対象外の宣言 (関数プロトタイプ等) は `;` 区切りで読み飛ばします。

> [!NOTE]
> `struct_json` エンジン (`struct_json_to_json.c`/`from_json.c`) 自体は、記述子 (`sj_struct_desc`/`sj_field_desc`) さえ渡されればネスト構造体・固定長配列を含めて汎用的に変換できます。単体テストは手書き記述子でこの経路を検証します。

### 生成は宣言駆動・単一フェーズ

当初、`pre-build` フック + 生成物を `#include` するラッパー `.c` という案を検討しましたが、この方式には次の問題がありました。

- `pre-build` は「`build` 開始前に無条件で実行される処理」であり、`SRCS_C` のワイルドカード確定 (パース時) との間に評価順序のギャップがある (2 回目の `make` でようやく反映される)。

しかし、解析対象のヘッダーは **アプリ側が `makepart.mk` で静的に宣言する** ため、生成される `_meta.c` / `_meta.h` のパスはパース時点で決定論的に分かります。そこで、Make の明示ルール (前提条件: ヘッダー・`structgen` 実行体) として「ヘッダー → メタデータ生成 → コンパイル」を表現し、1 回の `make` 実行内で完結させます (`prod/src/cmd/struct-json-sample/makepart.mk` の `STRUCTGEN_HEADERS` を参照)。ラッパー `.c` や `pre-build` フックは使いません。

`structgen` はヘッダー内の `typedef struct` をすべて変換し、型一覧 (宣言順の enum と取得関数) を同じ stem のヘッダーへ書き出します。  
個別の記述子は生成 C の `static` とし、利用側は一覧のキーで記述子を取得します。

`structgen` 実行体自体は別サブディレクトリ (`prod/src/cmd/structgen/`) の再帰 `make` で作られるため、再帰 make の限界により依存関係をこのファイル内だけでは表現できません。そのため `prod/src/cmd/makelocal.mk` で `SUBDIRS` を明示し、`structgen` を `struct-json-sample` より先にビルドさせています。

### makefw 拡張の範囲

`.l`/`.y` を置くだけで `$(GENDIR)` (`gen/`) に中間生成物を配置し自動コンパイルする仕組み (`framework/makefw/makefiles/_flex_bison_compile.mk`) は、flex/bison 由来かどうかを問わない汎用ルールとして makefw 本体に追加しました。一方、「ヘッダーを解析してメタデータ C ソースを生成する」という `structgen` の呼び出し (`STRUCTGEN_HEADERS` の宣言と生成ルール) は、この app に固有のロジックであり、makefw 本体には追加していません。

Windows で WinFlexBison を使う場合は、`BISON=win_bison` と `FLEX=win_flex` を指定します。makefw は Windows で `FLEXFLAGS` の既定値を `--wincompat` とし、MSVC で利用できるコードを生成します。変換対象は `PLATFORM_*` で出し分けず、クロスプラットフォームな外部コマンドとして扱います。

生成ツール `structgen` の実行ファイル名は、Linux の `structgen` と Windows の `structgen.exe` を `PLATFORM_*` で明示的に切り替えます。これにより、生成ルールの前提条件と実際の Windows ビルド成果物を一致させます。

#### 実装時に判明した詳細 (Linux でのリンク組み込み)

計画段階では「`OBJS` へ合流させれば十分」と想定していましたが、実装・実機ビルドで確認したところ、Linux の実行体/ライブラリのリンク入力は `$(OBJS)` 変数を直接使わず、`bin/filter_existing_source_objs.sh` が `SRCS_C` 由来のオブジェクトだけを検出してリンク コマンドを組み立てる方式でした。そのため、次の 2 つを両方行う必要があります。

- `OBJS += $(GENDIR_OBJS)` : Make の依存グラフに乗せ、`make` 実行時に実際にコンパイルさせる (ビルド トリガー)。
- `MAKEFW_EXTRA_OBJS += $(GENDIR_OBJS)` : `filter_existing_source_objs.sh` の対象外であるこれらのオブジェクトを、リンク コマンドの引数へ明示的に追加する (`framework/makefw/makefiles/_ident.mk` の `_IDENT_MANIFEST_OBJ` が同じ 2 段構えの先例)。

また、`$(GENDIR)/%.c` のコンパイルには `-I$(GENDIR)` に加え `-I.` (呼び出し元カレント ディレクトリ) も必要です。`structgen_ast.h` のような、生成元ディレクトリに置かれた補助ヘッダーを `gen/*.c` (bison/flex 生成物) が `#include` するためです。

flex/bison 自体が生成する `.tab.c`/`.lex.c` は上流ツールの出力であり本リポジトリでは改変できないため、cJSON 等の外部 OSS 取り込みと同様の考え方で、生成コードに起因する警告 (`-Wconversion`/`-Wsign-conversion`/`-Wswitch-default`/`-Wpadded`) に限り例外的に抑制しています (`MAKEFW_FLEXBISON_WARN_SUPPRESS`)。`GENDIR_EXTRA_C` 経由でアプリが自前生成する `.c` (`gen/sample_types_meta.c` 等) は抑制対象に含めず、通常の警告基準をそのまま適用します。

## 段階的実装

Phase 0〜4 の内容は [進捗](progress.md) を参照してください。将来的に `struct_json` エンジン (メタデータ→JSON 変換、ファイル入出力、対話パッチ) を `com_util` へ移す判断は Phase 4 で改めて行います。
