#include <testfw.h>
#include <struct_meta/json/json.h>
#include <cplat/base/result.h>
#include <cstddef>
#include <cstdint>

namespace
{
struct Sample
{
    int id;
    int optional;
};
const struct_meta_attribute kIdAttributes[] = {{"json.name", "person_id"}, {"json.required", nullptr}};
const struct_meta_field kFields[] = {
    {"id", STRUCT_META_FIELD_SIGNED_INTEGER, 0, offsetof(Sample, id), sizeof(int), 1, 0, nullptr, nullptr, kIdAttributes, 2},
    {"optional", STRUCT_META_FIELD_SIGNED_INTEGER, 0, offsetof(Sample, optional), sizeof(int), 1, 0, nullptr, nullptr, nullptr, 0},
};
const struct_meta_descriptor kDescriptor = {"Sample", sizeof(Sample), kFields, 2, nullptr, nullptr, 0};

struct Widths
{
    int64_t wide;
    uint32_t flags;
    int16_t offset;
    uint8_t rank;
    uint8_t reserved; /* 明示的アラインメント。 */
};
const struct_meta_field kWidthFields[] = {
    {"wide", STRUCT_META_FIELD_SIGNED_INTEGER, 0, offsetof(Widths, wide), sizeof(int64_t), 1, 0, nullptr, nullptr,
     nullptr, 0},
    {"flags", STRUCT_META_FIELD_UNSIGNED_INTEGER, 0, offsetof(Widths, flags), sizeof(uint32_t), 1, 0, nullptr, nullptr,
     nullptr, 0},
    {"offset", STRUCT_META_FIELD_SIGNED_INTEGER, 0, offsetof(Widths, offset), sizeof(int16_t), 1, 0, nullptr, nullptr,
     nullptr, 0},
    {"rank", STRUCT_META_FIELD_UNSIGNED_INTEGER, 0, offsetof(Widths, rank), sizeof(uint8_t), 1, 0, nullptr, nullptr,
     nullptr, 0},
};
const struct_meta_descriptor kWidthsDescriptor = {"Widths", sizeof(Widths), kWidthFields, 4, nullptr, nullptr, 0};

/** 幅ごとの限界値を JSON から書き戻し、結果コードを返します。 */
int decode_widths(const char *text, Widths *sample)
{
    cJSON *json = cJSON_Parse(text);
    if (json == nullptr)
    {
        return CPLAT_ERR_INVALID_ARGUMENT;
    }
    int ret = struct_meta_json_decode(&kWidthsDescriptor, json, sample);
    cJSON_Delete(json);
    return ret;
}
} // namespace

TEST(JsonDecodeTest, DecodesEachIntegerWidth)
{
    Widths sample = {}; // [準備_正常系] - 各幅の限界値を含む JSON を用意する。
    int actual = decode_widths(
        "{\"wide\":-999999999999999,\"flags\":4294967295,\"offset\":-32768,\"rank\":255}",
        &sample); // [手順_正常系]
    ASSERT_EQ(CPLAT_OK, actual);                  // [確認_正常系] - 幅ごとの限界値が受理されること。
    EXPECT_EQ(-999999999999999, sample.wide);     // [確認_正常系] - 64 ビット符号付きが復元されること。
    EXPECT_EQ(4294967295U, sample.flags);         // [確認_正常系] - 32 ビット符号なしが復元されること。
    EXPECT_EQ(-32768, sample.offset);             // [確認_正常系] - 16 ビット符号付きが復元されること。
    EXPECT_EQ(255, sample.rank);                  // [確認_正常系] - 8 ビット符号なしが復元されること。
}

TEST(JsonDecodeTest, RejectsValueOutsideFieldWidth)
{
    Widths sample = {}; // [準備_異常系] - 対象の幅に収まらない値を用意する。
    int over = decode_widths("{\"rank\":256}", &sample);      // [手順_異常系] - uint8_t の範囲を超える。
    int negative = decode_widths("{\"rank\":-1}", &sample);   // [手順_異常系] - 符号なしへ負値を与える。
    int wide = decode_widths("{\"offset\":40000}", &sample);  // [手順_異常系] - int16_t の範囲を超える。
    EXPECT_EQ(CPLAT_ERR_OUT_OF_RANGE, over);     // [確認_異常系] - 幅を超える値が拒否されること。
    EXPECT_EQ(CPLAT_ERR_OUT_OF_RANGE, negative); // [確認_異常系] - 符号なしへの負値が拒否されること。
    EXPECT_EQ(CPLAT_ERR_OUT_OF_RANGE, wide);     // [確認_異常系] - 符号付きの範囲外が拒否されること。
}

TEST(JsonDecodeTest, RejectsNonIntegerAndUnrepresentableNumber)
{
    Widths sample = {}; // [準備_異常系] - 整数でない値と表現できない値を用意する。
    int fraction = decode_widths("{\"wide\":1.5}", &sample);  // [手順_異常系] - 小数を与える。
    int huge = decode_widths("{\"wide\":1e300}", &sample);    // [手順_異常系] - double の範囲の巨大値を与える。
    EXPECT_EQ(CPLAT_ERR_OUT_OF_RANGE, fraction); // [確認_異常系] - 小数が拒否されること。
    EXPECT_EQ(CPLAT_ERR_OUT_OF_RANGE, huge);     // [確認_異常系] - 表現できない値が拒否されること。
}

TEST(JsonDecodeTest, UsesGenericJsonAttributes)
{
    cJSON *json = cJSON_Parse("{\"person_id\":42}");
    Sample sample = {0, 9}; // [準備_正常系] - 任意フィールドに既存値を持たせる。
    ASSERT_NE(nullptr, json);
    int actual = struct_meta_json_decode(&kDescriptor, json, &sample); // [手順_正常系]
    EXPECT_EQ(CPLAT_OK, actual); // [確認_正常系] - 必須の別名キーを読み込めること。
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
    EXPECT_EQ(CPLAT_ERR_MISSING_REQUIRED, actual);                     // [確認_異常系] - 必須キー欠落を報告すること。
    cJSON_Delete(json);
}
