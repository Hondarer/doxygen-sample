/**
 *******************************************************************************
 *  @file           sample_types.h
 *  @brief          struct-meta-sample コマンドが JSON と相互変換する構造体を定義します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  本ヘッダーは struct-meta-gen (`../struct-meta-gen/`) の解析対象そのものです。\n
 *  コマンドは生成した記述子だけを使い、このヘッダーの型名は直接参照しません。\n
 *  struct-meta-gen が解析できるのは `typedef struct { ... } Name;` の形の宣言だけです。
 *  関数プロトタイプなど、それ以外の宣言は含めないでください
 *  (詳細は `app/struct-meta/docs/architecture.md` を参照)。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#ifndef SAMPLE_TYPES_PRIVATE_H
#define SAMPLE_TYPES_PRIVATE_H

#include <stdint.h>

/**
 *  @brief          住所を表す、`person` のネスト メンバーです。
 *  @struct_meta{sample.category=location}
 */
typedef struct address
{
    char city[32]; /**< 都市名です。 @struct_meta{json.name=locality} */
    int zip;       /**< 郵便番号です。 */
} address;

/**
 *  @brief          動作確認に使う構造体です。ネスト構造体・固定長配列メンバーを含みます。
 *  @struct_meta{sample.category=person}
 */
typedef struct person
{
    int id;               /**< 識別子です。 @struct_meta{json.name=person_id} @struct_meta{json.required} */
    unsigned int age;     /**< 年齢です。 */
    double score;         /**< 得点です。 */
    char name[64];        /**< 氏名です。 */
    address home;         /**< 自宅です。 */
    address addresses[2]; /**< 追加の住所です。 */
    int scores[3];        /**< 得点の配列です。 */
    int64_t balance;      /**< 残高です。64 ビット符号付きの動作確認を兼ねます。 */
    long long counter;    /**< 通算回数です。`long long` の動作確認を兼ねます。 */
    uint32_t flags;       /**< 属性ビットです。32 ビット符号なしの動作確認を兼ねます。 */
    int16_t offset;       /**< 補正値です。16 ビット符号付きの動作確認を兼ねます。 */
    uint8_t rank;         /**< 等級です。8 ビット符号なしの動作確認を兼ねます。 */
    uint8_t reserved;     /**< 明示的アラインメントです。 @struct_meta{json.ignore} */
    int serial;           /**< 内部連番です。 @struct_meta{json.ignore} */
    int pad;              /**< 明示的アラインメントです。 @struct_meta{json.ignore} */
} person;

#endif /* SAMPLE_TYPES_PRIVATE_H */
