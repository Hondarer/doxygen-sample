#include <testfw.h>
#include <mock_stdio.h>
#include <calcbase/calcbase_spec.h>

class addTest : public Test
{
};

TEST_F(addTest, test_1_add_2)
{
    // Arrange
    int result;

    // Pre-Assert

    // Act
    int rtc = add(1, 2, &result); // [手順] - add(1, 2, &result) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_SUCCESS, rtc); // [確認_正常系] - add の戻り値が CALC_SUCCESS であること。
    EXPECT_EQ(3, result);         // [確認_正常系] - add が result に 3 を設定すること。
}

TEST_F(addTest, test_2_add_1)
{
    // Arrange
    int result;

    // Pre-Assert

    // Act
    int rtc = add(2, 1, &result); // [手順] - add(2, 1, &result) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_SUCCESS, rtc); // [確認_正常系] - add の戻り値が CALC_SUCCESS であること。
    EXPECT_EQ(3, result);         // [確認_正常系] - add が result に 3 を設定すること。
}

TEST_F(addTest, test_null_result)
{
    // Arrange

    // Pre-Assert

    // Act
    int rtc = add(1, 2, NULL); // [手順] - add(1, 2, NULL) を呼び出す。

    // Assert
    EXPECT_EQ(CALC_ERROR, rtc); // [確認_異常系] - add の戻り値が CALC_ERROR であること。
}
