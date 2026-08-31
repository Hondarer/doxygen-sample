# 実行時にヘッダーを解析する経路を、公開 API のまま端から端まで確かめる。
# 構文解析器 (flex/bison の生成物) を含むため、テスト実行体へ製品ソースを
# 引き込まず、共有ライブラリをそのままリンクする。
# 記述子の組み立てそのものは structMetaBuildTest が対象とする。
LIBS += cplat struct_meta
