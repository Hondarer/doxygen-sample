ifdef PLATFORM_WINDOWS
    # 製品ソースをテスト実行体へ直接定義する。
    # 製品ライブラリのリンク方式は変更しない。
    CFLAGS   += /DSJ_STATIC
    CXXFLAGS += /DSJ_STATIC
endif
