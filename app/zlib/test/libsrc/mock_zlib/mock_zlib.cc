#include <mock_zlib.h>

#include <cstdio>
#include <type_traits>

Mock_zlib *mock_zlib_instance = nullptr;

namespace
{

template <typename T> void trace_result(const char *func, const T value)
{
    if (getTraceLevel() <= TRACE_NONE)
    {
        return;
    }

    std::printf("  > %s", func);
    if (getTraceLevel() >= TRACE_DETAIL)
    {
        if constexpr (std::is_pointer_v<T>)
        {
            std::printf(" -> 0x%p\n", (const void *)value);
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            std::printf(" -> %f\n", (double)value);
        }
        else
        {
            std::printf(" -> %lld\n", (long long)value);
        }
    }
    else
    {
        std::printf("\n");
    }
}

void trace_void(const char *func)
{
    if (getTraceLevel() > TRACE_NONE)
    {
        std::printf("  > %s\n", func);
    }
}

} // namespace

#ifndef _WIN32
    #define MOCK_ZLIB_IMPL(return_type, name, ...) MOCK_WEAK_IMPL(return_type, name, __VA_ARGS__)
#else
    #define MOCK_ZLIB_IMPL(return_type, name, ...) extern "C" return_type ZEXPORT name(__VA_ARGS__)
#endif

#define MOCK_ZLIB_EXPAND(...) __VA_ARGS__

#define MOCK_ZLIB_RET(return_type, name, parameters, arguments, matchers) \
    return_type delegate_real_##name parameters \
    { \
        static auto real_fn = reinterpret_cast<decltype(&name)>(resolveSharedSymbolOrExit(kLibZlibName, #name)); \
        return real_fn arguments; \
    } \
    MOCK_ZLIB_IMPL(return_type, name, MOCK_ZLIB_EXPAND parameters) \
    { \
        return_type return_value; \
        if (mock_zlib_instance != nullptr) \
        { \
            return_value = mock_zlib_instance->name arguments; \
        } \
        else \
        { \
            return_value = delegate_real_##name arguments; \
        } \
        trace_result(__func__, return_value); \
        return return_value; \
    }

#define MOCK_ZLIB_VOID(return_type, name, parameters, arguments, matchers) \
    return_type delegate_real_##name parameters \
    { \
        static auto real_fn = reinterpret_cast<decltype(&name)>(resolveSharedSymbolOrExit(kLibZlibName, #name)); \
        real_fn arguments; \
    } \
    MOCK_ZLIB_IMPL(return_type, name, MOCK_ZLIB_EXPAND parameters) \
    { \
        if (mock_zlib_instance != nullptr) \
        { \
            mock_zlib_instance->name arguments; \
        } \
        else \
        { \
            delegate_real_##name arguments; \
        } \
        trace_void(__func__); \
    }

#include <mock_zlib_api_table.h>

#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET
#undef MOCK_ZLIB_EXPAND
#undef MOCK_ZLIB_IMPL

Mock_zlib::Mock_zlib()
{
#define MOCK_ZLIB_RET(return_type, name, parameters, arguments, matchers) \
    ON_CALL(*this, name matchers).WillByDefault(Invoke(delegate_real_##name));
#define MOCK_ZLIB_VOID(return_type, name, parameters, arguments, matchers) \
    ON_CALL(*this, name matchers).WillByDefault(Invoke(delegate_real_##name));
#include <mock_zlib_api_table.h>
#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET

    TESTFW_REGISTER_MOCK_INSTANCE(mock_zlib_instance);
}

Mock_zlib::~Mock_zlib()
{
    TESTFW_UNREGISTER_MOCK_INSTANCE(mock_zlib_instance);
}
