# AGENTS.md

## 重要事項

- 自動ステージング、コミット禁止。指示があるまでステージング、コミットは行わないこと。  
  エージェント作業にあわせてユーザーがステージングする場合はある。エージェントは Git の状態参照のみが許可されており、自動アンステージも禁止する。  
  作業の過程でスタッシュする場合もユーザーの許可を得ること。  
  ユーザーがステージング、コミットを指示した場合は、あくまでも例外的かつ単発の許可と解釈し、指示を永続化しないこと。
- 思考の断片は英語でもよいが、ユーザーに気づきを与えたり報告する際は日本語を用いること。
- コミット メッセージは日本語で。
- コミット メッセージを複数行で渡すときは、シェルに応じた構文を厳守すること。Bash ツール (POSIX sh) では PowerShell の here-string `@'...'@` は使えず、`@` がリテラルとして混入する。Bash では `git commit -F - <<'EOF' ... EOF` のヒアドキュメントを使う。PowerShell ツールでは here-string `@'...'@` を使い、閉じ `'@` を行頭に置く。どちらの場合もコミット後に `git log -1 --format='%B'` で先頭・末尾に余分な記号が混入していないか確認すること。
- 対話にあたって、寄せる、詰める、閉じる、乗せる、入れる、整える、潰す、片付ける、仕上げる などの言い回しは避け、正確な表現を心がける。
- サブモジュール配下のファイルを参照、編集する場合は、サブモジュール配下の AGENTS.md もあわせて参照すること。
- `app/{サブフォルダー}` 配下のファイルを参照、編集する場合は、`app/{サブフォルダー}` 配下の AGENTS.md もあわせて参照すること。  
  `app/{サブフォルダー}` はサブモジュールの場合もあれば通常のサブフォルダーの場合もあるが、等しく `app/` 以下の別カテゴリのソース ファイル群として扱う。
- `app/{サブフォルダー}` 配下のコードにおける `goto` と三項演算子の扱いは、[コーディング規範](app/general/docs/coding-guideline.md) の「制御構造の制限」に従うこと。  
  `goto` は、以前の全面禁止から、関数末尾の解放ラベルへの前方ジャンプに限って許容する方針へ改定済み。三項演算子は引き続き禁止であり、既存コードを変更する際に三項演算子があった場合は除去する。

## リポジトリ概要

レガシ C コードのモダナイゼーションを題材に、ビルド、テスト、Doxygen、Markdown 発行、.NET 呼び出し例をまとめた統合ワークスペースです。

主要な git ルートは以下です。

- `framework/doxyfw` - Doxygen と Doxybook2 を使うドキュメント生成フレームワーク
- `framework/docsfw` - Pandoc ベースの Markdown 発行フレームワーク
- `framework/testfw` - Google Test ベースのテスト フレームワーク
- `framework/makefw` - C/C++ と .NET 向けの Make テンプレート群

## 作業時の入口

- `makefile` - ルートの入口。`make`、`make test`、`make doxy`、`make docs`、`make clean` を提供する
- `app/<name>/` - 個別アプリケーション。詳細は各 app 配下の `README.md`、`AGENTS.md` を参照する
- `Directory.Build.props` - .NET 共通設定
- `framework/*` - 各フレームワークの独立した git ルート。変更前に各ルートの `AGENTS.md` と `README.md` を確認する
- `Start-VSCode-With-Env.cmd` - Windows で GNU Make と MSVC の環境を整えて VS Code を起動する

## make ファイル関連の指針

- makepart.mk, makechild.mk, makelocal.mk など、make ファイルの断片については、空ファイルは不要。  
  see: `framework/makefw/docs/makeparts.md`
- 末端の makefile は小文字の `makefile` を正とし、`Makefile` (大文字始まり) は使用しない。サブモジュールなど管理対象外の OSS 由来ファイルはこの限りではない。  
  `.gitignore` は小文字の `makefile` のみを unignore しており、大文字小文字を区別するファイル システムでは `Makefile` が暗黙に管理対象外になる。

## make コマンド実行の指針

**フル ビルドは時間がかかる。以下の指針に従うこと:**

1. **修正箇所の特定** - どのモジュール (`app/` 配下のサブディレクトリ) を変更したかを明確にする
2. **局所的なテスト** - 変更したモジュール配下で `cd app/MODULE && make test` を実行し、そのモジュールのみビルド・テストする
3. **clean + 再 make は高コスト** - make clean が必要になるのはファイル パスの変更などが伴う場合だけであり、依存ファイルの内容変更であれば make だけで事足りるため、むやみに make clean しない。
4. **全体テストが必要な場合** - 以下の場合は全体テストをユーザーに相談してから実行する:
    - ローカル テストで外部依存エラーが検出された場合
    - 複数モジュールにまたがる変更をした場合
    - ライブラリ層 (framework/) を変更した場合

### ビルド警告の確認

makefw はコンパイラとリンカーの警告を `.warn` ファイルへ記録し、警告がない場合はファイルを削除する。  
ビルド後は、対象モジュールから内容のある `.warn` ファイルを検索して警告を確認すること。

```bash
# 警告ファイルの一覧を確認
cd <module-dir>
find . -type f -name '*.warn' -size +0 -print

# 警告内容を確認
find . -type f -name '*.warn' -size +0 -print0 | xargs -0 -r sed -n '1,200p'
```

警告が見つかった場合は、`.warn` ファイルを直接編集せず、警告の原因を修正してから同じビルドを再実行すること。

また、Windows において、make 終了時にプロセス終了を検知できないケースがある。

make は約 60 秒間無応答になる場合があるため、出力を定期的に確認しながら、無応答時は 120 秒程度を目安に待機すること。

## 主要コマンド

```bash
make
make test
make doxy
make cleandocs
make docs
make clean
make skills
make sync-app-env
```

`make sync-app-env` は、`app/<name>/**/makepart.mk` の `OUTPUT_DIR` を正本として、実行時のコマンド探索パスとライブラリ探索パスを `.vscode` 配下へ生成する。app の追加・削除の際にこれらを手で編集せず、本コマンドを実行すること。  
`.github/workflows/ci.yml` と `.jenkins` は `bin/load-app-env.sh` を介して `.vscode/.env.linux` / `.vscode/.env.windows` を読むため、app の増減で編集は発生しない。  
see: `app/general/docs/vscode-variables.md`

## テスト コメントの注意点

- `[手順]` と `[確認_*]` などのテスト エビデンス用コメントは、コメントだけで手順と確認内容の対応が分かるように記載すること。
- `TEST`、`TEST_F`、`TEST_P` などのテスト本体では、`{` の次の非空行を `// Arrange` とすること。
- ローカル変数の宣言、モック オブジェクトの生成、テスト データや設定の準備は、すべて `// Arrange` より後に記載すること。
- `[状態]` タグは、Arrange フェーズの内容をテスト エビデンスへ出力する場合に使用し、タグを付けない場合も `// Arrange` は省略しないこと。
- `// Arrange`、`// Pre-Assert`、`// Act`、`// Assert` は、各フェーズに処理がない場合も省略しないこと。
- Assert 後にリソースの解放、ファイルの削除、ハンドルの終了、グローバル状態の復元などの明示的な後処理がある場合は、その直前に `// Cleanup` を記載すること。
- 明示的な後処理がない場合は、空の `// Cleanup` を記載しないこと。
- 終了系 API 自体の挙動を試験する呼び出しや、結果を確定するために必要な終了操作は Act とし、fixture の `TearDown`、ヘルパー内の後処理、RAII による自動解放にはテスト本体の `// Cleanup` を要求しない。
- 関数の戻り値を確認する場合は、「戻り値が 0 であること。」のように主語を省略せず、「packet_parse の戻り値が POTR_OK であること。」のように対象の関数名を記載すること。
- 同じ関数を複数回呼び出す場合は、引数、条件、呼び出し順のいずれかも記載し、どの呼び出しの戻り値を確認しているかを区別すること。
- マルチ フェーズ テスト (1 つのテスト関数内で Arrange/Pre-Assert/Act/Assert のサイクルを複数回含むテスト。部分反復を含む) は、`framework/testfw/docs/about-test-phase.md` の「シングル フェーズ テストとマルチ フェーズ テスト」に従い、実際に繰り返す構造コメントにだけ `_N` を付与すること。1 回目のサイクルには番号を付けず、`// Cleanup` にも番号を付けないこと。

## Markdown 記述の注意点

Markdown の本文段落では、1 文の途中に表示幅調整目的の改行を入れないこと。原則として 1 文を 1 行に書き、改行は文末または Markdown の構造上必要な箇所に限ること。

ドキュメント (`.md`) 内でファイル ツリーや階層構造を記述するとき、罫線文字 (`└` `├` `─` など、Unicode Ambiguous) は使用しない。日本語フォントはこれらに全角グリフを割り当てるため、コンソールやプレビューでインデントがずれる。ツリー構造には ASCII 記号 (`+` `-` `|`) を使うこと。  
背景: [east-asian-ambiguous-width.md](framework/docsfw/docs/east-asian-ambiguous-width.md)

Markdown 文書を作成または変更した場合は、`framework/docsfw/bin/text_style_jp.py` を使って日本語スタイル チェックを実施し、指摘された項目をすべて対処した上で文書を確定すること。

`text_style_jp.py` は、本文段落の一文一行を Markdown の表示にも反映するため、文末へ強制改行用の半角空白 2 個を付与する場合がある。  
この空白は意図的な Markdown 構文であるため、`git diff --check` が `trailing whitespace` として報告しても削除しないこと。

```bash
# 変更箇所の確認 (ルール名と差分を表示)
python framework/docsfw/bin/text_style_jp.py <対象ファイル> --dry-run
# 問題がなければ直接上書きで適用する
python framework/docsfw/bin/text_style_jp.py <対象ファイル> --in-place
```

**`--dry-run` を必ず使用すること。** `--check` は日本語スタイルの自動補正の有無しか判定しないため、補正を伴わない `box-drawing` ルール (全角罫線文字の検出) は `--check` では報告されない。全角罫線の混入を検出するには `--dry-run` が必要。

スタイル チェックの結果、日本語として不自然な変換 (変換すべきでない語の置換、文脈上不自然な表記統一、カタカナ語の不自然な分割など) がツールから提案された場合は、その変換を適用せず、ユーザーに辞書またはアルゴリズムの更新案を提示すること。

## ソース ファイル作成時の注意点

新規ファイル作成時は `clang-format`、変更時は `git-clang-format` を利用してソースの整形を行うこと。  
この際、ネストの整形によって Doxygen コメントの字下げが崩れることがあるため、整形後にはコメントの字下げチェックも実施すること。

コーディング規範 (整数型の選択、関数引数の異常入力対応など) は [コーディング規範](app/general/docs/coding-guideline.md) に従うこと。

非自明な挙動 (OS の仕様やバグなど) への回避策や、調査して判明した根拠をコードに反映するときは、その根拠となる URL を該当箇所のコメントに `see: <URL>` 形式で残すこと。コミット メッセージだけでなくソース内に残し、後から再調査せずに意図を追えるようにする。

一括置換 (raw API 呼び出しのラッパー化など) で意図的に置換対象外とした箇所には、対象外である理由をコメントとして該当行に残すこと。コメントがないと、将来の一括置換や grep ベースの点検で「置換漏れ」と誤認され、不適切に変更されるおそれがある。対象外の判断を撤回して置き換えた場合は、そのコメントも同時に除去する。

## app/ ライブラリのインクルード規則

`app/{サブフォルダ}` 配下の C ライブラリは、以下の 3 層構造でヘッダーを管理する。

| ディレクトリ | 用途 | インクルード形式 |
|---|---|---|
| `prod/include/` | 公開 API (ライブラリ利用者向け) | `<lib/file.h>` |
| `prod/include_internal/` | ライブラリ内部共有ヘッダー (`.c` をまたいで参照) | `<lib/subdir/file.h>` |
| `prod/libsrc/` | ソース ファイル (`.c`) のみ。ヘッダーは置かない | - |

### _internal の付与ルール

#### ヘッダー ファイル名

- 同名の公開ヘッダーが存在する場合のみ `_internal` を付与する (例: `console.h` が公開にあるため `console_internal.h`)
- 対応する公開ヘッダーが存在しない場合はサフィックスなし (例: `path_format.h`)

#### シンボル名 (関数・型・外部リンケージ変数)

- `prod/include_internal/` で宣言する関数と型は `<lib>_internal_<rest>` とする (例: `sample_internal_registry_add`)
- `prod/include_internal/` で `extern` する変数は `g_<lib>_internal_<rest>` とする (例: `g_sample_internal_default_registry`)
- 公開 (`prod/include/`) の関数と型は `<lib>_<rest>`、公開共有変数は `g_<lib>_<rest>` とし、`_internal_` を付けない
- 公開共有変数は必要最低限に厳選する。詳細は [コーディング規範](app/general/docs/coding-guideline.md) を参照する
- `static` 関数にはライブラリ接頭辞も `_internal_` も付けない。ファイル内共有変数は `s_<rest>`
- ファイル名規則とシンボル規則は付与条件が異なる。詳細は [コーディング規範](app/general/docs/coding-guideline.md) の「命名規則」を参照する

### makepart.mk の INCDIR ルール

- `INCDIR` に追加するのは `include/` と `include_internal/` のみとする
- `libsrc/` のサブディレクトリは `INCDIR` に追加しない

## 注意点

- 作成するファイルは Linux (GCC)、Windows (MSVC) のクロスプラットフォームで動作することを念頭に置いて作業すること。
- Windows では GNU Make が POSIX シェルで動く前提です。必要に応じて `Start-VSCode-With-Env.cmd` から環境を整えること。
- ドキュメント生成と公開は `framework/doxyfw` と `framework/docsfw` の連携で成り立つため、出力パスやスクリプト名を変更する際は両方を確認すること。
- テスト関連の変更では `framework/testfw` とその配下の `gtest` サブモジュールの役割を理解し区別すること。
- `bin/` 配下の Python スクリプトで日本語を出力するときは、モジュール レベルに以下を追加すること。  
  Windows のデフォルト `sys.stdout.encoding` は `cp932` であり、出力が文字化けする。

  ```python
  sys.stdout.reconfigure(encoding="utf-8")
  sys.stderr.reconfigure(encoding="utf-8")
  ```

- `TEST_SRCS` / `ADD_SRCS` に指定したソース ファイルは、`make test` 時にビルド ディレクトリへシンボリック リンクまたはコピーとして取り込まれる。ビルド ディレクトリ内のファイルを直接変更しても次回 `make test` で上書きされるため、`prod/` の実体ファイルを変更すること。  
  see: `framework/makefw/docs/makeparts.md` の「TEST_SRCS / ADD_SRCS の留意事項」
