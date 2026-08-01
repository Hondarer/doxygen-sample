# デモ用 app の構成

## 資料の位置付け

この資料は、GitHub リポジトリ `c-modernization-kit` に配置した app の役割と依存関係を説明します。
ここで示す組み合わせはデモ用であり、フレームワークを別のワークスペースへ展開するときの必須構成ではありません。

`app/general` は標準配布する一般資料です。
それ以外の app は目的に応じて選択でき、この資料を含む `app/c-modernization-kit` も削除できます。

## app の役割

| app | 役割 |
|---|---|
| `general` | フレームワークの一般規範、設計、運用手順 |
| `c-modernization-kit` | このリポジトリの app 構成と CI/CD の説明 |
| `com_util` | 複数の C app が利用する共通ユーティリティ ライブラリ |
| `calc` | 静的ライブラリ、共有ライブラリ、コマンド、Google Test の組み合わせ例 |
| `calc.net` | `calc` の共有ライブラリを P/Invoke で利用する .NET 実装例 |
| `porter` | `com_util` を利用する通信ライブラリ |
| `cjson`、`lua`、`sqlite` | 外部 OSS を makefw の規約でビルドする例 |
| `doxygen-sample` | Doxygen 文書生成の例 |
| `empty-lib` | ソースが空のライブラリを扱う例 |
| `override-sample` | ライブラリ関数を差し替える例 |
| `service-sample` | Linux のデーモンと Windows サービスの実装例 |
| `subfolder-sample` | サブディレクトリを再帰的にコンパイルする例 |
| `tutorial` | 小さなコマンドを段階的に追加する例 |

## app 間の依存関係

各 app の `appdeps.mk` がビルド順序の正本です。
現在の依存関係は次のとおりです。

- `calc`、`porter`、`empty-lib`、`override-sample`、`service-sample`、`subfolder-sample`、`tutorial` は `com_util` に依存します。
- `calc.net` は `calc` に依存し、推移的に `com_util` も利用します。
- `cjson`、`lua`、`sqlite`、`doxygen-sample` は、ほかの app に依存しません。
- `general` と `c-modernization-kit` は文書だけを持つため、ビルド依存関係を定義しません。

makefw はこの依存グラフから app の実行順序を決定します。
依存関係のない app は並列にビルドできます。

## calc と calc.net の構成

`calc` は、基本計算関数を持つ静的ライブラリと、計算処理の公開 API を持つ共有ライブラリを組み合わせます。
コマンドとテストは、静的リンク、動的リンク、両者の併用を比較できる構成です。

`calc.net` は `calc` の共有ライブラリを P/Invoke で呼び出します。
Linux と Windows で同じ .NET API を提供し、ネイティブ ライブラリの探索パスはワークスペースの生成設定から取得します。

## com_util のリンク方式

`com_util` は `LIB_TYPE = both` で共有ライブラリと静的ライブラリの両方を生成します。
利用側がどちらを選ぶかは、[リンク方式の規約](../../com_util/docs/link-policy.md) に従います。

静的リンクが許されるのは、ほかの `com_util` 利用共有ライブラリをロードしない末端の実行可能ファイルだけです。
同一プロセス内に静的版と動的版が同居すると、トレース レジストリや既定パーサーなどのプロセス グローバルな状態が複数生成され、コンパイル時にもリンク時にも検出されない不具合を招きます。

本リポジトリでの適用状況は次のとおりです。

| 対象 | リンク方式 | 理由 |
|---|---|---|
| `app/calc`、`app/porter`、`app/override-sample`、`app/subfolder-sample`、`app/empty-lib` | 動的 | 共有ライブラリであり、状態を一つに保つ必要があるため |
| `app/com_util/prod/src/cmd/` | 静的 | `com_util` に同梱する CLI ツール群を、ライブラリ探索パスの設定なしで実行可能にするため |
| `app/service-sample/prod/src/cmd/` | 静的 | systemd または Windows SCM から起動するため。systemd はシェル環境の `LD_LIBRARY_PATH` を継承しない |
| `app/tutorial/prod/src/` | 静的 | チュートリアル用の実行可能ファイルを単体で配布できるようにするため |

静的リンクしている実行可能ファイルへプラグイン機構や `dlopen`、`LoadLibrary` を追加する場合は、動的リンクへ変更する必要があります。

## 関連資料

- [クロスプラットフォーム ビルド システムの実装](../../general/docs/build-design.md)
- [GitHub Actions CI/CD 仕様](github-actions.md)
- [VS Code における環境変数と c_cpp_properties.json の保守手順](../../general/docs/vscode-variables.md)
