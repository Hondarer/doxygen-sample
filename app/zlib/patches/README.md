# zlib へのパッチ

展開後に makefw の `apply_patches.py` で番号順に適用します。  
展開されたファイルを直接編集せず、変更を unified diff として管理します。

## 0001-gcc-public-visibility.patch

Linux の共有ライブラリを `-fvisibility=hidden` で構築し、`ZLIB_API_VISIBILITY` 定義時に `ZEXTERN` で宣言した公開関数だけを公開します。  
内部関数と内部データの流出を防ぎ、公開関数と API モックの網羅性を照合できるようにします。  
圧縮アルゴリズム、型、関数シグネチャ、Windows の DLL 宣言は変更しません。  
パッチは `prod/include/zconf.h` に適用します。
