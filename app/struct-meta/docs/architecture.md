# struct-meta のアーキテクチャー

## 目的

`struct-meta` は、構造体レイアウトの表現と、その表現を使う操作を分離します。  
将来 `com_util` へ移しやすいように、再利用可能な機能を 1 個の共有ライブラリ `libstruct_meta` にまとめ、PoC の構文解析器とサンプルをコマンドとして分離します。

## 依存方向

```text
struct-meta-gen --生成--> meta
                           ↓
                        access
                       ↙   ↓   ↘
                    json patch print
                      ↓
                  json/file

struct-meta-sample --> generated catalog + json/file + patch + print
```

`meta` は記述子、フィールド種別、汎用属性、再帰検査を提供します。  
`access` はフィールド検索、属性検索、配列要素、パスの解決を提供し、構造体へのポインター演算を集約します。  
パスは `scores`、`scores[1]`、`addresses[0].city` の形式を扱い、空文字列や途中で配列添字を省略した曖昧なパスを拒否します。  
`json`、`patch`、`print` はアクセス機能を利用し、メタデータのレイアウトを独自に解釈しません。

`patch` は、ルートからメニューを辿る編集と、`access` が解決したパスから始める編集を提供します。  
パスが配列全体で終わる場合は要素選択へ、構造体で終わる場合はその構造体のフィールド選択へ進みます。  
どちらの編集方法でも、メニューは現在位置と選択候補の完全な C フィールド パスを表示します。

## 記述子と属性

公開 API の入口は、利用前に `struct_meta_descriptor_validate()` で記述子全体を再帰検査します。  
検査対象には、フィールド範囲、型ごとの制約、ネスト記述子との要素サイズ一致、属性キーの妥当性と重複、記述子の循環が含まれます。

属性は key/value の配列です。  
JSON 変換は `json.name`、`json.ignore`、`json.required` を解釈します。  
属性モデル自体は JSON に依存しないため、別カテゴリが独自の名前空間を追加できます。

## 生成器の境界

`struct-meta-gen` は flex/bison を使う限定的な C ヘッダー解析器です。  
`typedef struct { ... } name;`、対応済みプリミティブ型、同じヘッダー内のネスト構造体、固定長配列を扱います。  
プリプロセッサ展開、任意の宣言子、ビット フィールド、ポインター、可変長配列を含む完全な C 文法は対象外です。

生成器はレイアウトを計算せず、生成 C コードへ `offsetof` と `sizeof` を出力します。  
これにより、Linux/GCC と Windows/MSVC の実際のコンパイラがレイアウトを決定します。  
生成カタログは、型数、列挙 ID による取得、構造体名による検索を提供します。

Doxygen の JSON 属性の解析は生成器だけの例外的責務です。  
生成器は解析結果を `json.name`、`json.ignore`、`json.required` という汎用属性へ変換し、JSON 実装の構造を記述子へ埋め込みません。

## ビルド

解析対象ヘッダーは `struct-meta-sample/makepart.mk` の `STRUCT_META_GEN_HEADERS` で静的に宣言します。  
`struct-meta-gen` を先にビルドする必要があるため、`prod/src/cmd/makelocal.mk` が再帰 make の順序を明示します。  
flex/bison と `gen/*.c` の汎用コンパイル規則は makefw を利用し、ヘッダーからメタデータを作る規則はこの app 内に保持します。

Windows で WinFlexBison を使う場合は、`BISON=win_bison` と `FLEX=win_flex` を指定します。  
makefw は Windows で `FLEXFLAGS` の既定値を `--wincompat` とし、MSVC で利用できるコードを生成します。  
`.l` と `.y` の変換対象は `PLATFORM_*` で出し分けず、flex/bison をクロスプラットフォームな外部コマンドとして扱います。

生成ツール `struct-meta-gen` の実行ファイル名は、Linux の `struct-meta-gen` と Windows の `struct-meta-gen.exe` を `PLATFORM_*` で明示的に切り替えます。  
これにより、生成規則の前提条件と実際の Windows ビルド成果物を一致させます。

## JSON デコードの更新単位

JSON デコードは従来どおり、検証済みのフィールドから順に出力先を更新します。  
後続フィールドでエラーが発生しても、先に更新したフィールドは元へ戻しません。  
呼び出し側が原子的な更新を必要とする場合は、一時領域へデコードして成功後に置き換えます。
