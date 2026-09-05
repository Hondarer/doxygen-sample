---
name: create-unit-test
description: app 配下の C/C++ 単体テストを作成・変更・レビューする際に、testfw 固有の配置とエビデンス規則を適用します。
---

# app 単体テストの作成

作業に応じて次の節を参照してください。

- 配置やビルド対象を変える場合は `framework/testfw/docs/how-to-test.md` の `TEST_SRCS`、`ADD_SRCS`、main の扱いと、必要なら `app/general/docs/testing-tutorial.md` の配置例
- フェーズやエビデンス コメントを変える場合は `framework/testfw/docs/about-test-phase.md` の該当するテスト形式
- 期待値や照合方法を変える場合は `framework/testfw/docs/how-to-expect.md` の該当するマクロ
- mock を新設する場合は、対象パスの指示にある専用スキル、または通常の app 関数向け `create-mock`

既存の近いテストと対象の契約を確認し、要求された振る舞いを検証してください。  
変更後は影響する局所テストを実行し、網羅性の判断に必要ならカバレッジを確認してください。  
レビューだけの場合は、指摘を裏付ける確認を選び、テストの追加や全面的な再実行を前提にしないでください。
