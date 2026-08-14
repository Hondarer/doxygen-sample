---
name: edit-vscode-env
description: app の追加や削除、実行ファイルや共有ライブラリの出力先変更に伴って VS Code と CI の探索パスを更新するとき、または DLL や SO が見つからない問題を調査するときに使用します。
---

# VS Code 環境変数の更新

1. `app/general/docs/vscode-variables.md` を読んでください。
2. `makepart.mk` の `OUTPUT_DIR` と app 依存関係を正本として確認してください。
3. `.vscode`、GitHub Actions、Jenkins の生成ファイルを手編集しないでください。
4. ルートで `make sync-app-env` を実行してください。
5. `bash bin/sync-app-env.sh --check` で差分がないことを確認してください。
6. Linux の `LD_LIBRARY_PATH` と Windows の `PATH` の双方を確認してください。

探索パスの順序を変更する場合は、同名ライブラリの解決順も確認してください。
