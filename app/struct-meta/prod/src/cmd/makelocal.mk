# struct-meta-gen を struct-meta-sample より
# 先にビルドする必要があるため、ワイルドカード自動検出 (兄弟を独立とみなし並列化) では
# なく、宣言順を並列ビルド下でも維持する明示指定を用いる。
# see: framework/makefw/docs/makeparts.md
SUBDIRS := \
	struct-meta-gen \
	struct-meta-sample
