---
name: create-unit-test
description: app 配下の C または C++ 単体テストを作成、変更、レビューするときに使用します。テスト配置、TEST_SRCS、main のラップ、Google Mock、テスト フェーズ、エビデンス コメントを扱います。
---

# app 単体テストの作成

1. `framework/testfw/docs/how-to-test.md` を読んでください。
2. `framework/testfw/docs/about-test-phase.md` と `framework/testfw/docs/how-to-expect.md` を読んでください。
3. app 向けの配置例は `app/general/docs/testing-tutorial.md` を確認してください。
4. 対象 app の AGENTS.md と近いテストを確認してください。
5. テスト対象ソース、mock、`TEST_SRCS`、`ADD_SRCS`、`LIBS` を決定してください。
6. Arrange、Pre-Assert、Act、Assert と、必要な場合だけ Cleanup を記載してください。
7. 対象 app の局所テストを実行し、結果とカバレッジを確認してください。

mock を新設する場合は、対象に応じた mock 作成スキルも使用してください。
