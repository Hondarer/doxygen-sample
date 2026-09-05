# 配布ソースは生成物であり、直接編集・整形しない。
LIB_TYPE = shared
ifdef PLATFORM_WINDOWS
    # zutil.h / gzguts.h が ZLIB_INTERNAL を定義する。
    # see: https://github.com/madler/zlib/blob/v1.3.2/zconf.h
    DEFINES += ZLIB_DLL
else ifdef PLATFORM_LINUX
    DEFINES += ZLIB_API_VISIBILITY
    # upstream の構造体配置・整数変換・switch に由来する警告だけを抑制する。
    CFLAGS += -fvisibility=hidden -Wno-padded -Wno-cast-qual -Wno-conversion -Wno-sign-conversion -Wno-switch-default -Wno-switch-enum
endif
