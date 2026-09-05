# AGENTS.md

## 最優先の制約

- ユーザーから個別に指示されるまで、ステージング、コミット、アンステージ、スタッシュを行わないでください。
- Git の書き込み操作に対する許可は、その指示に限る単発の許可として扱ってください。
- ユーザーへの確認、気付き、作業結果は日本語で報告してください。
- 対話では、意味が曖昧な作業用語を避け、対象と操作を具体的に記載してください。

## AGENTS.md の適用範囲

`AGENTS.md` は、それが置かれたディレクトリと、その配下すべてに適用します。

作業対象のパスを、リポジトリ ルートから対象ディレクトリまで順にたどってください。  
途中のディレクトリに `AGENTS.md` があれば、上位からの差分として、ルート側から順に重ねて適用します。

- 適用の条件は、階層上に `AGENTS.md` が存在することだけです。そのディレクトリがサブモジュールの Git ルートかどうかは条件に含めません。
- 階層の深さに上限はありません。`app/<name>/` や `framework/<name>/` に限らず、より深い階層の `AGENTS.md` も同じ規則で適用します。
- 複数の対象を変更する場合は、対象ごとに同じ走査を行ってください。

例として `framework/testfw/gtest/lib/` を変更する場合は、`AGENTS.md`、`framework/testfw/AGENTS.md`、`framework/testfw/gtest/AGENTS.md` を、この順に重ねて適用します。

サブモジュール境界は、Git のコミット単位を分けるためのものです。  
`AGENTS.md` の適用範囲には影響しません。

`CLAUDE.md` は `AGENTS.md` への転送であり、規則の正本ではありません。  
`CLAUDE.md` の有無にかかわらず、階層上の `AGENTS.md` を適用してください。

### 探索の方法

`AGENTS.md` の有無は、戻り値が非 0 にならない方法で確認してください。  
`ls path/AGENTS.md` や `test -f path/AGENTS.md` を単体で実行すると、ファイルが無い場合に戻り値が非 0 となり、正常な走査であるにもかかわらずエラー表示を伴います。

対象パス上の `AGENTS.md` は、次のようにまとめて列挙します。  
一致が無くても戻り値は 0 です。

```bash
target=framework/testfw/gtest/lib
dir=.
if [ -f "$dir/AGENTS.md" ]; then echo "$dir/AGENTS.md"; fi
for part in $(echo "$target" | tr '/' ' '); do
    dir="$dir/$part"
    if [ -f "$dir/AGENTS.md" ]; then echo "$dir/AGENTS.md"; fi
done
```

リポジトリ全体を見る場合は `find . -name AGENTS.md | sort` を使用します。  
個別に確認する場合は、`ls -d path/AGENTS.md 2>/dev/null || true` のように、戻り値を 0 に正規化してください。

### 差分の解釈

- 下位の `AGENTS.md` は、上位に無い規則を追加し、上位の規則を対象固有に具体化します。
- 既定では、下位は上位の規則を解除しません。
- 上位の規則を解除または上書きする場合は、下位の `AGENTS.md` に解除する規則と理由を明示してください。明示がある項目に限り、下位の記載を優先します。
- 明示のない相違は、上位の規則が有効なままです。

## 作業時の参照順

作業を始める前に、次の順序で文書を確認してください。

1. 適用範囲の規則で決まる `AGENTS.md` を、ルート側から順に
2. 各 `AGENTS.md` と同じディレクトリの `README.md`
3. 作業内容に該当する `SKILL.md`
4. `AGENTS.md` または `SKILL.md` が示す正本ドキュメント

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
- `framework/docsfw` - Markdown 発行フレームワーク (Pandoc による静的発行と mkdocs による動的発行)
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

- C/C++ の変更は、対象パスに適用される `AGENTS.md` とコーディング規範が指定する局所テストを実行してください。
- framework または複数 app にまたがる全体テストは、コストを説明してからユーザーに確認してください。
- ビルド後は、対象範囲の内容がある `.warn` ファイルを確認してください。
- Markdown の変更は、対象ファイルごとに `text_style_jp.py --dry-run` を先に実行し、問題がなければ `--in-place` を実行してください。
- `text_style_jp.py` が付与する行末の半角空白 2 個は Markdown の強制改行であり、`git diff --check` の指摘だけを理由に削除しないでください。

## 共通の実装条件

- 管理対象の末端 make ファイルは小文字の `makefile` とします。
- `app/<name>` の C ライブラリは、公開ヘッダーを `prod/include/`、内部共有ヘッダーを `prod/include_internal/`、ソースを `prod/libsrc/` に配置します。
- ライブラリのソースを責務別サブディレクトリへ分ける場合は、各サブディレクトリに makefw のテンプレート `makefile` を置くサブディレクトリ走査方式を使います。`prod/libsrc/` 配下の `makepart.mk` で `ADD_SRCS` へ相対パスを列挙しないでください。`ADD_SRCS` はディレクトリ外のソースを引き込む機能であり、ライブラリ ルート直下へシンボリック リンクを作ります。詳細は [サブフォルダー コンパイル](framework/makefw/docs/subfolder-compilation.md) を参照してください。
- Linux/GCC と Windows/MSVC の双方で動作する構成を維持してください。
- 非自明な OS 仕様や回避策をコードへ反映する場合は、根拠 URL を `see: <URL>` 形式でソース内に残してください。
