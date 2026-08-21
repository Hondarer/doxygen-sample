/**
 *******************************************************************************
 *  @file           struct_json_meta.h
 *  @brief          C 構造体をメタデータとして記述する記述子データモデルを定義します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  C 構造体には実行時リフレクションがないため、構造体と JSON の相互変換や
 *  対話形式でのフィールド編集には、型・フィールド名・ネスト・配列構成を
 *  表す何らかのメタデータが必要です。\n
 *  本ヘッダーが定義する @ref sj_struct_desc / @ref sj_field_desc は、その
 *  メタデータのデータモデルです。\n
 *  `structgen` (ヘッダー解析ツール) は、プログラム本体が実際に使う構造体
 *  ヘッダーを解析し、この記述子を初期化する C ソースを生成します。\n
 *  フィールドのオフセット (@ref sj_field_desc::offset) とサイズは、
 *  `structgen` 自身が計算するのではなく、生成コードに埋め込んだ
 *  `offsetof`/`sizeof` を実ヘッダーに対してコンパイラへ計算させることで
 *  求めます。パディングやアラインメントの違いを気にする必要がありません。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#ifndef STRUCT_JSON_META_H
#define STRUCT_JSON_META_H

#include <stddef.h>

/**
 *  @defgroup       STRUCT_JSON_PUBLIC_API 公開 API (struct_json)
 *  @brief          struct_json ライブラリの公開 API です。
 */

/**
 *  @ingroup        STRUCT_JSON_PUBLIC_API
 *  @{
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     *  @brief          フィールドの値の種別です。
     */
    typedef enum sj_field_kind
    {
        SJ_FIELD_INT,        /**< int (符号あり整数) */
        SJ_FIELD_UNSIGNED,   /**< unsigned (符号なし整数) */
        SJ_FIELD_FLOAT,      /**< float */
        SJ_FIELD_DOUBLE,     /**< double */
        SJ_FIELD_CHAR_ARRAY, /**< char 配列。NUL 終端文字列として JSON 文字列と相互変換します。 */
        SJ_FIELD_STRUCT      /**< ネストした構造体。@ref sj_field_desc::nested を参照します。 */
    } sj_field_kind;

    typedef struct sj_struct_desc sj_struct_desc;

    /**
     *  @brief          構造体 1 フィールド分の記述子です。
     */
    typedef struct sj_field_desc
    {
        const char *name;   /**< JSON オブジェクトのキー名 (= C のフィールド名) です。 */
        sj_field_kind kind; /**< フィールドの値の種別です。 */
        unsigned int pad;   /**< 明示的アラインメント (次の size_t メンバーを 8 バイト境界へ揃える)。 */
        size_t offset;      /**< 構造体先頭からのバイト オフセットです (`offsetof`)。 */
        size_t elem_size;   /**< 要素 1 個分のバイト サイズです (`sizeof`)。 */

        /**
         *  @brief          配列要素数です。
         *
         *  スカラー フィールドは 1 を設定します。固定長配列メンバーは
         *  ソース上の `[N]` の N を設定します。
         */
        size_t array_count;

        /**
         *  @brief          `kind` が @ref SJ_FIELD_CHAR_ARRAY のときの、配列全体のバイト サイズです。
         *
         *  `char name[N]` の `sizeof(name)` (= N) です。JSON 文字列との相互変換時の
         *  バッファー境界チェックに使用します。他の `kind` では 0 です。
         */
        size_t char_buf_size;

        /**
         *  @brief          `kind` が @ref SJ_FIELD_STRUCT のときの、ネストした構造体の記述子です。
         *
         *  他の `kind` では NULL です。
         */
        const sj_struct_desc *nested;

        /**
         *  @brief          フィールドの短い説明です。
         *
         *  structgen が Doxygen の前置 `@brief` または後置コメントから埋めます。\n
         *  無いときは NULL です。JSON 変換には使いません。
         */
        const char *brief;

        /**
         *  @brief          JSON オブジェクトのキー名です。
         *
         *  NULL または空のときは @ref sj_field_desc::name を使います。\n
         *  dump / patch の表示には使いません。
         */
        const char *json_name;

        /**
         *  @brief          1 なら JSON の読み書きから外します。
         *
         *  dump / patch の対象には残します。
         */
        int json_ignore;

        /**
         *  @brief          1 なら JSON のキー欠落をエラーにします。
         *
         *  0 のときは欠落しても値を触らず成功します。
         */
        int json_required;
    } sj_field_desc;

    /**
     *  @brief          構造体 1 つ分の記述子です。
     */
    struct sj_struct_desc
    {
        const char *name;            /**< 構造体名です (診断メッセージ用)。 */
        size_t size;                 /**< 構造体全体のバイト サイズです (`sizeof`)。 */
        const sj_field_desc *fields; /**< フィールド記述子の配列です。 */
        size_t field_count;          /**< `fields` の要素数です。 */

        /**
         *  @brief          構造体の短い説明です。
         *
         *  structgen が Doxygen の前置 `@brief` または後置コメントから埋めます。\n
         *  無いときは NULL です。JSON 変換には使いません。
         */
        const char *brief;
    };

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* STRUCT_JSON_META_H */
