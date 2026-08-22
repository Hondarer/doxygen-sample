#include <gtest/gtest.h>
#include <struct_meta/json/json.h>
#include <com_util/base/result.h>
#include <cstddef>

namespace
{
struct Sample
{
    int id;
    int optional;
};
const struct_meta_attribute kIdAttributes[] = {{"json.name", "person_id"}, {"json.required", nullptr}};
const struct_meta_field kFields[] = {
    {"id", STRUCT_META_FIELD_INT, 0, offsetof(Sample, id), sizeof(int), 1, 0, nullptr, nullptr, kIdAttributes, 2},
    {"optional", STRUCT_META_FIELD_INT, 0, offsetof(Sample, optional), sizeof(int), 1, 0, nullptr, nullptr, nullptr, 0},
};
const struct_meta_descriptor kDescriptor = {"Sample", sizeof(Sample), kFields, 2, nullptr};
} // namespace

TEST(JsonDecodeTest, UsesGenericJsonAttributes)
{
    cJSON *json = cJSON_Parse("{\"person_id\":42}");
    Sample sample = {0, 9}; // [準備_正常系] - 任意フィールドに既存値を持たせる。
    ASSERT_NE(nullptr, json);
    int actual = struct_meta_json_decode(&kDescriptor, json, &sample); // [手順_正常系]
    EXPECT_EQ(COM_UTIL_OK, actual); // [確認_正常系] - 必須の別名キーを読み込めること。
    EXPECT_EQ(42, sample.id);
    EXPECT_EQ(9, sample.optional);
    cJSON_Delete(json);
}

TEST(JsonDecodeTest, ReportsMissingRequiredAttribute)
{
    cJSON *json = cJSON_CreateObject();
    Sample sample = {}; // [準備_異常系] - 必須キーのない JSON を用意する。
    ASSERT_NE(nullptr, json);
    int actual = struct_meta_json_decode(&kDescriptor, json, &sample); // [手順_異常系]
    EXPECT_EQ(COM_UTIL_ERR_MISSING_REQUIRED, actual); // [確認_異常系] - 必須キー欠落を報告すること。
    cJSON_Delete(json);
}
