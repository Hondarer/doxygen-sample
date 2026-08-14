#include <testfw.h>
#include <mock_stdio.h>
#include <calcbase/calcbase_spec.h>

class divideTest : public Test
{
};

// divide(20, 4) が 5 を返すことの確認
TEST_F(divideTest, test_20_divide_4)
{
    // Arrange
    int result;

    // Pre-Assert

    // Act
    int rtc = divide(20, 4, &result); // [手順] - divide(20, 4, &result) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_SUCCESS, rtc); // [確認_正常系] - divide の戻り値が CALC_SUCCESS であること。
    EXPECT_EQ(5, result);         // [確認_正常系] - divide が result に 5 を設定すること。
}

// divide(10, 3) が整数除算の 3 を返すことの確認
TEST_F(divideTest, test_10_divide_3)
{
    // Arrange
    int result;

    // Pre-Assert

    // Act
    int rtc = divide(10, 3, &result); // [手順] - divide(10, 3, &result) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_SUCCESS, rtc); // [確認_正常系] - divide の戻り値が CALC_SUCCESS であること。
    EXPECT_EQ(3, result);         // [確認_正常系] - divide が整数除算の結果として result に 3 を設定すること。
}

// ゼロ除算時に divide が CALC_ERROR を返すことの確認
TEST_F(divideTest, test_divide_by_zero)
{
    // Arrange
    int result;

    // Pre-Assert

    // Act
    int rtc = divide(10, 0, &result); // [手順] - divide(10, 0, &result) **ゼロ除算** を呼び出す。

    // Assert
    EXPECT_EQ(CALC_ERROR, rtc); // [確認_異常系] - divide の戻り値が CALC_ERROR であること。
}

// divide(-12, 4) が -3 を返すことの確認
TEST_F(divideTest, test_negative_divide)
{
    // Arrange
    int result;

    // Pre-Assert

    // Act
    int rtc = divide(-12, 4, &result); // [手順] - divide(-12, 4, &result) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_SUCCESS, rtc); // [確認_正常系] - divide の戻り値が CALC_SUCCESS であること。
    EXPECT_EQ(-3, result);        // [確認_正常系] - divide が result に -3 を設定すること。
}

// result が NULL のとき divide が CALC_ERROR を返すことの確認
TEST_F(divideTest, test_null_result)
{
    // Arrange

    // Pre-Assert

    // Act
    int rtc = divide(20, 4, NULL); // [手順] - divide(20, 4, NULL) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_ERROR, rtc); // [確認_異常系] - divide の戻り値が CALC_ERROR であること。
}
