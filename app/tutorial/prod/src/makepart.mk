# ライブラリの指定 (static library を利用)
# 単体で配布・実行できるチュートリアル用の実行可能ファイルであり、
# 他の com_util 利用共有ライブラリをロードしないため、静的リンクとする。
# see: app/com_util/docs/link-policy.md
LIBS += com_util_static
ifdef PLATFORM_WINDOWS
    CFLAGS   += /DCOM_UTIL_STATIC
    CXXFLAGS += /DCOM_UTIL_STATIC
    # libcom_util は both 生成で、static 側にも dllexport 付きシンボルを含む。
    # そのまま exe をリンクすると .exp と import lib も生成されるため、抑止する。
    LDFLAGS  += /NOEXP /NOIMPLIB
endif
