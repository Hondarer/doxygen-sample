---
name: create-mock
description: app 配下の C ライブラリ関数に通常の Google Mock 対応 mock を追加または変更するときに使用します。include_override、Mock クラス、ON_CALL、MOCK_WEAK_IMPL、makepart.mk、局所テストを扱います。
---

# app mock の作成

1. `app/general/docs/testing-tutorial.md` の mock 節を読んでください。
2. `framework/testfw/docs/how-to-mock.md` を読んでください。mock 関数本体の一時受けは `mock_ret` です。
3. 対象 app の AGENTS.md と既存の同種 mock を確認してください。
4. override ヘッダー、Mock クラス、`ON_CALL`、関数ラッパーを更新してください。
5. app 関数と同名のラッパーには `MOCK_WEAK_IMPL` を使用してください。
6. `makepart.mk` のソースとリンク先を確認してください。
7. 対象 app の局所テストを実行してください。

`mock_com_util` には `create-mock-com-mock` を優先してください。  
API 表から生成する第三者共有ライブラリには `create-mock-shared-lib` を優先してください。
