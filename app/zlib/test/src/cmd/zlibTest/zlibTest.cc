#include <testfw.h>
#include <zlib.h>

#include <array>
#include <cstdio>
#include <vector>

class zlibTest : public Test
{
  protected:
    void TearDown() override
    {
        std::remove("zlib_roundtrip_test.gz");
    }
};

TEST_F(zlibTest, memory_roundtrip_including_empty_and_binary)
{
    // Arrange
    const std::vector<std::vector<Bytef>> inputs = {
        {}, {0, 1, 0, 127, 128, 255}, std::vector<Bytef>(4096, 65)}; // [状態] - 空・バイナリ・反復データを用意する。

    // Pre-Assert

    for (const auto &source : inputs)
    {
        const Bytef empty = 0;
        const Bytef *input = &empty;
        if (!source.empty())
        {
            input = source.data();
        }
        const uLong input_size = static_cast<uLong>(source.size());
        std::vector<Bytef> compressed(compressBound(input_size));
        uLongf compressed_size = static_cast<uLongf>(compressed.size());
        std::vector<Bytef> restored(source.size() + 1);
        uLongf restored_size = static_cast<uLongf>(restored.size());

        // Act
        const int compress_ret =
            compress(compressed.data(), &compressed_size, input, input_size); // [手順] - メモリーを圧縮する。
        const int restore_ret = uncompress(restored.data(), &restored_size, compressed.data(),
                                           compressed_size); // [手順] - 圧縮結果を展開する。

        // Assert
        ASSERT_EQ(Z_OK, compress_ret); // [確認_正常系] - 圧縮に成功すること。
        ASSERT_EQ(Z_OK, restore_ret);  // [確認_正常系] - 展開に成功すること。
        restored.resize(restored_size);
        EXPECT_EQ(source, restored); // [確認_正常系] - 空・バイナリを含めて元データと一致すること。
    }
}

TEST_F(zlibTest, invalid_data_and_short_buffer)
{
    // Arrange
    const Bytef invalid[] = {0xff, 0xff, 0xff}; // [状態] - zlib 形式ではないデータを用意する。
    Bytef output[16] = {};
    uLongf size = sizeof(output);
    uLongf short_size = 1;

    // Pre-Assert

    // Act
    const int invalid_ret = uncompress(output, &size, invalid, sizeof(invalid)); // [手順] - 不正データを展開する。
    const int short_ret =
        compress(output, &short_size, invalid, sizeof(invalid)); // [手順] - 出力領域を 1 バイトに制限する。

    // Assert
    EXPECT_EQ(Z_DATA_ERROR, invalid_ret); // [確認_異常系] - 不正な圧縮形式を検出すること。
    EXPECT_EQ(Z_BUF_ERROR, short_ret);    // [確認_異常系] - 出力領域不足を検出すること。
}

TEST_F(zlibTest, checksum_known_vector)
{
    // Arrange
    const Bytef source[] = "123456789"; // [状態] - チェックサムの既知の入力を用意する。

    // Pre-Assert

    // Act
    const uLong crc = crc32(0, source, 9);     // [手順] - CRC-32 を計算する。
    const uLong adler = adler32(1, source, 9); // [手順] - Adler-32 を計算する。

    // Assert
    EXPECT_EQ(0xcbf43926UL, crc);   // [確認_正常系] - CRC-32 の既知の値と一致すること。
    EXPECT_EQ(0x091e01deUL, adler); // [確認_正常系] - Adler-32 の既知の値と一致すること。
}

TEST_F(zlibTest, gzip_file_roundtrip)
{
    // Arrange
    const std::array<Bytef, 6> source = {0, 255, 1, 2, 0, 128}; // [状態] - NUL を含むデータを用意する。
    std::array<Bytef, 6> restored = {};

    // Pre-Assert

    // Act
    gzFile writer = gzopen("zlib_roundtrip_test.gz", "wb"); // [手順] - gzip 出力を開く。
    ASSERT_NE(nullptr, writer);                             // [確認_正常系] - 出力ファイルが開くこと。
    const int written =
        gzwrite(writer, source.data(), static_cast<unsigned>(source.size())); // [手順] - バイナリを圧縮して書き込む。
    const int write_close_ret = gzclose(writer);
    gzFile reader = gzopen("zlib_roundtrip_test.gz", "rb"); // [手順] - gzip 入力を開く。
    ASSERT_NE(nullptr, reader);                             // [確認_正常系] - 入力ファイルが開くこと。
    const int read_size =
        gzread(reader, restored.data(), static_cast<unsigned>(restored.size())); // [手順] - データを展開して読み込む。
    const int read_close_ret = gzclose(reader);

    // Assert
    EXPECT_EQ(6, written);            // [確認_正常系] - 全バイトを書き込んだこと。
    EXPECT_EQ(6, read_size);          // [確認_正常系] - 全バイトを読み込んだこと。
    EXPECT_EQ(Z_OK, write_close_ret); // [確認_正常系] - 書き込みを完了できたこと。
    EXPECT_EQ(Z_OK, read_close_ret);  // [確認_正常系] - 読み込みを終了できたこと。
    EXPECT_EQ(source, restored);      // [確認_正常系] - ファイルの往復で元データを復元できたこと。
}

TEST_F(zlibTest, size_t_api_roundtrip)
{
    // Arrange
    const Bytef source[] = "zlib 1.3.2"; // [状態] - size_t 版 API の入力を用意する。
    Bytef output[128] = {};
    Bytef restored[sizeof(source)] = {};
    z_size_t output_size = sizeof(output);
    z_size_t restored_size = sizeof(restored);

    // Pre-Assert

    // Act
    const int compress_ret = compress_z(output, &output_size, source, sizeof(source)); // [手順] - size_t 版で圧縮する。
    const int restore_ret =
        uncompress_z(restored, &restored_size, output, output_size); // [手順] - size_t 版で展開する。

    // Assert
    EXPECT_EQ(Z_OK, compress_ret);                   // [確認_正常系] - 圧縮に成功すること。
    EXPECT_EQ(Z_OK, restore_ret);                    // [確認_正常系] - 展開に成功すること。
    EXPECT_EQ(sizeof(source), restored_size);        // [確認_正常系] - 復元サイズが一致すること。
    EXPECT_THAT(restored, ElementsAreArray(source)); // [確認_正常系] - 復元内容が一致すること。
}

TEST_F(zlibTest, decompression_buffer_too_small)
{
    // Arrange
    const Bytef source[] = "restored data exceeds one byte";
    Bytef compressed[128] = {};
    uLongf compressed_size = sizeof(compressed);
    ASSERT_EQ(Z_OK, compress(compressed, &compressed_size, source,
                             sizeof(source))); // [状態確認] - 展開対象の圧縮データを準備できたこと。
    Bytef restored[1] = {};
    uLongf restored_size = sizeof(restored); // [状態] - 展開先を 1 バイトに制限する。

    // Pre-Assert

    // Act
    const int ret =
        uncompress(restored, &restored_size, compressed, compressed_size); // [手順] - 狭いバッファーへ展開する。

    // Assert
    EXPECT_EQ(Z_BUF_ERROR, ret); // [確認_異常系] - 展開先の不足を検出すること。
}
