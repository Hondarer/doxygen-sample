/**
 *******************************************************************************
 *  @file           structgen_ast.h
 *  @brief          structgen が構造体ヘッダーの解析結果を保持する AST を定義します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  structgen はフル C パーサーではなく、`typedef struct { ... } Name;` の形に
 *  限定した宣言だけを解析対象とします。詳細は
 *  `app/struct-json-sample/docs/architecture.md` を参照してください。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#ifndef STRUCTGEN_AST_H
#define STRUCTGEN_AST_H

/**
 *  @brief          型指定 1 個分の解析結果です (bison の型付き値として使用)。
 */
typedef struct sg_typespec
{
    char *name;      /**< 型のスペリングです ("int"/"unsigned"/"char"/"float"/"double"、
                       *   または他の `typedef struct` の名前 (ネスト メンバー))。 */
    int is_struct;   /**< 1 なら `name` は同一ヘッダー内の構造体名 (ネスト メンバー) です。 */
    int pad;         /**< 明示的アラインメント (構造体全体を 8 バイト境界へ揃える)。 */
} sg_typespec;

/**
 *  @brief          構造体 1 フィールド分の解析結果です。
 */
typedef struct sg_field
{
    char *name;         /**< フィールド名です。 */
    char *type_name;    /**< 型のスペリングです (プリミティブの綴り、または構造体名)。 */
    long array_count;   /**< `[N]` の N です。スカラー フィールドは 0 です。 */
    int line;            /**< ソース上の行番号です (診断メッセージ用)。 */
    int is_struct_type;  /**< 1 なら `type_name` は同一ヘッダー内の構造体名 (ネスト メンバー) です。 */
    struct sg_field *next;
} sg_field;

/**
 *  @brief          `typedef struct { ... } Name;` 1 個分の解析結果です。
 */
typedef struct sg_struct
{
    char *name;          /**< 構造体名 (typedef 名) です。 */
    sg_field *fields;     /**< フィールドの連結リストです。 */
    struct sg_struct *next;
} sg_struct;

/**
 *  @brief          フィールド リストを構築するための一時ハンドルです (末尾ポインターを保持)。
 */
typedef struct sg_field_list
{
    sg_field *head;
    sg_field *tail;
} sg_field_list;

/**
 *  @brief          構造体リストを構築するための一時ハンドルです (末尾ポインターを保持)。
 */
typedef struct sg_struct_list
{
    sg_struct *head;
    sg_struct *tail;
} sg_struct_list;

sg_field *sg_field_create(char *name, char *type_name, int is_struct_type, long array_count, int line);
sg_field_list *sg_field_list_create(sg_field *first);
sg_field_list *sg_field_list_append(sg_field_list *list, sg_field *field);

sg_struct *sg_struct_create(char *name, sg_field_list *fields);
void sg_struct_list_append(sg_struct_list **list, sg_struct *s);
const sg_struct *sg_struct_list_find(const sg_struct_list *list, const char *name);

#endif /* STRUCTGEN_AST_H */
