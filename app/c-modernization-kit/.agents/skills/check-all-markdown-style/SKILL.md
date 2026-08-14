---
name: check-all-markdown-style
description: リポジトリ内の第一者管理 Markdown 全体へ text_style_jp.py を適用するときに使用します。Git 状態、動的な対象列挙、外部 OSS と生成物の除外、dry-run、差分確認、in-place 適用を扱います。
---

# Markdown 全体のスタイル確認

1. `app/c-modernization-kit/docs/markdown-style-bulk-check.md` を読んでください。
2. ルートと全サブモジュールの Git 状態がクリーンであることを確認してください。
3. 各 Git ルートの tracked Markdown を動的に列挙してください。
4. 外部 OSS と自動生成物を除外してください。
5. 全対象へ `text_style_jp.py --dry-run` を実行してください。
6. 不自然な変換がないことを確認してください。
7. 問題がなく、変更が許可されている場合だけ `--in-place` を実行してください。
8. 各 Git ルートの差分を読み、Markdown 構造と日本語を確認してください。

ステージング、コミット、アンステージを行わないでください。
