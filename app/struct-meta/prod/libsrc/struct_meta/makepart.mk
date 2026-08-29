# ライブラリの指定
LIBS += cplat cjson

# 責務別のサブディレクトリに置いた実装を、1 個の共有ライブラリへまとめる。
ADD_SRCS += \
    meta/validate.c \
    access/access.c \
    access/path.c \
    json/encode.c \
    json/decode.c \
    json/file.c \
    patch/patch.c \
    print/print.c

ifdef PLATFORM_WINDOWS
    # DLL エクスポート定義
    CFLAGS   += /DSTRUCT_META_EXPORTS
    CXXFLAGS += /DSTRUCT_META_EXPORTS
endif

# 生成されるライブラリを動的ライブラリ (shared) とする
# 未指定の場合 (デフォルト) は static
LIB_TYPE = shared
