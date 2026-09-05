#ifndef MOCK_ZLIB_H
#define MOCK_ZLIB_H

#include <zlib.h>
#include <testfw.h>

// 関数形式マクロは関数定義と MOCK_METHOD の展開を妨げるため解除する。
#undef gzgetc

// Windows でも実ライブラリが公開する 64-bit API を宣言する。
// see: https://github.com/madler/zlib/blob/v1.3.2/gzguts.h
// see: https://github.com/madler/zlib/blob/v1.3.2/zutil.h
#ifndef Z_LARGE64
extern "C"
{
    ZEXTERN gzFile ZEXPORT gzopen64(const char *, const char *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, int);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
    ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
    ZEXTERN uLong ZEXPORT crc32_combine64(uLong, uLong, z_off64_t);
    ZEXTERN uLong ZEXPORT crc32_combine_gen64(z_off64_t);
}
#endif

inline constexpr char kLibZlibName[] = "libzlib" TESTFW_SHARED_LIBRARY_EXTENSION;

#define MOCK_ZLIB_RET(return_type, name, parameters, arguments, matchers) \
    extern return_type delegate_real_##name parameters;
#define MOCK_ZLIB_VOID(return_type, name, parameters, arguments, matchers) \
    extern return_type delegate_real_##name parameters;
#include <mock_zlib_api_table.h>
#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET

class Mock_zlib
{
  public:
#define MOCK_ZLIB_RET(return_type, name, parameters, arguments, matchers)  MOCK_METHOD(return_type, name, parameters);
#define MOCK_ZLIB_VOID(return_type, name, parameters, arguments, matchers) MOCK_METHOD(return_type, name, parameters);
#include <mock_zlib_api_table.h>
#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET

    Mock_zlib();
    ~Mock_zlib();
};

extern Mock_zlib *mock_zlib_instance;

#endif /* MOCK_ZLIB_H */
