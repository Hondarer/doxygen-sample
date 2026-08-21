#include <testfw.h>

#include <struct_json/struct_json.h>

#include <com_util/base/result.h>

#include <cstddef>
#include <cstdio>

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
    unsigned int flags;
    double ratio;
    float temp;
    char name[8];
    address home;
    address others[2];
    int scores[3];
    int pad; /* 明示的アラインメント (ratio の 8 バイト境界に合わせた末尾パディング) */
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
    {"flags", SJ_FIELD_UNSIGNED, 0, offsetof(sample_data, flags), sizeof(unsigned int), 1, 0, nullptr, nullptr, nullptr,
     0, 0},
    {"ratio", SJ_FIELD_DOUBLE, 0, offsetof(sample_data, ratio), sizeof(double), 1, 0, nullptr, nullptr, nullptr, 0, 0},
    {"temp", SJ_FIELD_FLOAT, 0, offsetof(sample_data, temp), sizeof(float), 1, 0, nullptr, nullptr, nullptr, 0, 0},
    {"name", SJ_FIELD_CHAR_ARRAY, 0, offsetof(sample_data, name), sizeof(char), 1, sizeof(((sample_data *)0)->name),
     nullptr, nullptr, nullptr, 0, 0},
    {"home", SJ_FIELD_STRUCT, 0, offsetof(sample_data, home), sizeof(address), 1, 0, &k_address_desc, nullptr, nullptr,
     0, 0},
    {"others", SJ_FIELD_STRUCT, 0, offsetof(sample_data, others), sizeof(address), 2, 0, &k_address_desc, nullptr,
     nullptr, 0, 0},
    {"scores", SJ_FIELD_INT, 0, offsetof(sample_data, scores), sizeof(int), 3, 0, nullptr, nullptr, nullptr, 0, 0},
};
const sj_struct_desc k_sample_desc = {"sample_data", sizeof(sample_data), k_sample_fields, 8, nullptr};
} // namespace

class structJsonToJsonTest : public Test
{
};

TEST_F(structJsonToJsonTest, test_scalar_nested_and_array_fields)
{
    // Arrange
    sample_data s = {};
    s.id = 42;
    s.flags = 7U;
    s.ratio = 1.5;
    s.temp = 2.5f;
    snprintf(s.name, sizeof(s.name), "abc");
    s.home.zip = 100;
    snprintf(s.home.city, sizeof(s.home.city), "Tokyo");
    s.others[0].zip = 1;
    snprintf(s.others[0].city, sizeof(s.others[0].city), "A");
    s.others[1].zip = 2;
    snprintf(s.others[1].city, sizeof(s.others[1].city), "B");
    s.scores[0] = 10;
    s.scores[1] = 20;
    s.scores[2] = 30;

    // Act
    cJSON *json = nullptr;
    int actual_ret = sj_to_json(&k_sample_desc, &s, &json); // [手順_正常系] - sj_to_json() を呼び出す。

    // Assert
    EXPECT_EQ(COM_UTIL_OK, actual_ret); // [確認_正常系] - sj_to_json() から COM_UTIL_OK が返されること。
    ASSERT_NE(nullptr, json);           // [確認_正常系] - json が NULL でないこと。

    EXPECT_EQ(42, (int)cJSON_GetNumberValue(
                      cJSON_GetObjectItemCaseSensitive(json, "id"))); // [確認_正常系] - スカラー int が変換されること。
    EXPECT_STREQ("abc", cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(
                            json, "name"))); // [確認_正常系] - char[] が文字列になること。

    cJSON *home = cJSON_GetObjectItemCaseSensitive(json, "home");
    ASSERT_NE(nullptr, home); // [確認_正常系] - ネスト構造体がオブジェクトになること。
    EXPECT_STREQ("Tokyo", cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(
                              home, "city"))); // [確認_正常系] - ネストしたフィールドが変換されること。

    cJSON *others = cJSON_GetObjectItemCaseSensitive(json, "others");
    ASSERT_NE(nullptr, others);               // [確認_正常系] - 構造体配列が JSON 配列になること。
    EXPECT_EQ(2, cJSON_GetArraySize(others)); // [確認_正常系] - 配列要素数が一致すること。

    cJSON *scores = cJSON_GetObjectItemCaseSensitive(json, "scores");
    ASSERT_NE(nullptr, scores);               // [確認_正常系] - プリミティブ配列が JSON 配列になること。
    EXPECT_EQ(3, cJSON_GetArraySize(scores)); // [確認_正常系] - 配列要素数が一致すること。
    EXPECT_EQ(20, (int)cJSON_GetNumberValue(
                      cJSON_GetArrayItem(scores, 1))); // [確認_正常系] - 配列 2 番目の要素値が一致すること。

    // Cleanup
    cJSON_Delete(json);
}

TEST_F(structJsonToJsonTest, test_null_argument)
{
    // Arrange
    sample_data s = {};
    cJSON *json = nullptr;

    // Act
    int actual_ret = sj_to_json(nullptr, &s, &json); // [手順_異常系] - desc に NULL を渡して呼び出す。

    // Assert
    EXPECT_EQ(COM_UTIL_ERR_INVALID_ARGUMENT,
              actual_ret); // [確認_異常系] - COM_UTIL_ERR_INVALID_ARGUMENT が返されること。
}

TEST_F(structJsonToJsonTest, test_json_name_and_ignore)
{
    // Arrange
    struct tagged
    {
        int id;
        int hidden;
    };
    const sj_field_desc tagged_fields[] = {
        {"id", SJ_FIELD_INT, 0, offsetof(tagged, id), sizeof(int), 1, 0, nullptr, nullptr, "person_id", 0, 0},
        {"hidden", SJ_FIELD_INT, 0, offsetof(tagged, hidden), sizeof(int), 1, 0, nullptr, nullptr, nullptr, 1, 0},
    };
    const sj_struct_desc tagged_desc = {"tagged", sizeof(tagged), tagged_fields, 2, nullptr};
    tagged s = {};
    s.id = 9;
    s.hidden = 4;

    // Act
    cJSON *json = nullptr;
    int actual_ret = sj_to_json(&tagged_desc, &s, &json); // [手順_正常系] - json_name と json_ignore を付けて呼び出す。

    // Assert
    EXPECT_EQ(COM_UTIL_OK, actual_ret); // [確認_正常系] - sj_to_json() から COM_UTIL_OK が返されること。
    ASSERT_NE(nullptr, json);           // [確認_正常系] - json が NULL でないこと。
    EXPECT_EQ(9, (int)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(
                     json, "person_id"))); // [確認_正常系] - 別名キーで出力されること。
    EXPECT_EQ(nullptr, cJSON_GetObjectItemCaseSensitive(
                           json, "hidden")); // [確認_正常系] - json_ignore のキーは JSON に出ないこと。
    EXPECT_EQ(nullptr, cJSON_GetObjectItemCaseSensitive(json, "id")); // [確認_正常系] - C 名のキーは出ないこと。

    // Cleanup
    cJSON_Delete(json);
}
