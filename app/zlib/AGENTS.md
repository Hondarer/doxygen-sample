# AGENTS.md

## 対象

この app は zlib の公式配布アーカイブを makefw へ取り込むラッパーと、利用側の単体テスト用 API モックを管理します。

## 必須参照

- [README.md](README.md)
- 展開と更新では [packages/README.md](packages/README.md)
- 本体への変更では [patches/README.md](patches/README.md)

## 注意点

- 展開された zlib 本体は直接編集しません。
- モックの利用側は `mock_zlib` だけをリンクし、`zlib` と同時リンクしません。
- Windows のモック本体と利用側では `ZLIB_DLL` を定義しません。
- testfw の同名モックとは API と ABI が異なります。README の探索順と利用条件を守ります。
- API 表を変更した場合は公開関数の網羅テストを含む `make test` を実行します。
- 展開スクリプトを変更した場合は `python3 bin/test_extract_package.py` を実行します。
