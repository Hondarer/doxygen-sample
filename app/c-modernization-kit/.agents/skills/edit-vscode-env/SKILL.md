---
name: edit-vscode-env
description: |
  VS Code の環境変数設定を編集するときに使うスキルです。
  新しい app モジュールの PATH 追加、envFile 方式の構造、
  編集対象ファイルの一覧をまとめます。
when_to_use: |
  - 新しい app モジュールを追加して PATH を通すとき
  - VS Code のデバッグやタスクで DLL/SO が見つからないとき
  - 環境変数設定の構造を確認したいとき
---

# VS Code 環境変数の編集

このスキルは `.vscode/` 配下の環境変数設定を対象にします。  
詳細なドキュメントは `docs/vscode-variables.md` を参照してください。

## 構造

VS Code の環境変数設定は envFile 方式で一元管理しています。

| ファイル | 用途 | envFile 対応 |
|---|---|---|
| `.vscode/.env.linux` | Linux 向け PATH / LD_LIBRARY_PATH | - |
| `.vscode/.env.windows` | Windows 向け PATH | - |
| `.vscode/settings.json` | 統合ターミナル用環境変数 | 非対応 |
| `.vscode/launch.json` | デバッグ構成 (envFile 参照) | 対応 |
| `.vscode/tasks.json` | タスク (envFile 参照) | 対応 |

`launch.json` と `tasks.json` は `.env.linux` / `.env.windows` を参照するため、直接編集は不要です。  
`settings.json` の `terminal.integrated.env.*` は envFile をサポートしないため、別途編集が必要です。

## 新しいモジュールを追加・削除したとき

`.vscode` の `PATH` / `LD_LIBRARY_PATH` は手で編集しません。
`app/<name>/**/makepart.mk` の `OUTPUT_DIR` を正本として、`bin/sync-app-env.sh` が生成します。

```bash
make sync-app-env
```

生成される内容は次の規則で決まります。

- `OUTPUT_DIR` に `$(MYAPP_DIR)/prod/cbin` が現れる app は、`app/<name>/prod/cbin` がコマンド探索パスへ入る
- `OUTPUT_DIR` に `$(MYAPP_DIR)/prod/lib` が現れる app は、`app/<name>/prod/lib` がライブラリ探索パスへ入る (Windows は `PATH`)
- 並び順は app 名の `LC_ALL=C sort`。Windows の `PATH` は app ごとに `lib`、`cbin` の順

したがって、新しいモジュールで実行ファイルや共有ライブラリを出力する場合にすることは、対象ディレクトリの `makepart.mk` に `OUTPUT_DIR` を設定することだけです。

```make
# app/<name>/prod/src/cmd/makepart.mk
OUTPUT_DIR := $(MYAPP_DIR)/prod/cbin
```

```make
# app/<name>/prod/libsrc/makepart.mk
OUTPUT_DIR := $(MYAPP_DIR)/prod/lib
```

生成対象は `.vscode` の 4 ファイルにとどまらず、`.github/workflows/ci.yml`、`.jenkins/inner-build.sh`、`.jenkins/README.md` も同時に更新されます。

ルートの `make` の完了後には `bash bin/sync-app-env.sh --check` が自動で走り、差異があれば `app/app_env.warn` が生成されます。

## env ファイルの形式

`.env.linux` と `.env.windows` は 1 行 1 変数の形式です。

```text
PATH=値1:値2:${env:PATH}
LD_LIBRARY_PATH=値1:値2:${env:LD_LIBRARY_PATH}
```

- `${workspaceFolder}` でワークスペース ルートを参照
- `${env:PATH}` で既存の PATH を末尾に追加
- Linux は `:` 区切り、パス区切りは `/`
- Windows は `;` 区切り、パス区切りは `\`

## launch.json / tasks.json の envFile 参照

各デバッグ構成とタスクは `envFile` で env ファイルを参照します。

```json
// launch.json (Linux 構成)
"envFile": "${workspaceFolder}/.vscode/.env.linux"

// launch.json (Windows 構成)
"envFile": "${workspaceFolder}/.vscode/.env.windows"

// tasks.json
"linux": {
    "options": {
        "envFile": "${workspaceFolder}/.vscode/.env.linux"
    }
},
"windows": {
    "options": {
        "envFile": "${workspaceFolder}/.vscode/.env.windows"
    }
}
```

## 判断基準

`lib` や `cbin` を出力するかどうかは `makepart.mk` の `OUTPUT_DIR` で表現します。
`LIB_TYPE` (static / shared / both) による絞り込みは行いません。
静的ライブラリだけを出力する app のディレクトリが探索パスに載っても実害がないため、判定を `OUTPUT_DIR` の 1 つに統一しています。

「存在するが実行時には不要」といった個別の除外は設けていません。
除外が必要になった場合は、`bin/sync-app-env.sh` の導出規則そのものを見直します。

## 確認項目

- `makepart.mk` の `OUTPUT_DIR` が意図した出力先を指しているか
- `make sync-app-env` を実行したか
- 生成された差分に、想定外の app の増減がないか
- `bash bin/sync-app-env.sh --check` が終了コード 0 で完了するか
- マーカー (`# BEGIN app-env-sync` など) の内側を手で編集していないか

## 参照ドキュメント

- `docs/vscode-variables.md` - VS Code における環境変数と保守手順の詳細
- `bin/sync-app-env.sh` - 実行時パス設定の生成スクリプト
