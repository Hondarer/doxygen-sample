#include <cplat/base/result.h>
#include <sample_types_meta.h>
#include <struct_meta/access/access.h>
#include <testfw.h>

TEST(sampleTypesMetaTest, finds_descriptor_through_embedded_index)
{
    // Arrange
    /* 生成カタログには線形走査の経路が無い。埋め込みイメージへ接続できなければ
       attach_index() が abort() するため、検索が成立すること自体が
       「索引へ接続できた」ことの表明になる。 */

    // Pre-Assert
    ASSERT_EQ(2U, sample_types_meta_count()); // [事前条件] - 生成カタログが 2 型を持つこと。

    // Act
    const struct_meta_descriptor *person = sample_types_meta_find("person"); // [手順] - 型名で記述子を検索する。
    const struct_meta_descriptor *address = sample_types_meta_find("address"); // [手順] - もう 1 つの型名を検索する。
    const struct_meta_descriptor *unknown = sample_types_meta_find("unknown"); // [手順] - 未知の型名を検索する。
    const struct_meta_descriptor *empty = sample_types_meta_find("");          // [手順] - 空の型名を検索する。

    // Assert
    ASSERT_NE(nullptr, person);           // [確認_正常系] - person の記述子が引けること。
    EXPECT_STREQ("person", person->name); // [確認_正常系] - 引いた記述子が person であること。
    ASSERT_NE(nullptr, address);          // [確認_正常系] - address の記述子が引けること。
    EXPECT_STREQ("address", address->name); // [確認_正常系] - 引いた記述子が address であること。
    EXPECT_EQ(nullptr, unknown);            // [確認_正常系] - 未知の型名で NULL が返ること。
    EXPECT_EQ(nullptr, empty);              // [確認_正常系] - 空の型名で NULL が返ること。
}

TEST(sampleTypesMetaTest, rejects_null_name)
{
    // Arrange

    // Pre-Assert

    // Act
    const struct_meta_descriptor *actual = sample_types_meta_find(nullptr); // [手順] - NULL の型名を検索する。

    // Assert
    EXPECT_EQ(nullptr, actual); // [確認_異常系] - NULL の型名で NULL が返ること。
}

TEST(sampleTypesMetaTest, registers_descriptors_into_index)
{
    // Arrange
    const struct_meta_field *field = nullptr;
    const struct_meta_descriptor *person = sample_types_meta_find("person");

    // Pre-Assert
    ASSERT_NE(nullptr, person); // [事前条件] - person の記述子が引けること。

    // Act
    /* 検索は索引の有無に関わらず成立するため、ここでは結果の同一性だけを確かめる。 */
    int actual = struct_meta_descriptor_find_field(person, "balance",
                                                   &field); // [手順] - 登録済み記述子のフィールドを検索する。

    // Assert
    ASSERT_EQ(CPLAT_OK, actual);                            // [確認_正常系] - フィールド検索が成功すること。
    EXPECT_STREQ("balance", field->name);                   // [確認_正常系] - 要求したフィールドが返ること。
    EXPECT_EQ(STRUCT_META_FIELD_SIGNED_INTEGER, field->kind); // [確認_正常系] - 符号付き整数と判定されること。
    EXPECT_EQ(sizeof(int64_t), field->element_size);        // [確認_正常系] - 幅が 8 バイトであること。
}
