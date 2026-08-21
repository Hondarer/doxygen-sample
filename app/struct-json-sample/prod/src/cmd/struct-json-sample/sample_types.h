/**
 *******************************************************************************
 *  @file           sample_types.h
 *  @brief          struct-json-sample コマンドが JSON と相互変換する構造体を定義します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  本ヘッダーは structgen (`../structgen/`) の解析対象そのものです。\n
 *  コマンドは生成した記述子だけを使い、このヘッダーの型名は直接参照しません。\n
 *  structgen が解析できるのは `typedef struct { ... } Name;` の形の宣言だけです。
 *  関数プロトタイプなど、それ以外の宣言は含めないでください
 *  (詳細は `app/struct-json-sample/docs/architecture.md` を参照)。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#ifndef SAMPLE_TYPES_H
#define SAMPLE_TYPES_H

/**
 *  @brief          住所を表す、`person` のネスト メンバーです。
 */
typedef struct address
{
    char city[32]; /**< 都市名です。 @json_name{locality} */
    int zip;       /**< 郵便番号です。 */
} address;

/**
 *  @brief          動作確認に使う構造体です。ネスト構造体・固定長配列メンバーを含みます。
 */
typedef struct person
{
    int id;               /**< 識別子です。 @json_name{person_id} @json_required */
    unsigned int age;     /**< 年齢です。 */
    double score;         /**< 得点です。 */
    char name[64];        /**< 氏名です。 */
    address home;         /**< 自宅です。 */
    address addresses[2]; /**< 追加の住所です。 */
    int scores[3];        /**< 得点の配列です。 */
    int serial; /**< 内部連番です。 @json_ignore */
    int pad;    /**< 明示的アラインメントです。 @json_ignore */
} person;

#endif /* SAMPLE_TYPES_H */
