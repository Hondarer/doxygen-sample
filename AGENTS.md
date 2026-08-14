# AGENTS.md

## 最優先の制約

- ユーザーから個別に指示されるまで、ステージング、コミット、アンステージ、スタッシュを行わないでください。
- Git の書き込み操作に対する許可は、その指示に限る単発の許可として扱ってください。
- ユーザーへの確認、気付き、作業結果は日本語で報告してください。
- 対話では、意味が曖昧な作業用語を避け、対象と操作を具体的に記載してください。

## 作業時の参照順

作業を始める前に、次の順序で文書を確認してください。

1. この `AGENTS.md`
2. 対象が `framework/<name>/` またはサブモジュール配下なら、その Git ルートの `AGENTS.md` と `README.md`
3. 対象が `app/<name>/` 配下なら、その app 直下の `AGENTS.md` と `README.md`
4. 作業内容に該当する `SKILL.md`
5. AGENTS.md または SKILL.md が示す正本ドキュメント

複数の framework や app を変更する場合は、対象ごとに同じ確認を行ってください。  
下位の AGENTS.md は対象固有の規則だけを追加し、上位文書の規則を解除しません。

## 文書とスキルの役割

- AGENTS.md は、作業開始時の入口、必須参照先、対象固有の変更トリガーを示します。
- README.md は、人間向けにディレクトリの目的、主要入口、利用方法を示します。
- 詳細な規範、設計、手順は通常のドキュメントを唯一の正本とします。
- SKILL.md は発火条件と作業手順の要点だけを持ち、詳細は正本ドキュメントを参照します。
- framework や `app/general` などの汎用層から、実在する個別 app の実装を規範として参照しないでください。

詳細は [AGENTS とスキルの設計指針](app/general/docs/agents-and-skills-guideline.md) を参照してください。

## リポジトリ概要

このリポジトリは、レガシ C コードのモダナイゼーションを題材に、ビルド、テスト、Doxygen、Markdown 発行、.NET 呼び出し例をまとめた統合ワークスペースです。

- `framework/makefw` - C/C++ と .NET 向けの Make テンプレート
- `framework/testfw` - Google Test ベースのテスト フレームワーク
- `framework/doxyfw` - Doxygen と Doxybook2 による生成フレームワーク
- `framework/docsfw` - Pandoc ベースの Markdown 発行フレームワーク
- `app/<name>` - ライブラリ、コマンド、サンプル、ワークスペース共通文書

ワークスペース固有の作業手順は [ワークスペース作業ガイド](app/c-modernization-kit/docs/workspace-agent-workflow.md) を参照してください。

## 主要な正本

- C/C++ の規範: [コーディング規範](app/general/docs/coding-guideline.md)
- テスト構成: [テスト方法](framework/testfw/docs/how-to-test.md)
- テスト フェーズとエビデンス: [テスト フェーズ](framework/testfw/docs/about-test-phase.md)
- make ファイル断片: [makeparts](framework/makefw/docs/makeparts.md)
- VS Code と app 環境変数: [VS Code 環境変数](app/general/docs/vscode-variables.md)
- Markdown の機械整形: [text_style_jp](framework/docsfw/bin/text_style_jp.md)
- 日本語本文の規範: [日本語技術文書の文章規範](framework/docsfw/docs/japanese-technical-writing-guideline.md)

## 変更後の確認

- C/C++ の変更は、対象 app の AGENTS.md とコーディング規範が指定する局所テストを実行してください。
- framework または複数 app にまたがる全体テストは、コストを説明してからユーザーに確認してください。
- ビルド後は、対象範囲の内容がある `.warn` ファイルを確認してください。
- Markdown の変更は、対象ファイルごとに `text_style_jp.py --dry-run` を先に実行し、問題がなければ `--in-place` を実行してください。
- `text_style_jp.py` が付与する行末の半角空白 2 個は Markdown の強制改行であり、`git diff --check` の指摘だけを理由に削除しないでください。

## 共通の実装条件

- 管理対象の末端 make ファイルは小文字の `makefile` とします。
- `app/<name>` の C ライブラリは、公開ヘッダーを `prod/include/`、内部共有ヘッダーを `prod/include_internal/`、ソースを `prod/libsrc/` に配置します。
- Linux/GCC と Windows/MSVC の双方で動作する構成を維持してください。
- 非自明な OS 仕様や回避策をコードへ反映する場合は、根拠 URL を `see: <URL>` 形式でソース内に残してください。
