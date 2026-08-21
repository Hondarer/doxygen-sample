/**
 *******************************************************************************
 *  @file           sample_types.h
 *  @brief          struct-json-sample コマンドが JSON と相互変換する構造体を定義します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  本ヘッダーは structgen (`../structgen/`) の解析対象そのものです。\n
 *  JSON 変換用に構造体を二重定義しないため、プログラム本体もこのヘッダーを
 *  そのまま使用します。structgen が解析できるのは `typedef struct { ... } Name;`
 *  の形の宣言だけです。関数プロトタイプなど、それ以外の宣言は含めないでください
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
    char city[32];
    int zip;
} address;

/**
 *  @brief          動作確認に使う構造体です。ネスト構造体・固定長配列メンバーを含みます。
 */
typedef struct person
{
    int id;
    unsigned int age;
    double score;
    char name[64];
    address home;
    address addresses[2];
    int scores[3];
} person;

#endif /* SAMPLE_TYPES_H */
