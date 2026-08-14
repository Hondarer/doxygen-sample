# AGENTS.md

## 対象

この app は、Linux のデーモンと Windows サービスを共通構成で実装するサンプルです。

## 必須参照

- [README.md](README.md)
- [発行ドキュメント](docs/README.md)
- [コーディング規範](../general/docs/coding-guideline.md)

## 注意点

- サービス制御と終了処理は、Linux と Windows の双方で確認してください。
- `make test` を局所確認に使用してください。
