# AGENTS.md

## 対象

この app は、共有ライブラリのシンボルを別実装で上書きする構成のサンプルです。

## 参照先

作業に関係する文書の該当節を参照してください。

- [README.md](README.md)
- [Doxygen 入力](prod/README.md)
- [発行ドキュメント](docs/README.md)
- [共有ライブラリと静的リンク](../../framework/testfw/docs/about-shared-lib-static-linking.md)

## 注意点

- Linux と Windows でシンボル解決方法が異なるため、両プラットフォームの構成を確認してください。
- サンプル固有の手法を、汎用 framework の必須規則として扱わないでください。
