# AGENTS.md

## 対象

この app は、C ライブラリ、コマンド、単体テストを含む基本的な計算サンプルです。

## 必須参照

- [README.md](README.md)
- [発行ドキュメント](docs/README.md)
- [コーディング規範](../general/docs/coding-guideline.md)
- [テスト方法](../../framework/testfw/docs/how-to-test.md)

## 注意点

- `prod/include/` の公開 API を変更した場合は、Doxygen コメントと公開 API テストを確認してください。
- テストの配置、フェーズ、エビデンス コメントは testfw の正本に従ってください。
- `make test` を局所確認に使用してください。
