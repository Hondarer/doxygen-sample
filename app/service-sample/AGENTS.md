# AGENTS.md

## 対象

この app は、Linux のデーモンと Windows サービスを共通構成で実装するサンプルです。

## 参照先

作業に関係する文書の該当節を参照してください。

- [README.md](README.md)
- [発行ドキュメント](docs/README.md)
- [コーディング規範](../general/docs/coding-guideline.md)

## 注意点

- サービス制御と終了処理は、Linux と Windows の双方で確認してください。
- 振る舞いやビルド構成を変更した場合は、影響する局所テストを実行してください。app 全体の確認には `make test` を使用できます。
