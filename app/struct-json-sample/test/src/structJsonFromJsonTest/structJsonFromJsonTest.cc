#include <testfw.h>

#include <struct_json/struct_json.h>

#include <com_util/base/result.h>

#include <cstddef>

namespace
{
struct address
{
    char city[16];
    int zip;
};

struct sample_data
{
    int id;
    char name[8];
    address home;
    int scores[3];
};

/* structgen 生成に依存しない、手書きの固定記述子。ネスト構造体・配列メンバーを含む。 */
const sj_field_desc k_address_fields[] = {
    {"city", SJ_FIELD_CHAR_ARRAY, 0, offsetof(address, city), sizeof(char), 1, sizeof(((address *)0)->city), nullptr,
     nullptr, nullptr, 0, 0},
    {"zip", SJ_FIELD_INT, 0, offsetof(address, zip), sizeof(int), 1, 0, nullptr, nullptr, nullptr, 0, 0},
};
const sj_struct_desc k_address_desc = {"address", sizeof(address), k_address_fields, 2, nullptr};

const sj_field_desc k_sample_fields[] = {
    {"id", SJ_FIELD_INT, 0, offsetof(sample_data, id), sizeof(int), 1, 0, nullptr, nullptr, nullptr, 0, 0},
    {"name", SJ_FIELD_CHAR_ARRAY, 0, offsetof(sample_data, name), sizeof(char), 1, sizeof(((sample_data *)0)->name),
     nullptr, nullptr, nullptr, 0, 1},
    {"home", SJ_FIELD_STRUCT, 0, offsetof(sample_data, home), sizeof(address), 1, 0, &k_address_desc, nullptr, nullptr,
     0, 0},
    {"scores", SJ_FIELD_INT, 0, offsetof(sample_data, scores), sizeof(int), 3, 0, nullptr, nullptr, nullptr, 0, 0},
};
const sj_struct_desc k_sample_desc = {"sample_data", sizeof(sample_data), k_sample_fields, 4, nullptr};
} // namespace

class structJsonFromJsonTest : public Test
{
};

TEST_F(structJsonFromJsonTest, test_scalar_nested_and_array_fields)
{
    // Arrange
    const char *text = "{\"id\":42,\"name\":\"abc\",\"home\":{\"city\":\"Tokyo\",\"zip\":100},"
                       "\"scores\":[10,20,30]}";
    cJSON *json = cJSON_Parse(text);
    ASSERT_NE(nullptr, json); // [確認_正常系] - テスト用 JSON テキストの解析に成功すること。
    sample_data s = {};

    // Act
    int actual_ret = sj_from_json(&k_sample_desc, json, &s); // [手順_正常系] - sj_from_json() を呼び出す。

    // Assert
    EXPECT_EQ(COM_UTIL_OK, actual_ret); // [確認_正常系] - sj_from_json() から COM_UTIL_OK が返されること。
    EXPECT_EQ(42, s.id);                // [確認_正常系] - スカラー int が書き戻されること。
    EXPECT_STREQ("abc", s.name);        // [確認_正常系] - JSON 文字列が char[] へ書き戻されること。
    EXPECT_EQ(100, s.home.zip);         // [確認_正常系] - ネストしたフィールドが書き戻されること。
    EXPECT_STREQ("Tokyo", s.home.city); // [確認_正常系] - ネストした char[] が書き戻されること。
    EXPECT_EQ(10, s.scores[0]);         // [確認_正常系] - 配列 1 番目の要素が書き戻されること。
    EXPECT_EQ(30, s.scores[2]);         // [確認_正常系] - 配列 3 番目の要素が書き戻されること。

    // Cleanup
    cJSON_Delete(json);
}

TEST_F(structJsonFromJsonTest, test_missing_key)
{
    // Arrange
    cJSON *json = cJSON_Parse("{\"id\":1}");
    ASSERT_NE(nullptr, json); // [確認_異常系] - テスト用 JSON テキストの解析に成功すること。
    sample_data s = {};

    // Act
    int actual_ret = sj_from_json(&k_sample_desc, json, &s); // [手順_異常系] - 必須キー欠落の JSON を渡して呼び出す。

    // Assert
    EXPECT_EQ(COM_UTIL_ERR_MISSING_REQUIRED,
              actual_ret); // [確認_異常系] - COM_UTIL_ERR_MISSING_REQUIRED が返されること。

    // Cleanup
    cJSON_Delete(json);
}

TEST_F(structJsonFromJsonTest, test_char_array_too_small)
{
    // Arrange
    cJSON *json = cJSON_Parse("{\"id\":1,\"name\":\"this-name-is-too-long\","
                              "\"home\":{\"city\":\"Tokyo\",\"zip\":1},\"scores\":[1,2,3]}");
    ASSERT_NE(nullptr, json); // [確認_異常系] - テスト用 JSON テキストの解析に成功すること。
    sample_data s = {};

    // Act
    int actual_ret =
        sj_from_json(&k_sample_desc, json, &s); // [手順_異常系] - バッファーを超える文字列を渡して呼び出す。

    // Assert
    EXPECT_EQ(COM_UTIL_ERR_BUFFER_TOO_SMALL,
              actual_ret); // [確認_異常系] - COM_UTIL_ERR_BUFFER_TOO_SMALL が返されること。

    // Cleanup
    cJSON_Delete(json);
}

TEST_F(structJsonFromJsonTest, test_null_argument)
{
    // Arrange
    sample_data s = {};

    // Act
    int actual_ret = sj_from_json(&k_sample_desc, nullptr, &s); // [手順_異常系] - json に NULL を渡して呼び出す。

    // Assert
    EXPECT_EQ(COM_UTIL_ERR_INVALID_ARGUMENT,
              actual_ret); // [確認_異常系] - COM_UTIL_ERR_INVALID_ARGUMENT が返されること。
}

TEST_F(structJsonFromJsonTest, test_json_name)
{
    // Arrange
    const sj_field_desc renamed_fields[] = {
        {"id", SJ_FIELD_INT, 0, offsetof(sample_data, id), sizeof(int), 1, 0, nullptr, nullptr, "person_id", 0, 0},
        {"name", SJ_FIELD_CHAR_ARRAY, 0, offsetof(sample_data, name), sizeof(char), 1, sizeof(((sample_data *)0)->name),
         nullptr, nullptr, nullptr, 0, 0},
        {"home", SJ_FIELD_STRUCT, 0, offsetof(sample_data, home), sizeof(address), 1, 0, &k_address_desc, nullptr,
         nullptr, 0, 0},
        {"scores", SJ_FIELD_INT, 0, offsetof(sample_data, scores), sizeof(int), 3, 0, nullptr, nullptr, nullptr, 0, 0},
    };
    const sj_struct_desc renamed_desc = {"sample_data", sizeof(sample_data), renamed_fields, 4, nullptr};
    cJSON *json = cJSON_Parse("{\"person_id\":7,\"name\":\"abc\",\"home\":{\"city\":\"Tokyo\",\"zip\":1},"
                              "\"scores\":[1,2,3]}");
    ASSERT_NE(nullptr, json); // [確認_正常系] - テスト用 JSON テキストの解析に成功すること。
    sample_data s = {};

    // Act
    int actual_ret = sj_from_json(&renamed_desc, json, &s); // [手順_正常系] - json_name のキーで呼び出す。

    // Assert
    EXPECT_EQ(COM_UTIL_OK, actual_ret); // [確認_正常系] - sj_from_json() から COM_UTIL_OK が返されること。
    EXPECT_EQ(7, s.id);                 // [確認_正常系] - 別名キーの値が書き戻されること。

    // Cleanup
    cJSON_Delete(json);
}

TEST_F(structJsonFromJsonTest, test_missing_optional_key)
{
    // Arrange
    cJSON *json = cJSON_Parse("{\"id\":1}");
    ASSERT_NE(nullptr, json); // [確認_正常系] - テスト用 JSON テキストの解析に成功すること。
    sample_data s = {};
    s.name[0] = 'x';

    // Act
    const sj_field_desc optional_fields[] = {
        {"id", SJ_FIELD_INT, 0, offsetof(sample_data, id), sizeof(int), 1, 0, nullptr, nullptr, nullptr, 0, 0},
        {"name", SJ_FIELD_CHAR_ARRAY, 0, offsetof(sample_data, name), sizeof(char), 1, sizeof(((sample_data *)0)->name),
         nullptr, nullptr, nullptr, 0, 0},
    };
    const sj_struct_desc optional_desc = {"sample_data", sizeof(sample_data), optional_fields, 2, nullptr};
    int actual_ret = sj_from_json(&optional_desc, json, &s); // [手順_正常系] - 任意キー欠落の JSON を渡す。

    // Assert
    EXPECT_EQ(COM_UTIL_OK, actual_ret); // [確認_正常系] - 任意キー欠落でも COM_UTIL_OK が返されること。
    EXPECT_EQ(1, s.id);                 // [確認_正常系] - 存在するキーは書き戻されること。
    EXPECT_EQ('x', s.name[0]);          // [確認_正常系] - 欠落キーの値は変わらないこと。

    // Cleanup
    cJSON_Delete(json);
}
