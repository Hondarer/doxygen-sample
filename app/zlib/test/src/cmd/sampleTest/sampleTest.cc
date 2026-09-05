#include <testfw.h>
#include <mock_zlib.h>

TEST(sampleTest, successful_roundtrip)
{
    // Arrange

    // Pre-Assert

    // Act
    const int ret = __real_main(0, nullptr); // [手順] - 実ライブラリへ委譲してサンプルを実行する。

    // Assert
    EXPECT_EQ(0, ret); // [確認_正常系] - 復元結果が一致し成功終了すること。
}

TEST(sampleTest, compression_error)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib;

    // Pre-Assert
    EXPECT_CALL(mock_zlib, compress2(_, _, _, _, Z_BEST_COMPRESSION))
        .WillOnce(Return(Z_MEM_ERROR));                      // [Pre-Assert確認_異常系] - 圧縮が 1 回呼ばれること。
                                                             // [Pre-Assert手順] - メモリー不足を返す。
    EXPECT_CALL(mock_zlib, uncompress(_, _, _, _)).Times(0); // [Pre-Assert確認_異常系] - 失敗後は展開しないこと。

    // Act
    const int ret = __real_main(0, nullptr); // [手順] - 圧縮エラー時のサンプルを実行する。

    // Assert
    EXPECT_EQ(1, ret); // [確認_異常系] - エラー終了すること。
}

TEST(sampleTest, decompression_error)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib;

    // Pre-Assert
    EXPECT_CALL(mock_zlib, uncompress(_, _, _, _))
        .WillOnce(Return(Z_DATA_ERROR)); // [Pre-Assert確認_異常系] - 展開が 1 回呼ばれること。
                                         // [Pre-Assert手順] - データ不正を返す。

    // Act
    const int ret = __real_main(0, nullptr); // [手順] - 展開エラー時のサンプルを実行する。

    // Assert
    EXPECT_EQ(1, ret); // [確認_異常系] - エラー終了すること。
}

TEST(sampleTest, restored_data_mismatch)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib;

    // Pre-Assert
    EXPECT_CALL(mock_zlib, uncompress(_, _, _, _))
        .WillOnce(Return(Z_OK)); // [Pre-Assert確認_異常系] - 展開が 1 回呼ばれること。
                                 // [Pre-Assert手順] - バッファーを更新せず成功を返す。

    // Act
    const int ret = __real_main(0, nullptr); // [手順] - 復元データが一致しないサンプルを実行する。

    // Assert
    EXPECT_EQ(1, ret); // [確認_異常系] - 内容不一致でエラー終了すること。
}
