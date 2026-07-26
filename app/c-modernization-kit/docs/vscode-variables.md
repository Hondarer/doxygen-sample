# VS Code における環境変数と c_cpp_properties.json の保守手順

本ドキュメントは、VS Code で環境変数がどこに効くかを説明したうえで、このリポジトリでアプリ追加・削除・改名が発生した際に、どのファイルをどう見直すべきかをまとめた手引きです。

## c_cpp_properties.json の正本

`-I` と `-D` の正本は `.vscode/c_cpp_properties.json` ではありません。  
`INCDIR` は `makepart.mk`、`app/makepart.mk`、各 C/C++ app の `app/<name>` 配下にあるすべての `makepart.mk` の合成結果が正本です。  
`DEFINES` は `makepart.mk`、`app/makepart.mk`、各 C/C++ app の `app/<name>/makepart.mk` の合成結果が正本であり、`.vscode/c_cpp_properties.json` はその派生物です。

### 基本ルール

- リポジトリ全体に効かせる IntelliSense 向け include / define は `makepart.mk` または `app/makepart.mk` に書く
- app 共通の IntelliSense 向け include / define は `app/<name>/makepart.mk` に書く
- 個別ターゲットだけが必要とする追加 `INCDIR` は、対象ディレクトリ配下の `makepart.mk` に書くと `.vscode/c_cpp_properties.json` にも反映される
- 個別ターゲットだけが必要な `DEFINES` は、必要なら `.vscode` へは反映されない前提で下位の `makepart.mk` に書く
- `.vscode/c_cpp_properties.json` を直接編集しても make のビルド設定には反映されない
- Linux の `_DEFAULT_SOURCE` のように実ビルドでも必要な define は `app/makepart.mk` などの正本側へ書く
- `TARGET_ARCH` は app 側の実値を `.vscode` へ持ち込まず、Linux / Win32 ともに `TARGET_ARCH=target_arch` を同期スクリプトが補う
- `.vscode/c_cpp_properties.json` の配列は、特殊項目を先頭に固定し、それ以外をソートして同期する

### 同期の流れ

`make -C app` のデフォルト ビルド後には、`INCDIR` は `makepart.mk`、`app/makepart.mk`、`app/*/**/makepart.mk`、`DEFINES` は `makepart.mk`、`app/makepart.mk`、`app/*/makepart.mk` の同期結果と `.vscode/c_cpp_properties.json` の dry-run 比較が自動で走ります。  
差異がある場合は `app/c_cpp_properties.warn` が生成され、既存の WARNING 表示と warn artifact 収集にそのまま乗ります。

警告が出たら、ワークスペース ルートで次を実行して `.vscode/c_cpp_properties.json` を更新します。

```bash
bash framework/makefw/bin/sync_c_cpp_properties.sh --write
```

差異確認だけを手動で行いたい場合は次を使います。

```bash
bash framework/makefw/bin/sync_c_cpp_properties.sh --check
```

この同期スクリプトは `.vscode/c_cpp_properties.json` の `defines` に必要なコメントも復元します。

### c_cpp_properties.json を見直すタイミング

- `INCDIR` を変更したとき  
  `makepart.mk`、`app/makepart.mk`、または `app/<name>` 配下の任意の `makepart.mk`
- `DEFINES` を変更したとき  
  `makepart.mk`、`app/makepart.mk`、または `app/<name>/makepart.mk`
- C/C++ app を追加・削除・改名したとき
- `app/c_cpp_properties.warn` が出たとき

### .vscode 側の特殊条件

`.vscode/c_cpp_properties.json` の `defines` は、app 正本の単純な mirror ではありません。

- `_DEFAULT_SOURCE` のような通常 define は `makepart.mk` / `app/makepart.mk` 側の正本からそのまま反映する
- `TARGET_ARCH` は app 側の実値を無視し、常に `TARGET_ARCH=target_arch` を使う
- `TARGET_ARCH=target_arch` を先頭に置き、それ以外の項目はソートして並べる

これは IntelliSense 用の互換条件ですが、通常の define 自体は make の build 設定と分離しません。

### 再チェック

```bash
make -C app
test ! -f app/c_cpp_properties.warn
```

## 概要

本リポジトリでは、実行時に必要なライブラリ探索パスとコマンド探索パスを複数の場所で管理しています。

- VS Code の統合ターミナル
- VS Code の `make test` タスク
- VS Code のデバッグ構成
- GitHub Actions
- Jenkins

更新要否の判断は、GitHub Actions の設定そのものではなく、`app` 配下の各アプリケーションの構成と依存関係に基づいて行います。  
ただし、現時点では GitHub Actions の CI が成功しているため、`ci.yml` は少なくとも現在の必要十分な実装例として参照できます。

## VS Code で環境変数が効く場所

### .vscode/settings.json

`terminal.integrated.env.*` は、VS Code の統合ターミナルにだけ反映されます。  
新しく開いたターミナルには反映されますが、既存のターミナルには反映されません。

このリポジトリでは、Linux では `LD_LIBRARY_PATH` と `PATH`、Windows では `PATH` を設定しています。  
これらの値は `bin/sync-app-env.sh` が生成します。

### .vscode/tasks.json

`tasks.json` の `options.envFile` は、タスク実行時の環境変数を外部ファイルから読み込みます。  
このリポジトリの `make test` タスクでは、OS ごとに `.vscode/.env.linux` または `.vscode/.env.windows` を参照しています。

特に、テスト失敗時に

```text
error while loading shared libraries: libxxxx.so: cannot open shared object file
```

のようなエラーが出た場合は、まず `tasks.json` の Linux 側 `LD_LIBRARY_PATH` を疑います。

### .vscode/launch.json

デバッグ構成は統合ターミナルの環境変数をそのまま使うわけではありません。  
必要な環境変数は、各デバッグ構成の `envFile` で外部ファイルを参照します。

- Linux 構成は `.vscode/.env.linux` を参照
- Windows 構成は `.vscode/.env.windows` を参照

### .vscode/.env.linux / .vscode/.env.windows

`launch.json` と `tasks.json` が共通で参照する環境変数定義ファイルです。  
`PATH` と `LD_LIBRARY_PATH` の行は `bin/sync-app-env.sh` が生成するため、手で編集しません。

`MAKEFW_HOME`、`DOXYFW_HOME`、`TESTFW_HOME`、`DOCSFW_HOME` もここで定義します。  
`MAKEFW_HOME` は make テンプレート群 (`framework/makefw`) の場所を表し、`make` / `make test` / `make doxy` などの実行で必須です。未設定だと `MAKEFW_HOME is required. Export MAKEFW_HOME before running make` で停止します。  
`DOXYFW_HOME` は Doxygen 生成フレームワークの場所を表し、`make doxy` はこの値を使って doxyfw を呼び出します。  
`TESTFW_HOME` はテスト フレームワークの場所を表し、`make` / `make test` はこの値を使って testfw をビルドし、テスト実行スクリプトやライブラリを参照します。  
`DOCSFW_HOME` は Markdown 発行フレームワークの場所を表し、VS Code の Markdown 発行タスクと `make docs` が参照します。

`settings.json` の `terminal.integrated.env.*` は `envFile` をサポートしないため、ターミナル用の `PATH` は `settings.json` にも同じ内容が必要です。  
この重複も `bin/sync-app-env.sh` が両方へ同時に生成します。

## 実行時パスの正本

`.vscode` と CI に書かれている実行時のコマンド探索パスとライブラリ探索パスは、これらのファイルが正本ではありません。
`app/<name>` 配下の `makepart.mk` が設定する `OUTPUT_DIR` が正本であり、各設定ファイルはその派生物です。

`bin/sync-app-env.sh` が `app/<name>/**/makepart.mk` を make で評価し、次の規則で導出します。

- `OUTPUT_DIR` に `$(MYAPP_DIR)/prod/cbin` が現れる app は、`app/<name>/prod/cbin` をコマンド探索パスへ追加する
- `OUTPUT_DIR` に `$(MYAPP_DIR)/prod/lib` が現れる app は、`app/<name>/prod/lib` をライブラリ探索パスへ追加する (Windows では `PATH` へ追加する)
- `test/lib` のように `prod/` 以外を指す `OUTPUT_DIR` は対象外とする
- 並び順は app 名の `LC_ALL=C sort` とし、Windows の `PATH` は app ごとに `lib`、`cbin` の順に並べる

`LIB_TYPE` (static / shared / both) による絞り込みは行いません。
静的ライブラリだけを出力する app のディレクトリが探索パスに載っても実害がないため、判定を `OUTPUT_DIR` の 1 つに統一しています。

`.vscode/pub_markdown.config.yaml` の `mergeSubfolderDocs` は、`app/<name>/docs` の有無から導出します。

app の一覧は `framework/makefw/bin/resolve_app_deps.sh --app-order` から取得するため、app を追加・削除しただけで導出結果が追従します。

### 生成対象と生成区間

| 対象 | ファイル | 生成される箇所 |
|---|---|---|
| 統合ターミナル | `.vscode/settings.json` | Linux の `LD_LIBRARY_PATH` / `PATH`、Windows の `PATH` |
| VS Code テスト タスク / デバッグ (Linux) | `.vscode/.env.linux` | `PATH` と `LD_LIBRARY_PATH` の行 |
| VS Code テスト タスク / デバッグ (Windows) | `.vscode/.env.windows` | `PATH` の行 |
| Markdown 発行 | `.vscode/pub_markdown.config.yaml` | `mergeSubfolderDocs` の行 |
| GitHub Actions | `.github/workflows/ci.yml` | `# BEGIN app-env-sync (linux)` / `(windows)` の区間 |
| Jenkins 実装 | `.jenkins/inner-build.sh` | `# BEGIN app-env-sync` の区間 |
| Jenkins 説明 | `.jenkins/README.md` | `<!-- BEGIN app-env-sync -->` の区間 |

マーカーで囲まれた区間の中身は毎回上書きされるため、手で編集しないでください。
`.vscode` の 4 ファイルはキー単位の行置換であり、その行以外は保持されます。

### 同期の流れ

ルートの `make` (および `make with-cov`) の完了後に `bash bin/sync-app-env.sh --check` が自動で走ります。
差異がある場合は `app/app_env.warn` が生成され、CI の warn artifact 収集にもそのまま乗ります。

警告が出たら、ワークスペース ルートで次を実行して各ファイルを更新します。

```bash
make sync-app-env
```

スクリプトを直接呼び出す場合は次のとおりです。

```bash
bash bin/sync-app-env.sh --write
bash bin/sync-app-env.sh --check
```

`--check` は差異があるときに終了コード 3 を返します。
これは warning を表す終了コードであり、ビルドは失敗しません。

### app を追加・削除したときにすること

`.vscode` と CI を手で編集する必要はありません。
`make sync-app-env` を実行し、生成された差分をコミットします。

`.gitmodules` への submodule 登録と `README.md` のサブモジュール一覧は自動生成の対象外のため、従来どおり手で更新します。

### 再チェック

```bash
make sync-app-env
bash bin/sync-app-env.sh --check
```

## framework home 系の環境変数を変更する場合

`MAKEFW_HOME` / `DOCSFW_HOME` / `DOXYFW_HOME` / `TESTFW_HOME` のような framework home 系の環境変数を変更する場合は、PATH 系とは別に以下も確認します。

| 対象 | ファイル | 何を更新するか |
|---|---|---|
| VS Code タスク / デバッグ | `.vscode/.env.linux`, `.vscode/.env.windows` | `MAKEFW_HOME`, `DOCSFW_HOME`, `DOXYFW_HOME`, `TESTFW_HOME` |
| VS Code 統合ターミナル | `.vscode/settings.json` | `MAKEFW_HOME`, `DOCSFW_HOME`, `DOXYFW_HOME`, `TESTFW_HOME` |
| GitHub Actions 全ジョブ | `.github/workflows/ci.yml` | workflow-wide `env` の `MAKEFW_HOME`, `DOCSFW_HOME`, `DOXYFW_HOME`, `TESTFW_HOME` |
| Jenkins コンテナー内ビルド | `.jenkins/inner-build.sh` | `/workspace` 基準の framework home 変数。特に `MAKEFW_HOME` は `make` 実行で必須 |
| Jenkins 説明 | `.jenkins/README.md`, `docs/c-modernization-kit/skill-guide/07-ci-cd/jenkins.md` | Jenkins 上の既定値と上書き方法 |
| CI 詳細説明 | `docs/c-modernization-kit/github-actions.md` | GitHub Actions 上の既定値と `publish-docs` での利用 |

## 個別 README の実行例を更新する

利用者向けの README に、古い `app/<name>/prod/...` パスや手動 `LD_LIBRARY_PATH` 設定例が残っていないかを確認します。
これらは自動生成の対象外です。特に、アプリ固有 README のトラブルシュート節は見落としやすい箇所です。

## 再チェック用チェックリスト

- `make sync-app-env` を実行し、生成された差分を確認した
- 生成結果に想定外の app の増減がないことを確認した (増減がある場合は `makepart.mk` の `OUTPUT_DIR` を疑う)
- `bash bin/sync-app-env.sh --check` が終了コード 0 で完了することを確認した
- framework home 系の環境変数を変更した場合は、上表のファイルを手で更新した
- 個別 README の実行例やトラブルシュートを更新した
- 旧 `app/<name>/prod/...` 構成が残っていないことを確認した
- 代表的なテストまたは実行例でライブラリ解決エラーが出ないことを確認した

## 確認コマンド例

### 正本 (OUTPUT_DIR) の確認

```bash
rg -n 'OUTPUT_DIR' --glob 'app/**/makepart.mk'
```

### 設定箇所の確認

```bash
rg -n "LD_LIBRARY_PATH|GITHUB_ENV|GITHUB_PATH|terminal.integrated.env|Set PATH for tests|Set PATH and library path for tests" \
  .vscode .github .jenkins -S
```

### 旧構成の残存確認

```bash
rg -n '\$\{workspaceFolder\}/prod/|/workspace/prod/|/path/to/.*/prod/' \
  docs .vscode .github .jenkins app -S
```

### 依存関係の確認

```bash
rg -n "LIBS \\+=|LIBSDIR \\+=|DllImport|NativeLibrary|include <com_util" app -S
```

### 代表テストの最小確認

Linux では、ライブラリ探索パスを明示して代表テストの起動確認ができます。

```bash
LD_LIBRARY_PATH=$PWD/app/calc/prod/lib:$PWD/app/calc.net/prod/lib:$PWD/app/override-sample/prod/lib:$PWD/app/porter/prod/lib:$PWD/app/com_util/prod/lib:${LD_LIBRARY_PATH} \
  ./app/calc/test/src/main/addTest/bin/addTest --gtest_list_tests
```

## 現在の VS Code / CI 設定を見る場所

日常的な保守では、次のファイルをあわせて確認してください。

- `.vscode/.env.linux`
- `.vscode/.env.windows`
- `.vscode/settings.json`
- `.github/workflows/ci.yml`
- `.jenkins/inner-build.sh`

これらはいずれも `bin/sync-app-env.sh` の生成物です。
内容に疑問がある場合は、`app/<name>/**/makepart.mk` の `OUTPUT_DIR` に立ち戻って確認してください。
