---
name: create-mock-shared-lib
description: 第三者共有ライブラリの公開 API を API 表から Google Mock 対応 mock にするときに使用します。real delegate、動的シンボル解決、Windows の dllimport 解除、実ライブラリとの同時リンク禁止を扱います。
---

# 第三者共有ライブラリの mock

1. `app/general/docs/shared-library-mock-guideline.md` を読んでください。
2. 対象 app の AGENTS.md と README.md でライブラリ固有設定を確認してください。
3. API 表から宣言、`MOCK_METHOD`、real delegate、ラッパー、`ON_CALL` を生成してください。
4. 可変長引数関数と公開変数を別扱いにしてください。
5. Windows の import 属性を解除する定義を確認してください。
6. 利用側が実ライブラリと mock を同時リンクしていないことを確認してください。
7. 対象 app の公開 API 網羅テストと局所テストを実行してください。

既定動作は実ライブラリへの委譲とし、解決失敗時は理由を出力して終了してください。
