# zlib

作業前に [作業規則](AGENTS.md) を確認してください。

このディレクトリは c-modernization-kit の `app/zlib` として、[zlib](https://zlib.net/) を makefw でビルドするためのラッパーを提供します。  
通常ディレクトリとして管理し、ビルドにはワークスペース内の makefw と testfw を使用します。

## 提供する機能

zlib 1.3.2 の圧縮・展開、チェックサム、gzip ファイル API を取り込みます。  
`contrib/minizip` と公式サンプル コマンドは取り込みません。

| 環境 | 共有ライブラリ | リンク指定 |
|---|---|---|
| Linux/GCC | `prod/lib/libzlib.so` | `LIBS += zlib` |
| Windows/MSVC | `prod/lib/libzlib.dll` と import library `libzlib.lib` | `LIBS += zlib` |

静的な実ライブラリは生成しません。  
OS にインストールされた `libz` や `zlib1.dll` の置き換えは行いません。  
公開 API と型は upstream の `<zlib.h>` と `<zconf.h>` を使います。  
Windows の DLL 本体にだけ `ZLIB_DLL` を指定します。  
利用側では未定義のまま import library でリンクでき、`ZLIB_WINAPI` は定義しません。

ほかの app から利用する場合は、その app の `appdeps.mk` に `APP_DEPS += zlib` を記載し、対象の `makepart.mk` にリンク指定を追加します。  
公開ヘッダーは system include として提供し、外来ヘッダーの警告を利用側から分離します。

## ビルドと更新

```sh
make -C app/zlib
make -C app/zlib test
python3 app/zlib/bin/test_extract_package.py
```

`packages/` の公式アーカイブから、公開ヘッダーを `prod/include/`、本体と内部ヘッダーを `prod/libsrc/zlib/` に自動展開します。  
展開物は Git 管理対象外です。  
本体への変更は [パッチ](patches/README.md) に記録します。

アーカイブ、展開スクリプト、パッチが変更された場合と、展開物が欠落した場合は自動的に再展開します。  
内容が同一のファイルは更新しません。  
パッケージの取得元、ハッシュ、更新手順は [packages/README.md](packages/README.md) を参照してください。

## 利用サンプル

`prod/cbin/zlib_sample` (Windows では `.exe`) は、固定データをメモリ上で圧縮・展開し、元データと比較します。  
一致した場合はバージョンと処理前後のサイズを表示して 0、失敗した場合は非 0 を返します。  
実行時の探索パスはワークスペースの環境設定を使用します。  
app の追加後は `make sync-app-env` で更新します。

## API モック

単体テストでは `<mock_zlib.h>` と `LIBS += mock_zlib` を使用します。  
モックを生成しない場合と、個別の動作を設定していない場合は、`libzlib` の実関数へ委譲します。  
実ライブラリのロードとシンボル解決に失敗すると、理由を標準エラーへ出力して終了します。

```cpp
NiceMock<Mock_zlib> mock_zlib;
EXPECT_CALL(mock_zlib, compress(_, _, _, _)).WillOnce(Return(Z_MEM_ERROR));
```

実ライブラリとモックを同時にリンクしないでください。  
Windows ではモック本体と利用側の `ZLIB_DLL` を未定義にします。  
実委譲用ライブラリは Linux の `LD_LIBRARY_PATH`、Windows の `PATH` から読み込みます。  
同時に生成できる `Mock_zlib` は 1 個であり、生成・破棄・呼び出しを複数スレッドで並行実行しません。  
完全隔離では、対象が呼ぶ関数すべてに動作を設定し、偽のストリームやファイル ハンドルを実関数へ渡さないでください。

### マクロと OS 差

| 呼び出し | モック対象 |
|---|---|
| `deflateInit`、`deflateInit2` | `deflateInit_`、`deflateInit2_` |
| `inflateInit`、`inflateInit2`、`inflateBackInit` | 末尾に `_` が付く実関数 |
| `gzprintf` | `gzvprintf` (書式文字列と `va_list`) |
| `gzgetc` | 実関数 `gzgetc`。モック ヘッダー内で同名マクロを解除 |

テスト対象ソースが通常の `<zlib.h>` でコンパイルされた場合、`gzgetc` マクロは公開構造体を直接読み書きすることがあります。  
関数モックだけでその処理全体を差し替えることはできません。  
関数として呼ぶ必要がある場合は `(gzgetc)(file)` と記述します。

Linux では `_LARGEFILE64_SOURCE=1` を指定し、64-bit offset API を含めて検証します。  
Windows のモックでは、本体がエクスポートする同 API の宣言を補います。  
Windows 専用の `gzopen_w` は Windows の API 表と網羅テストだけに含めます。  
`Z_PREFIX`、`Z_SOLO`、呼び出し規約の変更はこの構成の対象外です。

## テスト

- `zlibTest`: メモリの往復、空・バイナリ、不正入力、バッファー不足、チェックサム、gzip ファイル、size_t 版 API
- `sampleTest`: サンプルの成功、圧縮・展開エラー、復元内容不一致
- `mockZlibTest`: 実委譲、既定動作、エラー差し替え、初期化マクロ、可変長引数
- `exportTest`: API 表、IDENT manifest、実ライブラリの全エクスポートの一致、関数シグネチャの一致

## ライセンス

zlib 本体のライセンスはアーカイブ内の `LICENSE` が正本です。  
展開後は `prod/libsrc/zlib/LICENSE` でも参照できます。  
本体の著作権表示とライセンス条件を維持してください。  
app 直下の `LICENSE` は手書きのラッパー部分に適用します。
