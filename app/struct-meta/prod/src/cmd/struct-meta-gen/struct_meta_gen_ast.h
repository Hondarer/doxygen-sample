/**
 *******************************************************************************
 *  @file           struct_meta_gen_ast.h
 *  @brief          struct-meta-gen が構造体ヘッダーの解析結果を保持する AST を定義します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  struct-meta-gen はフル C パーサーではなく、`typedef struct { ... } Name;` の形に
 *  限定した宣言だけを解析対象とします。詳細は
 *  `app/struct-meta/docs/architecture.md` を参照してください。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#ifndef STRUCT_META_GEN_AST_H
#define STRUCT_META_GEN_AST_H

/**
 *  @brief          型指定 1 個分の解析結果です (bison の型付き値として使用)。
 */
typedef struct struct_meta_gen_typespec
{
    char *name;    /**< 型のスペリングです ("int"/"unsigned"/"char"/"float"/"double"、
                       *   または他の `typedef struct` の名前 (ネスト メンバー))。 */
    int is_struct; /**< 1 なら `name` は同一ヘッダー内の構造体名 (ネスト メンバー) です。 */
    int pad;       /**< 明示的アラインメント (構造体全体を 8 バイト境界へ揃える)。 */
} struct_meta_gen_typespec;

/**
 *  @brief          構造体 1 フィールド分の解析結果です。
 */
typedef struct struct_meta_gen_field
{
    char *name;                                   /**< フィールド名です。 */
    char *type_name;                              /**< 型のスペリングです (プリミティブの綴り、または構造体名)。 */
    char *brief;                                  /**< Doxygen から取り出した短い説明です。無いときは NULL です。 */
    struct struct_meta_gen_attribute *attributes; /**< Doxygen から取り出した汎用属性です。 */
    long array_count;                             /**< `[N]` の N です。スカラー フィールドは 0 です。 */
    int line;                                     /**< ソース上の行番号です (診断メッセージ用)。 */
    int is_struct_type; /**< 1 なら `type_name` は同一ヘッダー内の構造体名 (ネスト メンバー) です。 */
    struct struct_meta_gen_field *next;
} struct_meta_gen_field;

/**
 *  @brief          `typedef struct { ... } Name;` 1 個分の解析結果です。
 */
typedef struct struct_meta_gen_struct
{
    char *name;                                   /**< 構造体名 (typedef 名) です。 */
    char *brief;                                  /**< Doxygen から取り出した短い説明です。無いときは NULL です。 */
    struct struct_meta_gen_attribute *attributes; /**< Doxygen から取り出した汎用属性です。 */
    struct_meta_gen_field *fields;                /**< フィールドの連結リストです。 */
    struct struct_meta_gen_struct *next;
} struct_meta_gen_struct;

/**
 *  @brief          `@struct_meta{key}` または `@struct_meta{key=value}` 1 個分の解析結果です。
 */
typedef struct struct_meta_gen_attribute
{
    char *key;   /**< 属性名です。 */
    char *value; /**< 属性値です。値を持たない場合は NULL です。 */
    struct struct_meta_gen_attribute *next;
} struct_meta_gen_attribute;

/**
 *  @brief          フィールド リストを構築するための一時ハンドルです (末尾ポインターを保持)。
 */
typedef struct struct_meta_gen_field_list
{
    struct_meta_gen_field *head;
    struct_meta_gen_field *tail;
} struct_meta_gen_field_list;

/**
 *  @brief          構造体リストを構築するための一時ハンドルです (末尾ポインターを保持)。
 */
typedef struct struct_meta_gen_struct_list
{
    struct_meta_gen_struct *head;
    struct_meta_gen_struct *tail;
} struct_meta_gen_struct_list;

/**
 *  @brief          1 コメントから取り出した説明と汎用属性です。
 */
typedef struct struct_meta_gen_doc_attrs
{
    char *brief;
    struct_meta_gen_attribute *attributes;
    int invalid;
    int pad;
} struct_meta_gen_doc_attrs;

struct_meta_gen_field *struct_meta_gen_field_create(char *name, char *type_name, int is_struct_type, long array_count,
                                                    int line, char *brief, struct_meta_gen_attribute *attributes);
struct_meta_gen_field_list *struct_meta_gen_field_list_create(struct_meta_gen_field *first);
struct_meta_gen_field_list *struct_meta_gen_field_list_append(struct_meta_gen_field_list *list,
                                                              struct_meta_gen_field *field);

struct_meta_gen_struct *struct_meta_gen_struct_create(char *name, struct_meta_gen_field_list *fields, char *brief,
                                                      struct_meta_gen_attribute *attributes);
void struct_meta_gen_struct_list_append(struct_meta_gen_struct_list **list, struct_meta_gen_struct *s);
const struct_meta_gen_struct *struct_meta_gen_struct_list_find(const struct_meta_gen_struct_list *list,
                                                               const char *name);

/**
 *  @brief          Doxygen コメントが `@file` タグを持つかどうかを返します。
 */
int struct_meta_gen_doc_has_file_tag(const char *raw);

/**
 *  @brief          Doxygen コメントから `brief` 文字列を取り出します。
 *
 *  前置は `@brief` 本文だけを取ります。後置はコメント本体を `brief` とします。\n
 *  後置に `@brief` があればその本文を優先します。取れないときは NULL です。
 */
char *struct_meta_gen_brief_from_doc(const char *raw, int is_postfix);

/**
 *  @brief          前置と後置の `brief` から、採用する一方を返します。
 *
 *  後置があれば後置を返して前置を解放します。後置が無ければ前置を返します。
 */
char *struct_meta_gen_brief_choose(char *prefix_brief, char *postfix_brief);

/**
 *  @brief          連続する Doxygen コメントの原文を改行でつなぎます。
 *
 *  引数の所有権を受け取り、つないだ新しい文字列を返します。
 */
char *struct_meta_gen_doc_concat(char *first, char *second);

/**
 *  @brief          コメント原文から brief と汎用属性を取り出します。
 */
struct_meta_gen_doc_attrs struct_meta_gen_doc_attrs_from_raw(const char *raw, int is_postfix, int line);

/**
 *  @brief          前置と後置の属性をマージします。同じ属性名は不正とします。
 */
struct_meta_gen_doc_attrs struct_meta_gen_doc_attrs_choose(struct_meta_gen_doc_attrs prefix,
                                                           struct_meta_gen_doc_attrs postfix, int line);

#endif /* STRUCT_META_GEN_AST_H */
