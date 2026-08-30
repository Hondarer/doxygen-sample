#include <testfw.h>

extern "C"
{
#include "struct_meta_gen_ast.h"
}

TEST(structMetaGenAstTest, maps_character_scalar_types)
{
    // Arrange

    // Pre-Assert

    // Act
    const struct_meta_gen_scalar_type *plain =
        struct_meta_gen_find_scalar_type("char"); // [手順] - plain charを検索する。
    const struct_meta_gen_scalar_type *signed_char =
        struct_meta_gen_find_scalar_type("signed char"); // [手順] - signed charを検索する。
    const struct_meta_gen_scalar_type *unsigned_char =
        struct_meta_gen_find_scalar_type("unsigned char"); // [手順] - unsigned charを検索する。

    // Assert
    ASSERT_NE(nullptr, plain); // [確認_正常系] - plain charを認識すること。
    EXPECT_STREQ("STRUCT_META_FIELD_SIGNED_INTEGER",
                 plain->kind_name);  // [確認_正常系] - plain charを符号付きとすること。
    ASSERT_NE(nullptr, signed_char); // [確認_正常系] - signed charを認識すること。
    EXPECT_STREQ("STRUCT_META_FIELD_SIGNED_INTEGER",
                 signed_char->kind_name); // [確認_正常系] - signed charを符号付きとすること。
    ASSERT_NE(nullptr, unsigned_char);    // [確認_正常系] - unsigned charを認識すること。
    EXPECT_STREQ("STRUCT_META_FIELD_UNSIGNED_INTEGER",
                 unsigned_char->kind_name); // [確認_正常系] - unsigned charを符号なしとすること。
}

TEST(structMetaGenAstTest, parses_generic_attributes_and_removes_them_from_brief)
{
    // Arrange
    const char comment[] = "/**< 識別子です。 @struct_meta{json.name = person_id} @struct_meta{sample.flag} */";

    // Pre-Assert

    // Act
    struct_meta_gen_doc_attrs actual =
        struct_meta_gen_doc_attrs_from_raw(comment, 1, 10); // [手順] - 汎用属性を含む後置コメントを解析する。

    // Assert
    EXPECT_EQ(0, actual.invalid);                        // [確認_正常系] - コメントの解析が成功すること。
    EXPECT_STREQ("識別子です。", actual.brief);          // [確認_正常系] - brief から汎用属性の記述が除外されること。
    ASSERT_NE(nullptr, actual.attributes);               // [確認_正常系] - 1 個目の属性が取得できること。
    EXPECT_STREQ("json.name", actual.attributes->key);   // [確認_正常系] - 1 個目の属性名が json.name であること。
    EXPECT_STREQ("person_id", actual.attributes->value); // [確認_正常系] - 1 個目の属性値が person_id であること。
    ASSERT_NE(nullptr, actual.attributes->next);         // [確認_正常系] - 2 個目の属性が取得できること。
    EXPECT_STREQ("sample.flag",
                 actual.attributes->next->key);         // [確認_正常系] - 2 個目の属性名が sample.flag であること。
    EXPECT_EQ(nullptr, actual.attributes->next->value); // [確認_正常系] - 2 個目の属性値が NULL であること。
}

TEST(structMetaGenAstTest, rejects_duplicate_attributes)
{
    // Arrange
    const char comment[] = "/**< @struct_meta{sample.key=1} @struct_meta{sample.key=2} */";

    // Pre-Assert

    // Act
    struct_meta_gen_doc_attrs actual =
        struct_meta_gen_doc_attrs_from_raw(comment, 1, 20); // [手順] - 重複する属性を含むコメントを解析する。

    // Assert
    EXPECT_NE(0, actual.invalid); // [確認_異常系] - 重複する属性が拒否されること。
}

TEST(structMetaGenAstTest, rejects_duplicate_attributes_across_comments)
{
    // Arrange
    struct_meta_gen_doc_attrs prefix = struct_meta_gen_doc_attrs_from_raw("/** @struct_meta{sample.key=1} */", 0, 21);
    struct_meta_gen_doc_attrs postfix = struct_meta_gen_doc_attrs_from_raw("/**< @struct_meta{sample.key=2} */", 1, 21);

    // Pre-Assert

    // Act
    struct_meta_gen_doc_attrs actual = struct_meta_gen_doc_attrs_choose(
        prefix, postfix, 21); // [手順] - 同じ属性名を持つ前置と後置コメントを結合する。

    // Assert
    EXPECT_NE(0, actual.invalid); // [確認_異常系] - コメントをまたいで重複する属性が拒否されること。
}

TEST(structMetaGenAstTest, does_not_convert_legacy_json_commands)
{
    // Arrange
    const char comment[] = "/**< @json_name{person_id} @json_required @json_ignore */";

    // Pre-Assert

    // Act
    struct_meta_gen_doc_attrs actual = struct_meta_gen_doc_attrs_from_raw(
        comment, 1, 22); // [手順] - 廃止した JSON 固有コマンドを含むコメントを解析する。

    // Assert
    EXPECT_EQ(0, actual.invalid); // [確認_正常系] - 未知の Doxygen コマンドが汎用属性の構文エラーにならないこと。
    EXPECT_EQ(nullptr, actual.attributes); // [確認_正常系] - 廃止した JSON 固有コマンドが属性へ変換されないこと。
}

TEST(structMetaGenAstTest, rejects_invalid_attribute_forms)
{
    // Arrange
    const char empty_key[] = "/**< @struct_meta{=value} */";
    const char empty_value[] = "/**< @struct_meta{sample.key=} */";
    const char invalid_key[] = "/**< @struct_meta{sample key=value} */";
    const char missing_close[] = "/**< @struct_meta{sample.key=value */";
    const char multiline[] = "/**< @struct_meta{sample.key=line1\nline2} */";

    // Pre-Assert

    // Act
    struct_meta_gen_doc_attrs empty_key_result =
        struct_meta_gen_doc_attrs_from_raw(empty_key, 1, 30); // [手順] - 空の属性名を解析する。
    struct_meta_gen_doc_attrs empty_value_result =
        struct_meta_gen_doc_attrs_from_raw(empty_value, 1, 31); // [手順] - 空の属性値を解析する。
    struct_meta_gen_doc_attrs invalid_key_result =
        struct_meta_gen_doc_attrs_from_raw(invalid_key, 1, 32); // [手順] - 空白を含む属性名を解析する。
    struct_meta_gen_doc_attrs missing_close_result =
        struct_meta_gen_doc_attrs_from_raw(missing_close, 1, 33); // [手順] - 閉じ波括弧が無い属性を解析する。
    struct_meta_gen_doc_attrs multiline_result =
        struct_meta_gen_doc_attrs_from_raw(multiline, 1, 34); // [手順] - 改行を含む属性値を解析する。

    // Assert
    EXPECT_NE(0, empty_key_result.invalid);     // [確認_異常系] - 空の属性名が拒否されること。
    EXPECT_NE(0, empty_value_result.invalid);   // [確認_異常系] - 空の属性値が拒否されること。
    EXPECT_NE(0, invalid_key_result.invalid);   // [確認_異常系] - 空白を含む属性名が拒否されること。
    EXPECT_NE(0, missing_close_result.invalid); // [確認_異常系] - 閉じ波括弧が無い属性が拒否されること。
    EXPECT_NE(0, multiline_result.invalid);     // [確認_異常系] - 改行を含む属性値が拒否されること。
}
