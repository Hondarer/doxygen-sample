#include <testfw.h>
#include <mock_stdio.h>
#include <calcbase/calcbase_spec.h>

class multiplyTest : public Test
{
};

// multiply(5, 4) が 20 を返すことの確認
TEST_F(multiplyTest, test_5_multiply_4)
{
    // Arrange
    int result;

    // Pre-Assert

    // Act
    int rtc = multiply(5, 4, &result); // [手順] - multiply(5, 4, &result) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_SUCCESS, rtc); // [確認_正常系] - multiply の戻り値が CALC_SUCCESS であること。
    EXPECT_EQ(20, result);        // [確認_正常系] - multiply が result に 20 を設定すること。
}

// multiply(3, 0) が 0 を返すことの確認
TEST_F(multiplyTest, test_3_multiply_0)
{
    // Arrange
    int result;

    // Pre-Assert

    // Act
    int rtc = multiply(3, 0, &result); // [手順] - multiply(3, 0, &result) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_SUCCESS, rtc); // [確認_正常系] - multiply の戻り値が CALC_SUCCESS であること。
    EXPECT_EQ(0, result);         // [確認_正常系] - multiply が result に 0 を設定すること。
}

// multiply(-3, 4) が -12 を返すことの確認
TEST_F(multiplyTest, test_negative_multiply)
{
    // Arrange
    int result;

    // Pre-Assert

    // Act
    int rtc = multiply(-3, 4, &result); // [手順] - multiply(-3, 4, &result) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_SUCCESS, rtc); // [確認_正常系] - multiply の戻り値が CALC_SUCCESS であること。
    EXPECT_EQ(-12, result);       // [確認_正常系] - multiply が result に -12 を設定すること。
}

// result が NULL のとき multiply が CALC_ERROR を返すことの確認
TEST_F(multiplyTest, test_null_result)
{
    // Arrange

    // Pre-Assert

    // Act
    int rtc = multiply(5, 4, NULL); // [手順] - multiply(5, 4, NULL) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_ERROR, rtc); // [確認_異常系] - multiply の戻り値が CALC_ERROR であること。
}
