# コーディング規範

## 概要

C / C++ コードでの整数型の選択、関数引数の異常入力対応、変数宣言位置の扱いなど、コーディング規範を本書に集約します。  
適用範囲は主に `app/` 配下の C / C++ コードです。

本書は、ログ / トレース、テスト規約、ヘッダー設計など、コーディング規範を順次追加していくことを想定しています。  
現版では「命名規則」「構造体パディングの扱い」「整数型の選択」「関数引数の異常入力対応」「エラー処理と戻り値規約」「変数宣言位置と命令文の関係」「式の括弧」「関数引数の const 付与と Doxygen 方向タグ」「API 設計における概念の分離」「Doxygen コメントのプレースホルダー表記」「Doxygen コメントの @p などコマンド引数と日本語句読点の間隔」「Doxygen コード例内のコメント形式」を記載します。

本書は一般的な方針を定めるものです。  
各 app のドキュメントに優先事項、特化事項がある場合は、それに従ってください。

本書は、再度同じ種類のリファクタリング作業を行うときに決定論的に判断できる詳細さで記述します。  
方針を追記するときは、判定基準の表、判定手順、望ましい/望ましくないコード例、例外条件、検証コマンドを含めます。

関連する既存ガイドラインは [参照](#参照) を参照してください。

## 命名規則

### 適用範囲と用語の定義

本章は `app/` 配下の C / C++ コードで定義するすべての識別子を対象とします。  
外部 OSS 由来のコード、過去資産の保守的移植が app 配下の AGENTS.md で宣言されているもの、OS / SDK が定義する型やマクロの alias は対象外です。

命名を決める前に、対象のシンボルがどのスコープに属するかを次の手順で判定します。  
上から順に評価し、最初に一致した行のスコープとします。

| 判定順 | スコープ | 判定基準 |
|---|---|---|
| 1 | 公開 | `prod/include/` 配下のヘッダーで宣言されている |
| 2 | ライブラリ内共有 | `prod/include_internal/` 配下のヘッダーで宣言されている |
| 3 | ファイル内 | どのヘッダーでも宣言されず、`static` が付いている |
| 4 | 関数内ローカル | 関数本体の内側で宣言されている |

スコープはヘッダーの配置と `static` の有無で判定します。  
ライブラリ内共有の関数・型・外部リンケージ変数は、名前にも公開境界のマーカー (`_internal_`) を含め、公開シンボルと区別します。

ヘッダー **ファイル名** の `_internal` サフィックスは、同名の公開ヘッダーが存在するときに限り付けます。  
シンボル規則の付与条件とは異なります。詳細は後述の「ヘッダー ファイル名の _internal」を参照してください。

### スコープ別一覧

`<lib>` はライブラリ接頭辞を表すプレースホルダーです。

| スコープ | リンケージ | 宣言場所 | 記法 | 接頭辞 | 例 |
|---|---|---|---|---|---|
| 関数内ローカル変数 | なし | 関数本体 | snake_case | 付けない | `result`、`buf_size` |
| static 関数 | 内部 | `.c` 内 | snake_case | 付けない | `parse_header` |
| ファイル内共有変数 | 内部 | `.c` のファイル スコープ | snake_case | `s_` | `s_instance_count` |
| ライブラリ内共有関数 | 外部 | `include_internal/` | snake_case | `<lib>_internal_` | `sample_internal_registry_add` |
| ライブラリ内共有変数 | 外部 | `include_internal/` の `extern` | snake_case | `g_<lib>_internal_` | `g_sample_internal_default_config` |
| 公開関数 | 外部 | `include/` | snake_case | `<lib>_` | `sample_file_get_size` |
| 公開共有変数 | 外部 | `include/` の `extern` | snake_case | `g_<lib>_` | `g_sample_default_limits` (必要最低限に厳選) |
| 型 (struct / enum / union / 関数ポインター) | - | 宣言場所に従う | snake_case | 公開は `<lib>_`、ライブラリ内共有は `<lib>_internal_` | `sample_context`、`sample_internal_registry`、`sample_hook_fn` |
| 列挙定数 / マクロ | - | - | 全大文字 | `<LIB>_` | `SAMPLE_TRACE_LEVEL_INFO` |

表の各行は、次の原理から導かれます。  
個別のケースで判断に迷う場合は、この原理に立ち返って決定してください。

- 変数の `s_` / `g_` は **リンケージ** と「これは変数である」ことを表します。`static` なら `s_`、外部リンケージなら `g_` です。
- ライブラリ接頭辞は **リンカー名前空間** を表します。外部リンケージを持つシンボルにのみ付け、`static` 関数・`s_` 変数には付けません。
- `_internal_` は **公開境界** を表します。`include_internal/` で宣言する関数・型には `<lib>_internal_`、外部リンケージ変数には `g_<lib>_internal_` の形で付けます。公開ヘッダーで宣言するシンボルと `static` には付けません。

### 予約識別子の回避

規格が処理系用に予約している識別子の形式は使用しません。  
将来の libc や処理系の拡張とシンボルが衝突し、原因の分かりにくいビルド エラーや未定義動作を招くためです。  
本リポジトリはテストや一部実装が C++ であるため、C ソースでも C++ の予約規則を踏まえた形式を避けます。

| 禁止する形式 | 根拠 |
|---|---|
| `_t` サフィックス | POSIX.1 の名前空間規定は、標準ヘッダーをインクルードしたときに `_t` で終わる型名を処理系用に予約します |
| アンダースコアで始まるファイル スコープ識別子 | C 標準は、アンダースコアで始まるすべての識別子を、ファイル スコープの通常識別子およびタグ名前空間で予約します |
| 連続するアンダースコア (`__`) を含む識別子 | C++ 標準は、識別子の任意の位置に連続する `__` を含む名前を処理系用に予約します。C 単独では途中の `__` への制約は相対的に緩いですが、C++ とヘッダーやテストを共有するため、ユーザーが定義する識別子では使いません |

`_t` の禁止は `typedef struct` / `typedef enum` / `typedef union` / 関数ポインター typedef のすべてに適用します。  
アンダースコア始まりの禁止と `__` の禁止は、関数名、変数名、型名、マクロ名、インクルード ガードなど、ユーザーが **定義** する識別子に適用します。  
インクルード ガードにおける予約識別子の詳細は [`include-guard-guideline.md`](include-guard-guideline.md) を参照してください。

`__` を避けるのは、次の理由によります。

- C++ では `__` を含む識別子の定義が未定義動作になりうる
- 処理系や将来のコンパイラ拡張が同じ綴りをマクロや組込み識別子として使うと、衝突の診断が分かりにくい
- ライブラリ内共有の境界は `__` ではなく `_internal_` で表す (単一の `_` の並びであり、連続 `__` ではない)

処理系が提供する識別子やキーワード拡張の **参照** は禁止対象外です。  
`__FILE__`、`__LINE__`、`__func__`、`__declspec`、`__attribute__`、`_WIN32`、`__GNUC__` など、コンパイラや OS が定義するものを条件判定や属性に使うのは問題ありません。

例外は次の 2 つに限ります。

- OS / SDK / 外部 ABI が定義する型の alias で、元の型名を保存する必要があるもの。この場合は、例外である理由をヘッダーのコメントに残します
- 外部 OSS 由来のコード。改変しません

アンダースコア始まりは、次の 3 つの用途で使われがちです。  
それぞれの代替を以下のとおり定めます。

| 用途 | 規則 | 例 |
|---|---|---|
| マクロが名前を占有した関数の実体 | 呼び出し元情報を引数で受ける形とし、`_at` サフィックスを付ける | `sample_log_write_at(context, message, file, line)` |
| 既定インスタンス版と明示ハンドル版の対 | 明示ハンドル版を正名とし、既定インスタンス版に `_default_` を挟む | `sample_parser_parse(parser, ...)` と `sample_parser_default_parse(...)` |
| テスト専用フック | `_for_test` サフィックスのみで表し、前置きを付けない | `sample_shutdown_reset_for_test()` |

既定インスタンス版と明示ハンドル版の対で明示ハンドル版を正名とするのは、ハンドルを先頭引数に取る形が引数順序の規約に準拠した形であり、既定インスタンス版はそこからハンドルを暗黙化した派生形だからです。  
規約に準拠した形が装飾のない名前を持つようにします。

### 関数内ローカル変数

snake_case とし、接頭辞は付けません。  
ハンガリアン記法 (`sz`、`lp`、`dw` などの型を表す接頭辞) は使用しません。

関数の結果コードを受ける変数名は、新規コードでは `ret` を第一選択とします。

```c
/* 望ましい */
int ret = sample_file_get_size(&size, path);
if (ret != SAMPLE_OK)
{
    return ret;
}
```

`rc` や `rtc` など、結果コードを表す他の短い別名は、レビューで `ret` への統一を勧めてよいが、修正を必須としません。  
既存コードの `rc` 等を、本規則だけを目的に全面置換しません。

次の名前は、結果コードを受ける変数には採用しません。

| 名前 | 採用しない理由 |
|---|---|
| `result` | 計算結果、サイズ、ポインターなど本体の戻り値にも使う語で、結果コード専用として紛らわしい。世間の C では結果コード受けに `ret` / `rc` の方が多い |
| `err` | 成功 (`0` / `*_OK`) も含む変数を、エラー専用のように読ませる |
| `status` | Win32 や状態機械の status と混同しやすく、本リポジトリの結果コード体系 (`*_OK` / 負の分類) との対応が弱い |

結果コードを受ける変数に、呼び出しごとに異なる長い名前 (`open_result`、`file_get_size_ret` など) は付けません。  
同一関数内で結果コードを受ける変数の役割は「直前の API の結果コード」に定まることが多く、短い共通名の方が走査とレビューに向きます。  
本体の出力値は、対応する引数名から `_out` を除いた名前など、意味のある名前を使います。

同一関数内で複数の API の結果コードを順に受ける場合、同じ `ret` を代入し直して構いません。  
直前の呼び出し以外の結果コードを後で参照する必要があるときだけ、別変数を使います。

```c
/* 望ましい (同じ ret の使い回し) */
int ret;

ret = sample_open(path, &handle);
if (ret != SAMPLE_OK)
{
    return ret;
}

ret = sample_read(handle, buffer, size);
if (ret != SAMPLE_OK)
{
    sample_close(handle);
    return ret;
}
```

出力引数を受ける一時変数は、対応する引数名から `_out` を除いた名前とします。

ループ カウンターの `i`、`j`、`k` と、走査用の汎用ポインター `p` は、宣言と使用が同一の短い範囲に収まる場合に限り使用できます。

### static 関数

snake_case とし、**ライブラリ接頭辞も `<lib>_internal_` も付けません**。

ライブラリ接頭辞は外部リンケージを持つシンボルの目印です。  
`static` 関数に付けると外部から参照できるかのように読め、`nm` による公開シンボルの点検でも偽陽性を生みます。  
`<lib>_internal_` はライブラリ内共有 (外部リンケージ) の目印であり、ファイル内に閉じた `static` 関数には付けません。

```c
/* 望ましい */
static int parse_header(const unsigned char *buffer, size_t size);

/* 望ましくない (ライブラリ接頭辞が付いている) */
static int sample_parse_header(const unsigned char *buffer, size_t size);

/* 望ましくない (static なのに internal マーカーが付いている) */
static int sample_internal_parse_header(const unsigned char *buffer, size_t size);
```

同一ファイル内で機能のまとまりを表す接頭辞 (`argparser_`、`config_` など) は、ライブラリ接頭辞と異なるため使用できます。

### ファイル内共有変数

`s_` を前置きし、続きを snake_case とします。  
`const` を付けた読み取り専用のテーブルも同様です。

```c
/* 望ましい */
static size_t s_instance_count;
static const char s_hex_chars[] = "0123456789abcdef";

/* 望ましくない (全大文字はマクロと紛らわしい) */
static const char DESCRIPTOR_MAGIC[4] = { 'S', 'M', 'P', 'L' };

/* 望ましくない (static なのに外部リンケージを示唆する g_) */
static volatile sig_atomic_t g_stop_requested;
```

関数内で宣言する `static` 変数は、ファイル内共有変数ではないため `s_` を付けません。  
関数内ローカル変数の規則に従います。

### ライブラリ内共有関数

`<lib>_internal_` を必須とし、続きを snake_case とします。  
`internal` はライブラリ接頭辞の直後に置き、語の途中や末尾には置きません。

```c
/* include_internal/sample/trace/trace_common.h */
int sample_internal_trace_resolve_timestamp(sample_timespec *timestamp_out);

/* 望ましくない (ライブラリ接頭辞がない) */
int trace_resolve_timestamp(sample_timespec *timestamp_out);

/* 望ましくない (公開と同じ接頭辞で、internal マーカーがない) */
int sample_trace_resolve_timestamp(sample_timespec *timestamp_out);

/* 望ましくない (internal の位置が接頭辞の直後ではない) */
int sample_trace_internal_resolve_timestamp(sample_timespec *timestamp_out);
```

公開関数は `<lib>_`、ライブラリ内共有関数は `<lib>_internal_` とし、名前でも公開境界を表します。  
ヘッダー配置による境界と名前の境界を一致させ、`nm` やコード レビューで契約外シンボルを判別できるようにします。

昇格・降格では、宣言の移動に加えて接頭辞の付け替え (`sample_internal_foo` と `sample_foo` の相互変換) が必要です。  
呼び出し側もあわせて改名します。

### ライブラリ内共有変数

`g_<lib>_internal_` を前置きし、続きを snake_case とします。  
`g_` の直後にライブラリ接頭辞、その直後に `internal_` を置き、語の途中や末尾には置きません。

外部リンケージを持つ変数は、リンク時にライブラリ全体で 1 つの名前空間を共有します。  
接頭辞がないと、利用側のコードやほかのライブラリと衝突します。  
公開境界を `g_<lib>_internal_` で表すことで、公開共有変数の `g_<lib>_` とも区別し、シンボル衝突を避けやすくします。

```c
/* include_internal/sample/base/registry_internal.h */
extern sample_internal_registry g_sample_internal_default_registry;

/* 望ましくない (公開と同じ形で、internal マーカーがない) */
extern sample_internal_registry g_sample_default_registry;
```

### 公開関数

`<lib>_` を必須とし、続きを snake_case とします。  
`<lib>_internal_` は付けません。

カテゴリ名詞と動詞の並び順、生成と破棄の動詞の対応など、公開 API 名の詳細な構成規則は各 app の特化事項ドキュメントで定めます。  
本書は接頭辞と記法までを定めます。

### 公開共有変数

公開ヘッダーで `extern` する共有変数は、必要最低限に厳選します。

C では既定値テーブルなどの読み取り専用データ シンボルを公開せざるを得ない場面があるため、全面禁止とはしません。  
状態や設定を外に出す必要がある場合は、まずアクセサー関数の公開を検討します。

公開する場合は次を満たします。

- 名前は `g_<lib>_` を前置きし、続きを snake_case とする (`_internal_` は付けない)
- 読み取り専用を優先し、可能な限り `const` を付ける
- 変更可能なプロセス大域状態の公開は避ける

共有ライブラリの境界をまたぐデータ シンボルは、Windows で `__declspec(dllimport)` の扱いが関数と異なり、インポート ライブラリとの不整合を起こしやすいです。  
また、値の変更経路が追跡できず、スレッド安全性の保証も困難になります。  
これらのリスクを理由に、件数と役割を最小に保ちます。

```c
/* 公開が妥当な例 (読み取り専用の既定値) */
extern const sample_limits g_sample_default_limits;

/* 望ましくない (変更可能な状態の公開) */
extern sample_context *g_sample_current_context;
```

### 型の命名規則

型名は snake_case とします。  
公開ヘッダーで定義する型には `<lib>_` を前置きします。  
`include_internal/` のヘッダーで定義する型には `<lib>_internal_` を前置きします。  
`.c` の内部だけで使う型には接頭辞を付けません。

いずれの型でも `_t` サフィックスは付けません。

#### typedef struct

構造体タグ名と typedef 名を同一にします。

完全定義は次の形式で記述します。

```c
/* 公開型 */
typedef struct sample_context
{
    int value;
} sample_context;

/* ライブラリ内共有型 */
typedef struct sample_internal_registry
{
    size_t count;
} sample_internal_registry;
```

不透明型は次の形式で宣言します。

```c
typedef struct sample_context sample_context;
```

API の引数や戻り値でポインターを扱う場合は、型名に `*` を付けて明示します。  
ポインターを typedef 名に隠しません。

```c
sample_context *sample_context_create(void);
void sample_context_dispose(sample_context *context);
```

次のように、ポインターを typedef で隠す形式は禁止します。

```c
typedef struct sample_context *sample_handle;
```

匿名 struct typedef も使用しません。

```c
typedef struct
{
    int value;
} sample_context;
```

#### typedef enum

`typedef struct` と同じく、タグ名と typedef 名を同一にします。  
匿名 enum の typedef は使用しません。

匿名 enum を禁止するのは、前方宣言ができず、デバッガーの変数表示やコンパイラの診断メッセージに型名が現れないためです。

```c
/* 望ましい */
typedef enum sample_trace_level
{
    SAMPLE_TRACE_LEVEL_INFO = 0,
    SAMPLE_TRACE_LEVEL_WARNING = 1,
} sample_trace_level;

/* 望ましくない (匿名かつ _t サフィックス) */
typedef enum
{
    SAMPLE_TRACE_LEVEL_INFO,
} sample_trace_level_t;
```

列挙定数は全大文字とし、`<LIB>_` を前置きします。  
値が ABI として固定される enum では、全列挙定数に明示的な値を書き、新しい定数は末尾へ追加します。

#### 関数ポインター typedef

サフィックスは `_fn` に統一します。  
`_callback` / `_func` / `_handler` およびそれらに `_t` を付けた形は使用しません。

```c
/* 望ましい (公開) */
typedef void (*sample_hook_fn)(sample_context *context, void *user_data);

/* 望ましい (ライブラリ内共有) */
typedef void (*sample_internal_sink_write_fn)(const void *data, size_t size);

/* 望ましくない */
typedef void (*sample_hook_callback_t)(sample_context *context, void *user_data);
```

`_fn` の直前に `callback`、`func`、`handler` を重ねません。  
`sample_hook_callback_fn` は冗長です。

#### typedef union

`typedef struct` と同じ規則に従います。  
タグ名と typedef 名を同一にし、匿名 union の typedef は使用しません。

#### 型命名の対象外

次の型は本節の対象外です。

- 固定幅整数型と標準ライブラリ型 (`uint32_t`、`size_t` など)
- OS / SDK / 外部 ABI が定義する型の alias
- 外部 OSS 由来のコードが定義する型

### マクロの命名

全大文字とアンダースコアで構成し、`<LIB>_` を前置きします。

```c
/* 望ましい */
#define SAMPLE_PATH_MAX 260

/* 望ましくない (ライブラリ接頭辞がなく、他ライブラリと衝突しうる) */
#define PATH_MAX_LEN 260
```

公開ヘッダーで定義するマクロには、必ずライブラリ接頭辞を付けます。  
`include_internal/` で定義するマクロも `<LIB>_` を前置きします。マクロ名に `_INTERNAL_` を必須とはしません。  
`.c` の内部だけで使うマクロは接頭辞を省略できます。

例外として、関数と同名で呼び出し元情報を注入するマクロは、関数の見た目を保つため小文字で定義します。  
この場合は、注入先となる実体の関数名を「予約識別子の回避」の `_at` 規則に従って命名します。

```c
#define sample_log_write(context, message) \
    sample_log_write_at((context), (message), __FILE__, __LINE__)
```

### ヘッダー ファイル名の _internal

ヘッダー **ファイル名** の `_internal` サフィックスは、次の条件のときだけ付けます。

- 同名の公開ヘッダーが `prod/include/` に存在する (例: 公開 `console.h` に対する `console_internal.h`)
- 対応する公開ヘッダーが存在しない場合はサフィックスなし (例: `path_format.h`、`trace_common.h`)

この規則はシンボル名の `<lib>_internal_` とは付与条件が異なります。

| 対象 | 付与条件 | 例 |
|---|---|---|
| ヘッダー ファイル名 | 同名の公開ヘッダーがあるときだけ | `console_internal.h` / `path_format.h` |
| ライブラリ内共有の関数・型 | `include_internal/` で宣言するすべて | `sample_internal_console_flush` / `sample_internal_registry` |
| ライブラリ内共有の外部リンケージ変数 | `include_internal/` で `extern` するすべて | `g_sample_internal_default_registry` |

ファイル名に `_internal` が無くても、`include_internal/` 配下で宣言する関数・型・外部リンケージ変数には公開境界のマーカーを付けます。

### 既存コードへの適用と例外条件

- 新規コードでは、最初から本章の規則に従います。
- 既存ファイルを変更する際は、変更ファイル内の同種のシンボルすべてに本章を適用します。
- ライブラリ内共有の関数・型が `<lib>_` のまま、または共有変数が `g_<lib>_` のままになっている既存コードは、本規則を目的とした全面改名は求めません。変更対象ファイルに触れる機会に合わせて `<lib>_internal_` / `g_<lib>_internal_` へ移行します。
- 大規模な改名リファクタリングを行う際は、**1 commit = 1 カテゴリ (または 1 ヘッダー)** 単位で進めます。改名以外の変更 (戻り値の意味変更、引数の追加、ロジックの修正) を同じ commit に含めません。
- ABI 凍結を宣言しているシンボルについては、各 app のドキュメントの凍結リストが本章に優先します。凍結の範囲と根拠は当該ドキュメントに明記します。

改名を伴う機械的な置換では、次の手順で安全性を確認します。

1. 置換前の旧名の出現件数を記録する
2. 新名が未使用であること (衝突がないこと) を確認する
3. 識別子境界を指定して置換する。行番号を指定した置換は行わない
4. 置換後に旧名が 0 件であること、新名の件数が 1 で記録した件数と一致することを確認する
5. 識別子以外のトークンが変化していないことを差分で確認する

### 検証

```bash
# 外部 OSS とビルド生成物を除外する共通条件
EXCL='app/(lua|sqlite|cjson)/|/obj/|doxybook2_|/prod/(lib|cbin)/'

# _t サフィックスの検出 (例外として明記した alias のみが残る)
grep -rnE '(^|[[:space:]])typedef[[:space:]].*[A-Za-z0-9_]+_t[[:space:]]*(\)|;)|^\}[[:space:]]*[a-z0-9_]+_t[[:space:]]*;' \
  app --include=*.h --include=*.c | grep -vE "$EXCL"

# アンダースコア始まりのファイル スコープ識別子 (0 件であること)
grep -rnE '(^|[^A-Za-z0-9_"])_[a-z][A-Za-z0-9_]*[[:space:]]*\(' \
  app --include=*.h --include=*.c | grep -vE "$EXCL"

# ユーザー定義らしき識別子中の連続 __ (処理系マクロの参照は目視で除外。レビュー併用)
grep -rnE '\b[A-Za-z0-9_]*__[A-Za-z0-9_]*\b' app --include=*.h --include=*.c --include=*.cc --include=*.cpp \
  | grep -vE "$EXCL" | grep -vE '__FILE__|__LINE__|__func__|__FUNCTION__|__PRETTY_FUNCTION__|__declspec|__attribute__|__stdcall|__cdecl|__fastcall|__restrict|__inline|__forceinline|__asm|__volatile|__GNUC__|__clang__|__cplusplus|__x86_64__|__i386__|__linux__|__APPLE__|__WIN32|__WIN64|_WIN32|_MSC_VER|__COUNTER__|__DATE__|__TIME__|__TIMESTAMP__'

# 匿名 typedef (0 件であること)
grep -rnE 'typedef[[:space:]]+(enum|struct|union)[[:space:]]*(\{|$)' \
  app --include=*.h --include=*.c | grep -vE "$EXCL"

# 関数ポインター typedef のサフィックス (0 件であること)
grep -rnE 'typedef[[:space:]].*\(\*[A-Za-z_][A-Za-z0-9_]*\)' \
  app --include=*.h --include=*.c | grep -vE "$EXCL" | grep -vE '\*[a-z0-9_]+_fn\)'

# static 関数のライブラリ接頭辞 (0 件であること。<lib> は対象ライブラリの接頭辞)
grep -rnE '^static[[:space:]][^(;]*\b<lib>_[a-z0-9_]*[[:space:]]*\(' \
  app --include=*.c | grep -vE "$EXCL"

# static 関数への internal マーカー (0 件であること。<lib> は対象ライブラリの接頭辞)
grep -rnE '^static[[:space:]][^(;]*\b<lib>_internal_[a-z0-9_]*[[:space:]]*\(' \
  app --include=*.c | grep -vE "$EXCL"

# 公開ヘッダーに internal マーカー付き識別子が出ていないこと (0 件であること。<lib> は対象ライブラリの接頭辞)
grep -rnE '\b<lib>_internal_' app/*/prod/include --include=*.h | grep -vE "$EXCL"

# 公開ヘッダーの extern 変数 (件数は最小であること。新規追加時は必要性をレビューする)
grep -rnE '^[[:space:]]*extern[[:space:]]' app/*/prod/include --include=*.h \
  | grep -vE "$EXCL" | grep -vE '\('
```

`include_internal/` の関数・型・変数がすべて公開境界マーカー付きであることの機械検証は、公開型を引数に取る内部関数や、ファイル スコープ以外の識別子を偽陽性として拾いやすいため、目視とコード レビューを併用してください。

`static` 変数の `s_` 接頭辞は、関数定義を偽陽性として拾うため、機械的な検出だけでは判定できません。  
`^static` で始まる行を抽出し、宣言子が関数ではないものを目視で選別してください。

## 構造体パディングの扱い

### 基本ルール

`-Wpadded` が指摘する暗黙パディングは、`#pragma GCC diagnostic ignored "-Wpadded"` で抑止せず、構造体定義を見直して解消します。

ただし、**大きいアラインメント順への積極的な並び替えよりも、メンバーの意味上のまとまりと可読性を優先** します。  
意味の近いメンバーを保ったままでは暗黙パディングを避けられない場合は、明示的なパディング メンバーを追加してください。

### 明示的なパディング メンバー

明示的なパディングを追加するときは、次のルールに従います。

- メンバー名は `pad`、複数必要な場合は `pad1`、`pad2`、... とする
- コメントで明示的アラインメントのためのパディングであることを簡潔に明示する
- 幅は不足分だけに留める

```c
typedef struct sample_record
{
    int mode;
    unsigned int pad; /* 明示的アラインメント */
    intptr_t native_handle;
} sample_record;
```

### プラットフォーム依存の条件付きパディング

サポート対象外の 32 ビット環境だけを考慮した条件分岐は追加しません。  
パディングはサポート対象の ABI に基づいて定義します。  
複数アーキテクチャーを正式にサポートする構造体に限り、`ARCH_*` による条件付きパディングを使用します。

プラットフォームやアーキテクチャーごとにパディング有無を切り替える場合は、`#if defined(ARCH_X64)` や `#if defined(PLATFORM_WINDOWS)` のように、プラットフォーム抽象化ヘッダーの共通マクロを使います。  
`__x86_64__` や `_WIN32` を利用側で直接判定しません。

共通マクロの利用規則は、利用するライブラリのプラットフォーム抽象化ガイドラインを参照してください。

## 整数型の選択

### 基本ルール

C / C++ コードで整数値を表す型は、次の方針で選択します。

- 8bit 幅の整数値は `signed char` / `unsigned char` を用います。
- 16bit 幅の整数値は `short` / `unsigned short` を用います。
- 32bit 幅の整数値は `int` / `unsigned int` を用います。
- 64bit 幅の整数値は `int64_t` / `uint64_t` を用います。

`int8_t` / `uint8_t` / `int16_t` / `uint16_t` / `int32_t` / `uint32_t` は使用しません。  
`char` は処理系で符号付き / 符号なしが分かれるため、整数値として扱う場合は `signed char` / `unsigned char` を明示してください (文字列の要素として扱う場合は `char` を用います)。  
LP64 の Linux x86_64 では `long` が 64bit ですが、LLP64 の Windows x64 では `long` が 32bit です。  
したがって、`long` をクロスプラットフォームの 64 bit 整数型として使用しません。

> 現代的な Linux (GCC)・Windows (MSVC) 環境では、`signed char` / `unsigned char` が 8bit、`short` / `unsigned short` が 16bit、`int` / `unsigned int` が 32bit となります。
> LP64 (Linux x86_64 など) でも int は 32bit、long が 64bit です。
> LLP64 (Windows x64) でも int は 32bit、long は 32bit、long long が 64bit です。

### 値の意味に対応する型

値に対応する標準型、OS API 型、または対象ワークスペースの共通型がある場合は、ビット幅だけで型を決めず、値の意味に対応する型を優先します。  
`size_t`、`ptrdiff_t`、`ssize_t`、`off_t`、`time_t` は、特定の意味と演算規則を持つ型であり、単なる 32 bit または 64 bit の整数型として使用しません。

| 値の意味 | 選択する型 | 使用条件 |
|---|---|---|
| オブジェクト ポインターとの往復変換、アドレス値 | `uintptr_t` | ポインター幅へ追従させる。 |
| 負の無効値を持つ OS ネイティブ ハンドル | `intptr_t` | OS API がポインター幅の整数と負のセンチネル値を要求する場合に限定する |
| オブジェクトのバイト サイズ、要素数、`sizeof` の結果、それらと同じ範囲の配列添字 | `size_t` | オブジェクトに関する、負値を取らない値に使用する |
| 同一配列内の 2 つのポインターの差 | `ptrdiff_t` | 異なる配列を指すポインター同士の減算には使用しない |
| POSIX I/O API が返す処理済みバイト数とエラー値 | `ssize_t` | Linux 実装内の POSIX API 境界に限定し、Windows 側に露出させない |
| POSIX のファイル位置、ファイル サイズ、ファイル オフセット | `off_t` | Linux 実装内の POSIX API 境界に限定、Windows 側に露出させない |
| C / POSIX 時刻 API が扱う時刻の秒部 | `time_t` | `time()` の戻り値や `struct timespec::tv_sec` などに使用し、任意の期間には使用しない |
| クロスプラットフォームで受け渡す絶対時刻、単調時刻 | 各 app が定める標準時刻型 | 標準時刻型の定めがある場合はそれに従う (各 app の特化事項を参照) |
| 正規化されたナノ秒部、負になり得る時間差 | `int64_t` | 標準時刻型のナノ秒部または符号付きの差分値に使用する |
| 検査済みの非負ナノ秒期間、タイムアウト | `uint64_t` | 負値を受け付けないことを API 仕様で定め、外部入力の負値検査が完了した後に使用する |
| クロスプラットフォーム API のファイル位置、ファイル オフセット、I/O 結果 | `int64_t` | Linux の `off_t` / `ssize_t` と Windows の 64 bit API を共通化する場合に使用する |
| 文字列から入力し、負値や範囲外を検査するファイル オフセット | `int64_t` | 符号付き整数として解析し、構文、負値、上限を検査してから目的の型へ変換する |

MSVC の UCRT では `off_t` も `long` の別名であるため 32bit となります。64 bit のファイル位置を扱う共通 API には `int64_t` を使用し、Windows 実装では `_fseeki64` / `_ftelli64` / `_lseeki64` との境界で変換し、`ott_t` を用いません。

`ssize_t`、`off_t`、`time_t` の幅は処理系に依存します。  
これらの型をファイル形式、通信形式、共有メモリなど、バイナリ レイアウトを固定するデータには使用しません。

### 文字列入力から意味付き型への変換

コマンド ライン引数などの外部文字列を `size_t` やファイル位置へ変換する場合は、最初に `int64_t` として解析します。  
解析時には、文字列全体が整数として解釈されたこと、変換元の値が `int64_t` の範囲内であること、用途上の下限と上限を満たすことを確認します。

負値を許容しない値は、負値の検査後に変換先の最大値を確認してから `size_t` または `uint64_t` へ変換します。  
符号付き型と符号なし型を直接比較すると暗黙変換によって負値が大きな正値になるため、負値の検査より前に `SIZE_MAX` などと比較しません。  
クロスプラットフォーム API のファイル位置へ渡す値は、検査後も `int64_t` のまま保持し、Linux 実装内でのみ `off_t` へ変換します。

### 例外として固定幅型を維持する用途

次の用途では、固定幅型 (`uint8_t` / `int8_t` / `uint16_t` / `int16_t` / `uint32_t` / `int32_t`) を例外として維持します。

- バイト列入出力 (`uint8_t *` バッファー、ヘッダーや任意長データのポインター)
- ネットワーク バイト順序の値 (`htons` / `htonl` / `ntohs` / `ntohl` の周辺、固定長の通信フィールド)
- ワイヤ プロトコル / 通信パケットの構造体メンバー (`payload_len`、`flags`、`session_id`、`seq_num` 等の幅が仕様で決まっているフィールド)
- アルゴリズム規格上、幅が定義されている計算値 (CRC、暗号鍵長など)
- OS API 境界で固定幅が要請される箇所
    - Windows `DWORD` を経由する API (`Sleep`、`WriteFile`、`GetCurrentProcessId` 等)
    - POSIX `struct timespec::tv_nsec` (long) との境界キャスト
    - atomic 操作の state 変数 (例: `__atomic_compare_exchange_n` の引数)

例外を採用する場合は、当該ヘッダー / コードに「なぜ固定幅を維持するか」を簡潔に記載してください。

```c
/* Windows Sleep の DWORD 引数に渡すため uint32_t を維持する */
uint32_t timeout_dword = (uint32_t)timeout_ms;
Sleep(timeout_dword);
```

## 関数引数の異常入力対応

### 基本ルール

関数引数のうち「概念的には正の値のみを想定する」整数値も、型として `int` を採用します。  
これは、呼び出し側で計算結果として負値が混入したことを検出可能にするためです。

ただし、値の意味に対応する型が「[値の意味に対応する型](#値の意味に対応する型)」で定められている場合、または外部 API が型を指定している場合は、その型を優先します。  
負値を検査する必要がある外部文字列は、符号なしの目的型へ直接変換せず、`int64_t` として解析してから検査します。

負値が渡された場合の挙動は、仕様として明示します。

- 戻り値で結果を返せる関数は、引数不正を表すエラー コードを返します ([エラー処理と戻り値規約](#エラー処理と戻り値規約) を参照)。
- 戻り値を持たない関数 (例: sleep 系) は、無処理 (no-op) で戻ります。

仕様は Doxygen コメントに必ず明記します。

### 例

```c
/**
 *  @brief          指定時間ロックを待機します。
 *  @param[in]      timeout_ms タイムアウト (ms)。負値は @ref SAMPLE_ERR_INVALID_ARGUMENT を返します。
 *  @return         結果コード。
 */
int sample_lock_lock(sample_lock *mtx, int timeout_ms)
{
    if (timeout_ms < 0)
    {
        return SAMPLE_ERR_INVALID_ARGUMENT;
    }
    /* ... */
}

/**
 *  @brief          指定ミリ秒だけ呼び出しスレッドを停止します。
 *  @param[in]      ms 停止時間 (ms)。0 以下は無処理で戻ります。
 */
void sample_sleep_ms(int ms)
{
    if (ms <= 0)
    {
        return;
    }
    /* ... */
}
```

## エラー処理と戻り値規約

### 基本ルール

結果コードを戻り値で返す関数は、ライブラリ (app) ごとに 1 つの共通結果コード群へ統一します。  
共通結果コードは、ライブラリの接頭辞を付けた `#define` として専用ヘッダーに集約します。  
以下、接頭辞を `SAMPLE` とした例で示します。

- 成功は `SAMPLE_OK` (0) のみとします。非 0 はすべて「要求した操作が完遂されなかった」ことを表します。
- エラーは負値とします。`SAMPLE_ERR` (-1) は、分類済みコードに該当しないその他のエラーを表します。
- 分類済みのエラーは -2 以降の負値コード (`SAMPLE_ERR_INVALID_ARGUMENT` など) を使用します。
- カテゴリ (モジュール) ごとの結果コード型 (enum) やカテゴリ固有の成功/失敗定数は新設しません。戻り値の型は `int` とします。
- 成功を 1、失敗を 0 とする逆向きの規約、および 1/0/-1 のような三値の規約は使用しません。

各コードの値は ABI として凍結します。既存の値の変更は禁止し、コードの追加は末尾 (より小さい負値) への追記のみとします。

### 判定慣用句

呼び出し側の成否判定は、コード名との比較を正とします。

```c
int ret = sample_resource_attach(path, &handle);
if (ret != SAMPLE_OK)
{
    return ret;
}
```

全エラーが負値のため `ret < 0` も等価ですが、名前比較を推奨します。  
特定のエラーを区別する場合は、`ret == SAMPLE_ERR_TIMEOUT` のようにコード名で比較します。  
`-1` などの数値リテラルとの比較は行いません。

### 戻り値とエラー詳細の役割分担

戻り値は「分類済みの結果コード」を伝達し、OS 由来の詳細は出力引数で伝達します。

| 伝達手段 | 内容 |
|---|---|
| 戻り値 (`int`) | `SAMPLE_OK` または負値の分類済みエラー コード |
| `int *errno_out` などの出力引数 | 生の詳細値。Linux では `errno`、Windows では `GetLastError()` の値 |

分類済みコードでは失われる詳細 (`ENOENT` と `EACCES` の区別など) が必要な API のみ、`errno_out` を提供します。  
`errno`、`GetLastError()`、`HRESULT` などの OS エラー値を、共通結果コードとして直接返しません。

### 真偽値や状態を返す API の設計

「一致したか」「実行したか」などの真偽の答えと、操作の成否は分離します。

- 真偽の答えは `int *xxx_out` の出力引数で返し、戻り値は結果コードとします。
- 出力引数の値は、戻り値が `SAMPLE_OK` の場合のみ有効とします。

```c
/* 望ましい: 成否と真偽値を分離 */
int sample_paths_equal(const char *lhs, const char *rhs, int *equal_out, int *errno_out);

/* 望ましくない: 1 (一致) / 0 (不一致) / -1 (失敗) の三値 */
int sample_paths_equal(const char *lhs, const char *rhs, int *errno_out);
```

### 詳細コードの扱い

解析エラーの種別など、共通結果コードより細かい粒度の分類が必要な場合は、app 単位の単一コード集合へコードを追加することを原則とします。  
1 系統に集約することで、符号の規約が揃い、粗い分類と細かい分類を同じ判定慣用句で扱えます。

モジュール固有のコード体系を別に設けることは例外とし、採用する場合は共通結果コードと値が重複しないこと、および取得用 API と出力引数のどちらで伝達するかを、そのライブラリの特化事項として明記します。

ただし、レガシ マイグレーション案件や製品リリース後の改修など、既存の戻り値とその意味を維持する ABI 契約を重視する場合は、本節の原則よりも各 app のポリシーを優先します。  
既存の呼び出し元やバイナリ互換の制約から、コード集合の統合や値の再割り当てが行えない場合があるためです。  
この場合は、原則から外れる範囲と維持すべき戻り値規約を、そのライブラリの特化事項として明記します。

### 適用対象外

以下の API 群は、共通結果コードの適用対象外です。

| 対象外の API 群 | 理由 |
|---|---|
| 標準 C / OS API の互換ラッパー (`FILE *`/NULL、0/EOF、fd/-1、`BOOL` などを元 API と同じ規約で返す層) | 元 API の戻り値規約を保存し、元 API の感覚・差し替えとして使えること自体が設計意図 |
| ハンドル生成系 (`*_create` など) | 成功時ポインター / 失敗時 NULL というポインター返却 API の慣用に従う |
| 値をそのまま返す関数 (getter、比較関数など) | 結果コードの概念が適用されない |
| 戻り値を持たない関数 (`*_destroy` など) | 同上 |

互換ラッパー層を対象外とする場合は、対象外とする範囲と根拠を当該ライブラリのドキュメントに明記します。  
対象外の API を新設する場合は、元 API との対応と戻り値規約をヘッダーの Doxygen コメントに明記します。

### 各ライブラリの特化事項

本節は一般的な方針を定めるものです。  
各 app のドキュメントに優先事項、特化事項 (共通結果コードのヘッダー位置、コード一覧、適用対象外の具体的な範囲など) がある場合は、それに従ってください。

### 検証

```bash
# 数値リテラル比較や三値規約の残存確認 (対象モジュール配下)
grep -nE '(==|!=)[[:space:]]*-1\b' <dir>/*.c

# 局所テスト
cd <module-dir> && make test
```

## 文字列コピー関数の選択

`app/` 配下の管理対象コードでは、標準 C ライブラリの `strncpy` を直接使用しません。  
外部 OSS 由来のコードは本規則の対象外です。

`strncpy` は、コピー元の長さが `count` 以上の場合にコピー先を null 終端しません。  
また、`count` はコピー先バッファーの容量ではなく最大コピー文字数を表すため、コピー先サイズを渡す API と誤認しやすいインターフェースです。  
コピー元が `count` より短い場合は残りの領域を null 文字で埋めるため、文字列のコピーだけを目的とする処理では不要な書き込みも発生します。

null 終端文字列全体をコピーする場合は、コピー先の容量を受け取り、バッファー不足を戻り値で通知する API を使用します。  
展開先が共通の文字列コピー API を定めている場合は、その API を使用して戻り値を確認してください。  
意図的に文字列を切り詰める場合は、コピー後の null 終端を保証し、切り詰める条件を呼び出し側の仕様として明記してください。

## 変数宣言位置と命令文の関係

### 基本ルール

関数内の変数宣言は C17 スタイルを採用し、ブロック途中の宣言を許容します。  
変数宣言はスコープを必要最小限にし、誤認を防ぐため利用箇所に近い位置へ置きます。

一方で、可読性のため次の配置を推奨します。

- 同一ブロック内で早い段階に使う変数は、ブロック先頭付近に集めて宣言する
- 命令文の後でしか初期値が確定しない変数は、その直後に宣言する

### for ループ変数

ループ変数を `for` の初期化式で宣言する記法を許容します。  
`for (int i = 0; ... )` のように記述し、ループ変数の有効範囲をループ内へ限定してください。

### 例

```c
int process_items(const item_t *items, int count)
{
    int total = 0;

    if ((items == NULL) || (count <= 0))
    {
        return -1;
    }

    for (int i = 0; i < count; ++i)
    {
        total += items[i].value;
    }

    return total;
}
```

```c
int load_and_apply(config_handle_t *handle, const char *path)
{
    int ret = read_config_file(path);
    if (ret != 0)
    {
        return ret;
    }

    /* read_config_file の結果を見てから宣言したほうが意図を読み取りやすい */
    config_t cfg = {0};
    ret = parse_config(path, &cfg);
    if (ret != 0)
    {
        return ret;
    }

    return apply_config(handle, &cfg);
}
```

```c
/* 非推奨例: 宣言が後半へ散在し、変数の役割を追いにくい */
int calculate_total(const int *values, int count)
{
    if ((values == NULL) || (count <= 0))
    {
        return -1;
    }

    int total = 0;
    for (int i = 0; i < count; ++i)
    {
        total += values[i];
    }

    /* 利用箇所が離れており読み手に負荷をかける */
    int average = total / count;
    return average * count;
}
```

## 式の括弧

### 比較演算子と算術式

比較演算子 (`<`, `<=`, `>`, `>=`, `==`, `!=`) の左右に算術式やビット演算式を置く場合は、比較対象のまとまりが分かるように括弧で囲みます。

```c
if (pos > (MAX_BODY - ELLIPSIS_LEN))
{
    return -1;
}
```

次のように、比較演算子と算術演算子の関係が読み手に委ねられる書き方は避けます。

```c
if (pos > MAX_BODY - ELLIPSIS_LEN)
{
    return -1;
}
```

## 関数引数の const 付与と Doxygen 方向タグ

対象ワークスペースのすべての C コード (`app/` 配下) で、関数引数には次のルールで `const` を付与し、  
Doxygen の `@param[in/out/in,out]` を厳密化します。新規関数だけでなく、既存関数の変更時にも適用します。

### 引数の分類

各引数は **意味的方向 (Doxygen タグ)** と **物理的種別 (型)** の 2 軸で分類します。

| 意味的方向 | 説明 |
| --- | --- |
| `[in]` | 関数は値を読むだけで、関数復帰後の中身は呼び出し元視点で変わらない |
| `[out]` | 関数復帰後に初めて意味のある値が書き込まれる (初期化前の値は見ない) |
| `[in,out]` | 関数復帰前後の両方で意味があり、関数が書き換えうる |

| 物理的種別 | 例 |
| --- | --- |
| ポインター引数 | `T *p`, `const T *p`, `T **pp`, 配列 `T arr[]` |
| 値渡し引数 (リテラル) | `int n`, `size_t len`, `enum E e` |

**opaque handle (`*_t *handle` 等) / 同期プリミティブ (mutex, rwlock, condvar, thread, lock, once_flag) / `FILE *` は、関数本体で内部状態を変更しても常に `[in]` とします。**

呼び出し元はそのメンバーを直接読み戻さず、再度 API に渡すだけです。`_dispose`/`_close`/`_stop` 系も例外なく `[in]` です。

`[in,out]` を使うのは、呼び出し元が関数復帰後に同じポインター経由でデータを読み戻す場合に限ります。

| `[in,out]` を使う例 | 理由 |
| --- | --- |
| `strcat` の `dest` | 内容を append、呼び出し元が後で文字列として読む |
| `path_normalize` の `path` | in-place 編集、呼び出し元が後で読む |
| `compress`/`crypto` の `dst_len` | 入力: バッファー サイズ / 出力: 必要サイズ |
| `file_open` の `file` | 既存ハンドル状態を読んで再オープン判定 + 新規オープン結果を書き込む |

物理的な mutex 取得や内部状態変更は `@par スレッド セーフ` で言及し、`[in/out]` には反映しません。

### ポインター引数の const 付与判定

**手順** (ヘッダー宣言と impl 定義の両方に const を付ける):

1. 引数の意味的方向を決定します。`[out]` または `[in,out]` ならば const は付けません。
2. `[in]` の場合、impl (Linux/Windows 両方) を読み、次のいずれかに該当するか確認します。

   | 項目 | 該当時 |
   | --- | --- |
   | `*p = ...` または `p->member = ...` の代入がある | const **不可** |
   | `p` または `&p->member` を書き換え系 OS API に渡す (`fstat` の出力先など) | const **不可** |
   | `p` を内部 mutex / rwlock / condvar の取得対象として渡す (`pthread_mutex_lock`、`EnterCriticalSection`、利用するライブラリの同期プリミティブ API 等) | const **不可** (mutable 扱い) |
   | 上記いずれにも該当しない | const **可** |

3. const 可と判定したら、**ヘッダー宣言と impl 定義の両方** に `const T *` を付けます。

宣言と定義での修飾子・マクロの配置一覧は [宣言と定義の関係](#宣言と定義の関係) を参照します。

**例外**:

- `FILE *` 引数には慣習として `const` を付けません (C 標準 stdio に合わせる)。  
  Doxygen タグは上記の opaque handle ルールと同様に常に `[in]` とします。
- 不透明 handle で内部 mutex/rwlock を取得する関数は、論理的には read-only でも const を付けません。`-Wcast-qual` と整合させるためです。

**機械的フィルターの grep 例** (`<arg>` は引数名、`<dir>` は対象ディレクトリ):

```bash
# ポインター引数の書き換え検査
grep -nE '(\*[[:space:]]*<arg>[[:space:]]*=|<arg>->[a-z_]+[[:space:]]*=|\&<arg>->[a-z_]+)' <dir>/*.c

# 内部 lock 取得検査 (利用するライブラリの同期プリミティブ API があればパターンに追加する)
grep -nE '(pthread_mutex_lock|EnterCriticalSection)' <dir>/*.c
```

grep は補助です。最終判定は手読みで行います。

### 値渡し引数 (リテラル) の const 付与判定

値渡し引数は **impl 側 (定義) のみ** に `const` を付けます。ヘッダー宣言には付けません。

```c
/* ヘッダー (宣言) */
int foo(int n);

/* impl (定義) */
int foo(const int n)
{
    return n * 2;
}
```

C 言語では top-level の値渡し const は ABI に影響せず、宣言と定義の互換性に影響しません。  
慣習に従い宣言側はシンプルに保ち、impl 内の不慮書き換え防止のためにのみ定義側に const を付けます。  
これにより mock や呼び出し側コードは影響を受けません。

宣言と定義での修飾子・マクロの配置一覧は [宣言と定義の関係](#宣言と定義の関係) を参照します。

**手順**:

1. 引数の意味的方向を決定します。値渡しで `[in,out]` 相当が必要ならポインター化を検討します (ABI 変更を伴うため別 commit 推奨)。
2. 関数本体で引数を読むだけなら、impl 側に `const` を付けます。
3. 再代入 (`n = ...`、`n++`、`n--`、`n += ...` 等) があれば、§4 に従って impl を整理してから const を付けます。

**機械的フィルターの grep 例**:

```bash
grep -nE '\b<arg>[[:space:]]*(\+\+|--|=|\+=|-=|\*=|/=|%=)' <dir>/*.c
```

### [in] 引数は関数内部で更新しないことを原則とする

意味的に `[in]` の引数は関数内部で更新しません。再代入やループ カウンターとして使い回しているコードは  
可読性の問題でもあるため、const 化対応の一環として impl を整理します。

```c
/* 望ましくない: in 引数 n を使い回し */
int foo(int n)
{
    while (n > 0)
    {
        process(n);
        n--;
    }
    return 0;
}

/* 望ましい: ローカル変数で作業し、引数は const */
int foo(const int n)
{
    int remaining = n;

    while (remaining > 0)
    {
        process(remaining);
        remaining--;
    }

    return 0;
}
```

「真に書き換えが必要」と判定された場合は Doxygen を `[in,out]` に修正します。  
値渡しで `[in,out]` の場合はポインター化を検討します (ABI 変更を伴うため別 commit にします)。

### Doxygen @param[in/out/in,out] の厳密化

const 付与とセットで、Doxygen タグも impl の挙動に合わせて見直します。

**典型的な誤りパターン**:

| 実際の挙動 | 誤り | 正しいタグ |
| --- | --- | --- |
| 出力バッファー (初期化前の値を見ない) | `[in]` が付いている | `[out]` |
| in-place データ編集 (呼び出し元が復帰後に読む) | `[in]` のみ | `[in,out]` |
| opaque handle への操作 (`_dispose`/`_write`/`_set_*` 等) | `[in,out]` が付いている | `[in]` |

### 適用範囲と作業の進め方

- 新規関数では本ルールに従って最初から const と Doxygen タグを正しく付けます。
- 既存関数を変更する際は、変更ファイル内の関数全てに本ルールを適用します。
- 大規模な const 化リファクタリングを行う際は、**1 commit = 1 ヘッダー (カテゴリ)** 単位で進めます。  
  ヘッダー変更・impl 変更・対応する mock 追従・Doxygen タグ修正を同 commit にまとめると、  
  `-Wcast-qual` 警告の発生箇所が局所化されてレビューが容易になります。

### 検証

各 commit / PR で次を確認します。

```bash
# Linux ビルド (-Wcast-qual で警告 0 を確認)
cd <module-dir> && make 2>&1 | grep -E 'warning|error'

# 局所テスト
cd <module-dir> && make test

# Doxygen 警告チェック (該当ターゲットがあれば)
cd <module-dir> && make doxy 2>&1 | grep -i warning
```

全体リファクタリング完了後は、リポジトリ ルートでも `make` / `make test` を実行し、  
他モジュールへの影響がないことを確認します (非 const → const 化は呼び出し側で暗黙変換可なため、通常は影響なし)。

### 既存の模範例

本ルールに沿っている既存ヘッダーの一覧は、各 app のドキュメントに模範例として記載します。  
新規実装時の参考には、対象 app の特化事項ドキュメントを参照してください。

### mock 追従 (test 配下を持つモジュールの場合)

ヘッダーの const 化 / Doxygen 変更を行う commit には、対応する mock のシグネチャ追従を必ず含めます。

`delegate_real_*` 宣言、`MOCK_METHOD(...)` 宣言、および `MOCK_WEAK_IMPL(...)` の引数型を  
ヘッダー宣言と完全一致させます。  
`ON_CALL(...).WillByDefault(Invoke(delegate_real_*))` および `EXPECT_CALL(...)` の matcher は  
型推論で追従するため、通常は無修正で OK です。  
ただし `Matcher<T*>` のように明示型指定している箇所がある場合は併せて修正します。

## 宣言と定義の関係

公開 API の関数では、修飾子とマクロを **宣言 (ヘッダー)** と **定義 (impl の `.c`)** のどちらに置くかを次のとおり統一します。  
宣言を契約の単一の源とし、定義側の重複を排します。

| 対象 | 宣言 (ヘッダー) | 定義 (.c) |
| --- | --- | --- |
| エクスポート / 呼び出し規約マクロ (`EXAMPLE_EXPORT` / `EXAMPLE_API` 等) | 付ける | 付けない |
| 値渡し引数の top-level const (`const int n`) | 付けない | `[in]` に付ける |
| ポインター参照先の const (`const T *p`) | 付ける | 付ける |

`example` の例を示します。

宣言 (ヘッダー):

```c
EXAMPLE_EXPORT extern int EXAMPLE_API exampleHandler(int kind, int a, int b, int *result);
```

定義 (`.c`):

```c
/* Doxygen コメントは、ヘッダーに記載 */

int exampleHandler(const int kind, const int a, const int b, int *result)
{
    /* ... */
}
```

### エクスポート / 呼び出し規約マクロを定義側に付けない理由

- MSVC は先行する宣言から `__declspec(dllexport)` と `__stdcall` を継承します。
- `.c` は対応する公開ヘッダーを include 済みです (例: `exampleHandler.c` は `<example/example_spec.h>` を include)。  
  このため定義側にマクロを重ねても情報が重複するだけで、新たな意味を持ちません。
- 重複を排し、宣言を唯一の契約源とすることで保守性を上げます。

値渡し const とポインター const の配置理由は、それぞれ [値渡し引数 (リテラル) の const 付与判定](#値渡し引数-リテラル-の-const-付与判定) と [ポインター引数の const 付与判定](#ポインター引数の-const-付与判定) を参照します。

### 検証上の注意

Linux (GCC) では `{APP名}_EXPORT` / `{APP名}_API` が空に展開されるため、定義側マクロの有無で不整合は生じず、検出もできません。  
配置ルールの最終確認は MSVC ビルド (`Start-VSCode-With-Env.cmd` 環境) で行います。

## API 設計における概念の分離

### 基本ルール

用途が異なる属性は、暗黙に共有させず独立した概念として設計します。

| ルール | 内容 |
|---|---|
| 属性の独立 | 用途が異なる属性 (例: インスタンス名 / インスタンス識別番号 / 出力ファイル名 / ファイル識別番号) は別フィールド・別 setter にする |
| getter の提供 | setter を作る属性には、対応する getter (確認手段) も用意する |
| デフォルト値の独立 | ある属性のデフォルト値は、他の属性の設定に影響されない形で定義する |
| 排他の区別 | プロセス間排他とプロセス内排他は別概念として扱う。占有モードでもプロセス内は調停して同一資源への出力をサポートする |

### 理由

概念を暗黙に結合すると、ライブラリが識別名を設定した瞬間に出力ファイル名まで変わるなど、利用者の意図しない副作用が生じます。  
識別番号も同様で、用途 (OS トレースの識別 / ファイルの分離) が異なるなら共有しません。

### 判定手順

1. 新しい setter を追加するとき、その値が既存のどの属性のデフォルト値・導出値に影響するかを列挙する。
2. 影響がある場合、その影響が利用者の期待どおりか検討する。期待と異なり得るなら属性を分離する。
3. 分離した各属性に getter を用意する。

## Doxygen コメントのプレースホルダー表記

### 基本ルール

Doxygen コメント内で可変部分 (プレースホルダー) を表すときは、山括弧 `<` `>` ではなく波括弧 `{` `}` を使用します。

```c
/* NG: Doxygen が <ファイル名> を XML/HTML タグと解釈する */
/**
 *  デフォルト パスは log/<ファイル名>.log です。
 *  識別名は @c <name>-<identifier> です。
 */

/* OK: 波括弧であればタグと解釈されない */
/**
 *  デフォルト パスは log/{ファイル名}.log です。
 *  識別名は @c {name}-{identifier} です。
 */
```

### 理由

Doxygen は `<...>` を XML/HTML タグとして解釈し、`warning: Unsupported xml/html tag <ファイル名> found` 警告を出力します。  
さらに、この解釈は XML 出力にも影響し、XML を入力とする Doxybook2 の Markdown 変換が正しく行えなくなります。  
`@c` の指定やバッククォートのコード スパンの外にある場合は、日本語のプレースホルダーでも警告を出力します。

### 適用範囲

- Doxygen コメント (`/** */`) 内のすべてのプレースホルダー表記に適用します。
- 通常の C コメント (`/* */`) は Doxygen の処理対象外ですが、将来の Doxygen 化やコピーを考慮して `{}` に統一します。
- 関数テンプレート構文 (`template <typename T>` など) をコード ブロック (`@code` / バッククォート) 内に書く場合は対象外です。コード ブロック内の `<` `>` はタグと解釈されません。

### 検証

`make doxy` 実行後に生成される `doxy*.warn` で「Unsupported xml/html tag」が検出されないことを確認します。

```bash
grep "Unsupported xml/html tag" <module-dir>/doxy*.warn
```

`doxy*.warn` は生成物のため、警告が残っていても手では編集せず、コメント側を修正して再生成します。

## Doxygen コメントの @p などコマンド引数と日本語句読点の間隔

### 基本ルール

`@p` `@c` `@a` `@b` `@e` `@em` `@ref` など、空白区切りの 1 語を引数に取る Doxygen コマンドでは、引数の直後に日本語句読点 (`、` `。` `，` `．`) を続ける場合、引数と句読点の間に半角スペースを入れます。

**NG: 句読点が引数に取り込まれる**

`@return 成功時は @p buf、EOF またはエラー時は NULL を返します。`

**OK: 半角スペースで区切る**

`@return 成功時は @p buf 、EOF またはエラー時は NULL を返します。`

### 理由

`@p` などのコマンドは、空白区切りの次の 1 トークンをまるごと引数として読み取ります。  
引数と日本語句読点の間に空白がないと、句読点が引数に取り込まれ、意図しない書式や `@ref` のリンク解決失敗を招きます。  
Doxygen はこのケースで警告を出さないため、生成物 (XML/HTML) を確認しない限り気付けません。Doxygen 1.15.0 の XML 出力で実際に不正な入れ子となることを検証済みです。

### 適用範囲

- `@p` `@c` `@a` `@b` `@e` `@em` `@ref` の直後に `、` `。` `，` `．` が続く箇所すべてに適用します。
- コマンドの手前に句読点が続く場合 (`、@p` など) は Doxygen が正しく解釈するため対象外です。

### 検証

`framework/docsfw/bin/text_style_jp.py` の `doxygen-inline-arg-punctuation-spacing` ルールで検出できます。

```bash
python framework/docsfw/bin/text_style_jp.py <対象ファイル> --dry-run
```

## Doxygen コード例内のコメント形式

### 基本ルール

Doxygen コメント (`/** */`) 内の `@code` ~ `@endcode` に書くコード例では、コメントに `/* */` を使用せず `//` を使用します。

```c
/* NG: 「既定値」の後のコメント終端記号が、外側のドキュメント コメントを終端させる */
/**
 *  @par 使用例
    @code{.c}
    int count = 1; /* 既定値 */
    @endcode
 */

/* OK: // であればドキュメント コメントは終端しない */
/**
 *  @par 使用例
    @code{.c}
    int count = 1; // 既定値
    @endcode
 */
```

### 理由

コード例内の `*/` が外側のドキュメント コメントを途中で終端させ、以降のコード例が実コードとして解析されてビルド エラーになります。  
clang-format も実コードと誤認し、コメント内容を再インデントする差分を提示します。

### 適用範囲

- ヘッダー・ソースの Doxygen コメント内の `@code` ブロックすべてに適用します。
- Markdown ファイル (`.md`) 内のコード フェンスは C コメントの外にあるため対象外です (`/* */` を使用できます)。

### 検証

コード例を追加・変更したモジュールがビルド エラーにならないこと、および `git clang-format --diff` がコメント内容の再インデントを提示しないことを確認します。

## 参照

- [`source-style-guideline.md`](source-style-guideline.md) - `.gitattributes` / `.editorconfig` / `.clang-format` によるソース スタイル維持
- [`include-guard-guideline.md`](include-guard-guideline.md) - インクルード ガード命名規則
- [POSIX の名前空間規定](https://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html#tag_15_02_02) - `_t` で終わる型名が処理系用に予約されること
- [C の予約識別子 (cppreference)](https://en.cppreference.com/w/c/language/identifier) - アンダースコアで始まる識別子がファイル スコープで予約されること
- [C++ の識別子 (cppreference)](https://en.cppreference.com/w/cpp/language/identifiers) - 連続する `__` を含む識別子が処理系用に予約されること
- 各 app の特化事項ドキュメント (`docs/{app}/` 配下) - プラットフォーム抽象化、標準時刻型、共通結果コードなどの app 固有規則
- [POSIX `<stddef.h>`](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stddef.h.html) - `size_t` / `ptrdiff_t` の定義
- [POSIX `<sys/types.h>`](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_types.h.html) - `ssize_t` / `off_t` / `time_t` の定義
- [Microsoft C ランタイムの標準型](https://learn.microsoft.com/ja-jp/cpp/c-runtime-library/standard-types?view=msvc-170) - MSVC における `size_t` / `ptrdiff_t` / `off_t` の定義
- [Microsoft C ランタイムの `fseek` / `_fseeki64`](https://learn.microsoft.com/ja-jp/cpp/c-runtime-library/reference/fseek-fseeki64?view=msvc-170) - Windows で 64 bit ファイル位置を扱う API
