---
name: maintain-doxygen-comment
description: app 配下の C または C++ ヘッダーやソースへ Doxygen コメントを追加、変更、レビューするときに使用します。コメントの配置、タグ、方向指定、スレッド安全性、生成確認を扱います。
---

# Doxygen コメントの保守

1. `app/general/docs/doxygen-comment-guideline.md` を読んでください。
2. タグ仕様は `framework/doxyfw/docs/commands.md`、雛形は `framework/doxyfw/docs/cheatsheet.md` を確認してください。
3. 対象パスに適用される `AGENTS.md` と公開ヘッダーを確認してください。
4. 宣言側を正本とし、定義側へ同じ説明を複製しないでください。
5. 公開関数の定義ごと、直前にマーカー `/* Doxygen コメントは、ヘッダーに記載 */` を置いてください。
6. 引数方向、NULL、所有権、戻り値、スレッド安全性を実装と一致させてください。
7. ソース整形後にコメントの字下げを確認してください。
8. 対象 app の `make doxy` を実行し、警告と生成結果を確認してください。

生成済み Doxybook2 Markdown は直接編集しないでください。
