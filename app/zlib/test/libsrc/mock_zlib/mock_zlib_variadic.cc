#include <mock_zlib.h>

#include <cstdarg>

// gzprintf の可変長引数を、API 表の gzvprintf に集約する。
// see: https://github.com/madler/zlib/blob/v1.3.2/zlib.h
#ifndef _WIN32
MOCK_WEAK_IMPL(int, gzprintf, gzFile file, const char *format, ...)
#else
extern "C" int ZEXPORTVA gzprintf(gzFile file, const char *format, ...)
#endif
{
    va_list args;
    va_start(args, format);
    const int result = gzvprintf(file, format, args);
    va_end(args);
    return result;
}
