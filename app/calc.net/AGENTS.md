# AGENTS.md

## 対象

この app は、C ライブラリを .NET から利用するラッパーとサンプルです。

## 必須参照

- [README.md](README.md)
- [発行ドキュメント](docs/README.md)
- [.NET ビルド構成](../general/docs/dotnet-relwithdebinfo.md)
- [.NET テスト結果設計](../general/docs/dotnet-test-results-design.md)

## 注意点

- P/Invoke 宣言を変更した場合は、対応する C 側の公開ヘッダーと ABI を確認してください。
- Linux と Windows の双方でライブラリ名と呼び出し規約が一致することを確認してください。
- `make test` を局所確認に使用してください。
