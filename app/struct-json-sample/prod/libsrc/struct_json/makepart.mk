# ライブラリの指定
LIBS += com_util cjson

ifdef PLATFORM_WINDOWS
    # DLL エクスポート定義
    CFLAGS   += /DSJ_EXPORTS
    CXXFLAGS += /DSJ_EXPORTS
endif

# 生成されるライブラリを動的ライブラリ (shared) とする
# 未指定の場合 (デフォルト) は static
LIB_TYPE = shared
