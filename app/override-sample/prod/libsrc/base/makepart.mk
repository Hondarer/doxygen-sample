# ライブラリの指定
LIBS += cplat

ifdef PLATFORM_WINDOWS
    # DLL エクスポート定義
    # DLL export definition
    CFLAGS   += /DBASE_EXPORTS
    CXXFLAGS += /DBASE_EXPORTS
endif
