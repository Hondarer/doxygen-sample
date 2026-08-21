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
    char *name;    /**< 型のスペリングです ("int"/"unsigned"/"char"/"float"/"double"、
                       *   または他の `typedef struct` の名前 (ネスト メンバー))。 */
    int is_struct; /**< 1 なら `name` は同一ヘッダー内の構造体名 (ネスト メンバー) です。 */
    int pad;       /**< 明示的アラインメント (構造体全体を 8 バイト境界へ揃える)。 */
} sg_typespec;

/**
 *  @brief          構造体 1 フィールド分の解析結果です。
 */
typedef struct sg_field
{
    char *name;         /**< フィールド名です。 */
    char *type_name;    /**< 型のスペリングです (プリミティブの綴り、または構造体名)。 */
    char *brief;        /**< Doxygen から取り出した短い説明です。無いときは NULL です。 */
    char *json_name;    /**< `@json_name{...}` の値です。無いときは NULL です。 */
    int json_ignore;    /**< `@json_ignore` があれば 1 です。 */
    int json_required;  /**< `@json_required` があれば 1 です。 */
    long array_count;   /**< `[N]` の N です。スカラー フィールドは 0 です。 */
    int line;           /**< ソース上の行番号です (診断メッセージ用)。 */
    int is_struct_type; /**< 1 なら `type_name` は同一ヘッダー内の構造体名 (ネスト メンバー) です。 */
    struct sg_field *next;
} sg_field;

/**
 *  @brief          `typedef struct { ... } Name;` 1 個分の解析結果です。
 */
typedef struct sg_struct
{
    char *name;       /**< 構造体名 (typedef 名) です。 */
    char *brief;      /**< Doxygen から取り出した短い説明です。無いときは NULL です。 */
    sg_field *fields; /**< フィールドの連結リストです。 */
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

/**
 *  @brief          1 コメントから取り出した説明と JSON タグです。
 */
typedef struct sg_doc_attrs
{
    char *brief;
    char *json_name;
    int json_ignore;
    int json_required;
    int has_json_name;
    int has_json_ignore;
    int has_json_required;
    int pad;
} sg_doc_attrs;

sg_field *sg_field_create(char *name, char *type_name, int is_struct_type, long array_count, int line, char *brief,
                          char *json_name, int json_ignore, int json_required);
sg_field_list *sg_field_list_create(sg_field *first);
sg_field_list *sg_field_list_append(sg_field_list *list, sg_field *field);

sg_struct *sg_struct_create(char *name, sg_field_list *fields, char *brief);
void sg_struct_list_append(sg_struct_list **list, sg_struct *s);
const sg_struct *sg_struct_list_find(const sg_struct_list *list, const char *name);

/**
 *  @brief          Doxygen コメントが `@file` タグを持つかどうかを返します。
 */
int sg_doc_has_file_tag(const char *raw);

/**
 *  @brief          Doxygen コメントから `brief` 文字列を取り出します。
 *
 *  前置は `@brief` 本文だけを取ります。後置はコメント本体を `brief` とします。\n
 *  後置に `@brief` があればその本文を優先します。取れないときは NULL です。
 */
char *sg_brief_from_doc(const char *raw, int is_postfix);

/**
 *  @brief          前置と後置の `brief` から、採用する一方を返します。
 *
 *  後置があれば後置を返して前置を解放します。後置が無ければ前置を返します。
 */
char *sg_brief_choose(char *prefix_brief, char *postfix_brief);

/**
 *  @brief          連続する Doxygen コメントの原文を改行でつなぎます。
 *
 *  引数の所有権を受け取り、つないだ新しい文字列を返します。
 */
char *sg_doc_concat(char *first, char *second);

/**
 *  @brief          コメント原文から brief と JSON タグを取り出します。
 */
sg_doc_attrs sg_doc_attrs_from_raw(const char *raw, int is_postfix);

/**
 *  @brief          前置と後置の属性をマージします。同じタグは後置を優先します。
 */
sg_doc_attrs sg_doc_attrs_choose(sg_doc_attrs prefix, sg_doc_attrs postfix);

#endif /* STRUCTGEN_AST_H */
