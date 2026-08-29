#include <testfw.h>
#include <struct_meta/json/json.h>
#include <cplat/base/result.h>
#include <cstddef>

namespace
{
struct Sample
{
    int id;
    int hidden;
};
const struct_meta_attribute kIdAttributes[] = {{"json.name", "person_id"}};
const struct_meta_attribute kHiddenAttributes[] = {{"json.ignore", nullptr}};
const struct_meta_field kFields[] = {
    {"id", STRUCT_META_FIELD_INT, 0, offsetof(Sample, id), sizeof(int), 1, 0, nullptr, nullptr, kIdAttributes, 1},
    {"hidden", STRUCT_META_FIELD_INT, 0, offsetof(Sample, hidden), sizeof(int), 1, 0, nullptr, nullptr,
     kHiddenAttributes, 1},
};
const struct_meta_descriptor kDescriptor = {"Sample", sizeof(Sample), kFields, 2, nullptr, nullptr, 0};
} // namespace

TEST(JsonEncodeTest, UsesGenericJsonAttributes)
{
    Sample sample = {42, 7}; // [準備_正常系] - 名前変更属性と除外属性を持つ値を用意する。
    cJSON *json = nullptr;
    ASSERT_EQ(CPLAT_OK, struct_meta_json_encode(&kDescriptor, &sample, &json)); // [手順_正常系]
    ASSERT_NE(nullptr, json); // [確認_正常系] - 属性に従った JSON が生成されること。
    EXPECT_EQ(42, cJSON_GetObjectItemCaseSensitive(json, "person_id")->valueint);
    EXPECT_EQ(nullptr, cJSON_GetObjectItemCaseSensitive(json, "id"));
    EXPECT_EQ(nullptr, cJSON_GetObjectItemCaseSensitive(json, "hidden"));
    cJSON_Delete(json);
}

TEST(JsonEncodeTest, RejectsCorruptDescriptor)
{
    const struct_meta_descriptor descriptor = {"Sample", sizeof(Sample), nullptr, 1, nullptr, nullptr, 0};
    Sample sample = {}; // [準備_異常系] - フィールド配列が欠けた記述子を用意する。
    cJSON *json = nullptr;
    int actual = struct_meta_json_encode(&descriptor, &sample, &json); // [手順_異常系]
    EXPECT_EQ(CPLAT_ERR_CORRUPT_DESCRIPTOR, actual);                   // [確認_異常系] - 検査エラーになること。
    EXPECT_EQ(nullptr, json);
}
