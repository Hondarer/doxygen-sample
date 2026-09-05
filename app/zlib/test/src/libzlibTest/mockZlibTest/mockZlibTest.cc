#include <testfw.h>
#include <mock_zlib.h>

#include <cstdio>

class mockZlibTest : public Test
{
  protected:
    void TearDown() override
    {
        std::remove("zlib_printf_test.gz");
    }
};

TEST_F(mockZlibTest, delegates_without_mock)
{
    // Arrange
    const Bytef source[] = "123456789"; // [状態] - モックを生成せず既知の入力を用意する。

    // Pre-Assert

    // Act
    const uLong crc = crc32(0, source, 9); // [手順] - 実関数へ委譲する。

    // Assert
    EXPECT_EQ(0xcbf43926UL, crc);              // [確認_正常系] - 実ライブラリの計算結果であること。
    EXPECT_STREQ(ZLIB_VERSION, zlibVersion()); // [確認_正常系] - 同梱した版を読み込んだこと。
}

TEST_F(mockZlibTest, delegates_unspecified_calls)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib; // [状態] - 個別の動作を指定せずモックを生成する。
    const Bytef source[] = "123456789";

    // Pre-Assert

    // Act
    const uLong crc = crc32(0, source, 9); // [手順] - 既定動作を通して実関数へ委譲する。

    // Assert
    EXPECT_EQ(0xcbf43926UL, crc); // [確認_正常系] - モック生成後も実関数を呼べること。
}

TEST_F(mockZlibTest, overrides_error_and_init_macro)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib;
    z_stream stream = {}; // [状態] - マクロが渡すストリームを用意する。

    // Pre-Assert
    EXPECT_CALL(mock_zlib, deflateInit_(&stream, Z_DEFAULT_COMPRESSION, StrEq(ZLIB_VERSION), sizeof(z_stream)))
        .WillOnce(Return(Z_MEM_ERROR)); // [Pre-Assert確認_異常系] - 初期化マクロの展開先が所定の引数で呼ばれること。
                                        // [Pre-Assert手順] - メモリー不足を返す。

    // Act
    const int ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION); // [手順] - 公開マクロから呼び出す。

    // Assert
    EXPECT_EQ(Z_MEM_ERROR, ret); // [確認_異常系] - モックのエラーが呼び出し元へ戻ること。
}

TEST_F(mockZlibTest, printf_passes_variadic_arguments)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib; // [状態] - 実ファイルを使わず書式化を観測する。
    char output[32] = {};

    // Pre-Assert
    EXPECT_CALL(mock_zlib, gzvprintf(nullptr, StrEq("%s:%d"), _))
        .WillOnce(
            [&output](gzFile, const char *format, va_list args)
            {
                return std::vsnprintf(output, sizeof(output), format, args);
            }); // [Pre-Assert確認_正常系] - 可変長引数が gzvprintf へ渡ること。

    // Act
    const int ret = gzprintf(nullptr, "%s:%d", "value", 42); // [手順] - 異なる型の可変長引数を渡す。

    // Assert
    EXPECT_EQ(8, ret);                // [確認_正常系] - 書式化後の長さを返すこと。
    EXPECT_STREQ("value:42", output); // [確認_正常系] - 引数の内容を保持していること。
}

TEST_F(mockZlibTest, printf_delegates_to_real_file)
{
    // Arrange
    char output[32] = {}; // [状態] - モック未生成で実ファイルへの委譲を検証する。

    // Pre-Assert

    // Act
    gzFile writer = gzopen("zlib_printf_test.gz", "wb");    // [手順] - 実ファイルを開く。
    ASSERT_NE(nullptr, writer);                             // [確認_正常系] - 出力を開けること。
    const int ret = gzprintf(writer, "%s:%d", "value", 42); // [手順] - 可変長引数を実関数へ委譲する。
    const int close_ret = gzclose(writer);
    gzFile reader = gzopen("zlib_printf_test.gz", "rb");
    ASSERT_NE(nullptr, reader); // [確認_正常系] - 入力を開けること。
    const int size = gzread(reader, output, sizeof(output));
    const int read_close_ret = gzclose(reader);

    // Assert
    EXPECT_EQ(8, ret);                // [確認_正常系] - 書式化結果の長さを返すこと。
    EXPECT_EQ(Z_OK, close_ret);       // [確認_正常系] - gzip 出力を完了できること。
    EXPECT_EQ(Z_OK, read_close_ret);  // [確認_正常系] - gzip 入力を終了できること。
    EXPECT_EQ(8, size);               // [確認_正常系] - 書き込んだ長さを読み出せること。
    EXPECT_STREQ("value:42", output); // [確認_正常系] - 実ファイルに書式化結果が残ること。
}
