#include <testfw.h>

#include <mock_zlib.h>

#include <set>
#include <string>
#include <type_traits>

#define MOCK_ZLIB_RET(return_type, name, parameters, arguments, matchers) \
    using expected_##name##_fn = return_type(*) parameters; \
    static_assert(std::is_same<decltype(&name), expected_##name##_fn>::value, #name " のシグネチャが不一致です");
#define MOCK_ZLIB_VOID(return_type, name, parameters, arguments, matchers) \
    using expected_##name##_fn = return_type(*) parameters; \
    static_assert(std::is_same<decltype(&name), expected_##name##_fn>::value, #name " のシグネチャが不一致です");
#include <mock_zlib_api_table.h>
#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET

using expected_gzprintf_fn = int (*)(gzFile, const char *, ...);
static_assert(std::is_same<decltype(&gzprintf), expected_gzprintf_fn>::value, "gzprintf signature mismatch");

static const char *const kExpectedExportNames[] = {
#define MOCK_ZLIB_RET(return_type, name, parameters, arguments, matchers)  #name,
#define MOCK_ZLIB_VOID(return_type, name, parameters, arguments, matchers) #name,
#include <mock_zlib_api_table.h>
#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET
    "gzprintf",
};

// libzlib の期待シンボルと実ライブラリの全エクスポートが一致することの確認
TEST(exportTest, zlib_symbols_match_api_table)
{
    // Arrange
    std::set<std::string> expected(
        std::begin(kExpectedExportNames),
        std::end(kExpectedExportNames)); // [状態] - mock_zlib の API 表から期待する公開関数名を構築する。
#if defined(PLATFORM_WINDOWS)
    expected.insert(testing::identManifestSymbolName(
        "libzlib" TESTFW_SHARED_LIBRARY_EXTENSION)); // [状態] - IDENT manifest シンボル名を期待値へ追加する。
#endif                                               /* PLATFORM_WINDOWS */
    std::string path = findWorkspaceRoot() + "/app/zlib/prod/lib/libzlib" +
                       TESTFW_SHARED_LIBRARY_EXTENSION; // [状態] - 検査対象を libzlib の動的ライブラリとする。

    // Pre-Assert

    // Act
    std::set<std::string> actual = testing::getActualExportNames(path); // [手順] - libzlib のエクスポート名を取得する。

    // Assert
    testing::expectExportNamesMatch(expected,
                                    actual); // [確認_正常系] - libzlib のエクスポートに不足や想定外がないこと。
}
