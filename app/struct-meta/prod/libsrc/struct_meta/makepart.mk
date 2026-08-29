# ライブラリの指定
LIBS += cplat cjson

# 責務別のサブディレクトリに置いた実装を、1 個の共有ライブラリへまとめる。
ADD_SRCS += \
    meta/index.c \
    meta/integer.c \
    meta/validate.c \
    access/access.c \
    access/path.c \
    json/encode.c \
    json/decode.c \
    json/file.c \
    patch/patch.c \
    print/print.c

# json のモジュール私有ヘッダーを参照する。
# ADD_SRCS のソースは引き込み先で構築するため、引用符形式の探索起点が元ディレクトリに
# ならない。see: app/general/docs/coding-guideline.md の「モジュール私有ヘッダー」
INCDIR += \
    $(MYAPP_DIR)/prod/libsrc/struct_meta/json

ifdef PLATFORM_WINDOWS
    # DLL エクスポート定義
    CFLAGS   += /DSTRUCT_META_EXPORTS
    CXXFLAGS += /DSTRUCT_META_EXPORTS
endif

# 生成されるライブラリを動的ライブラリ (shared) とする
# 未指定の場合 (デフォルト) は static
LIB_TYPE = shared
