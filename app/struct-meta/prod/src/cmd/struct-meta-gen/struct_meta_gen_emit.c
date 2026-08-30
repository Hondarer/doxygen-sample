/**
 *******************************************************************************
 *  @file           struct_meta_gen_emit.c
 *  @brief          解析済みの構造体定義から、メタデータ記述子の C ソースを生成します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  フィールドのオフセットとサイズは、生成コードに埋め込んだ `offsetof`/`sizeof` を
 *  実ヘッダーに対してコンパイラへ計算させます。struct-meta-gen 自身はレイアウトを
 *  計算しません (`docs/architecture.md` の設計方針を参照)。
 *
 *  ネスト構造体メンバーがある場合、参照先の構造体記述子を先に出力してから
 *  (依存順)、それを参照する構造体の記述子を出力します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include "struct_meta_gen_emit.h"
#include "struct_meta_gen_emit_array.h"

#include <cplat/base/result.h>
#include <cplat/crt/stdio.h>
#include <cplat/hashtable/hashtable.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct_meta_gen_struct_list *g_struct_meta_gen_structs;

/** 生成パス・識別子を組み立てる作業バッファーのバイト数です。 */
#define STRUCT_META_GEN_EMIT_PATH_BYTES 512

/**
 *  @brief          記述子をすでに出力した構造体名を記録する連結リストです。
 */
typedef struct emitted_name
{
    const char *name;
    struct emitted_name *next;
} emitted_name;

static int is_emitted(const emitted_name *list, const char *name)
{
    for (; list != NULL; list = list->next)
    {
        if (strcmp(list->name, name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 *  @brief          フィールドの型スペリングから @ref struct_meta_field_kind の列挙定数名を求めます。
 *
 *  型名と列挙定数名の対応は @ref struct_meta_gen_find_scalar_type が持つ表を正本とします。
 */
static const char *field_kind_name(const char *type_name, int is_char_array)
{
    if (is_char_array)
    {
        return "STRUCT_META_FIELD_CHAR_ARRAY";
    }

    const struct_meta_gen_scalar_type *scalar = struct_meta_gen_find_scalar_type(type_name);
    if (scalar != NULL)
    {
        return scalar->kind_name;
    }
    /* 文法規則が受理する非構造体型は、char か表にある型に限られる。 */
    return "STRUCT_META_FIELD_CHAR_ARRAY";
}

static const struct_meta_gen_attribute *field_attribute(const struct_meta_gen_field *field, const char *key)
{
    for (const struct_meta_gen_attribute *attribute = field->attributes; attribute != NULL; attribute = attribute->next)
    {
        if (strcmp(attribute->key, key) == 0)
        {
            return attribute;
        }
    }
    return NULL;
}

static int is_explicit_byte_type(const char *type_name)
{
    return ((strcmp(type_name, "signed char") == 0) || (strcmp(type_name, "unsigned char") == 0) ||
            (strcmp(type_name, "int8_t") == 0) || (strcmp(type_name, "uint8_t") == 0))
               ? 1
               : 0;
}

static int field_is_byte_array(const struct_meta_gen_field *field)
{
    if (field->array_count <= 0)
    {
        return 0;
    }
    if (is_explicit_byte_type(field->type_name) != 0)
    {
        return 1;
    }
    const struct_meta_gen_attribute *kind = field_attribute(field, "meta.kind");
    return ((strcmp(field->type_name, "char") == 0) && (kind != NULL) && (kind->value != NULL) &&
            (strcmp(kind->value, "bytes") == 0))
               ? 1
               : 0;
}

static void validate_meta_attributes(const struct_meta_gen_field *field)
{
    const struct_meta_gen_attribute *kind = field_attribute(field, "meta.kind");
    if ((kind != NULL) &&
        ((kind->value == NULL) || (strcmp(kind->value, "bytes") != 0) || (field_is_byte_array(field) == 0)))
    {
        fprintf(stderr, "struct-meta-gen: %d: meta.kind はバイト配列へ bytes だけを指定できます: %s\n", field->line,
                field->name);
        exit(1);
    }

    const struct_meta_gen_attribute *format = field_attribute(field, "meta.format");
    if ((format != NULL) &&
        ((format->value == NULL) || (strcmp(format->value, "hex") != 0) || (field_is_byte_array(field) == 0)))
    {
        fprintf(stderr, "struct-meta-gen: %d: meta.format=hex はバイト配列だけへ指定できます: %s\n", field->line,
                field->name);
        exit(1);
    }
}

/**
 *  @brief          フィールドの型スペリングから、要素 1 個分の `sizeof` 式を求めます。
 */
static const char *elem_sizeof_expr(const char *type_name)
{
    const struct_meta_gen_scalar_type *scalar = struct_meta_gen_find_scalar_type(type_name);
    if (scalar != NULL)
    {
        return scalar->sizeof_expr;
    }
    return "sizeof(char)";
}

static int count_fields(const struct_meta_gen_struct *s)
{
    int count = 0;
    for (const struct_meta_gen_field *f = s->fields; f != NULL; f = f->next)
    {
        count++;
    }
    return count;
}

/**
 *  @brief          パスの末尾ファイル名を返します (`/` と `\\` を区切りとします)。
 */
static const char *path_basename(const char *path)
{
    const char *base = path;
    for (const char *p = path; *p != '\0'; p++)
    {
        if ((*p == '/') || (*p == '\\'))
        {
            base = p + 1;
        }
    }
    return base;
}

/**
 *  @brief          ヘッダー パスから識別子用 stem を取り出します (`sample_types.h` → `sample_types`)。
 *
 *  英数字と `_` 以外は `_` に置換します。先頭が数字のときは先頭に `x` を付けます。
 */
static int header_stem(const char *header_path, char *dest, size_t dest_size)
{
    const char *base = path_basename(header_path);
    char raw[STRUCT_META_GEN_EMIT_PATH_BYTES];
    size_t raw_len = 0;

    for (const char *p = base; (*p != '\0') && (*p != '.'); p++)
    {
        if (raw_len + 1U >= sizeof(raw))
        {
            return 1;
        }
        if (isalnum((unsigned char)*p) != 0)
        {
            raw[raw_len] = *p;
        }
        else
        {
            raw[raw_len] = '_';
        }
        raw_len++;
    }
    raw[raw_len] = '\0';

    if (raw_len == 0U)
    {
        return 1;
    }
    if (isdigit((unsigned char)raw[0]) != 0)
    {
        if (dest_size < (raw_len + 2U))
        {
            return 1;
        }
        dest[0] = 'x';
        memcpy(dest + 1, raw, raw_len + 1U);
        return 0;
    }
    if (dest_size < (raw_len + 1U))
    {
        return 1;
    }
    memcpy(dest, raw, raw_len + 1U);
    return 0;
}

/**
 *  @brief          stem を全大文字にした接頭辞を作ります (`sample_types` → `SAMPLE_TYPES`)。
 */
static int stem_prefix(const char *stem, char *dest, size_t dest_size)
{
    size_t i;
    for (i = 0; stem[i] != '\0'; i++)
    {
        if ((i + 1U) >= dest_size)
        {
            return 1;
        }
        dest[i] = (char)toupper((unsigned char)stem[i]);
    }
    dest[i] = '\0';
    return 0;
}

/**
 *  @brief          `--out` の `.c` から同名の `.h` パスを導出します。
 */
static int header_out_path(const char *c_path, char *header_path_out, size_t header_path_size)
{
    size_t len = strlen(c_path);
    if ((len < 2U) || (c_path[len - 2U] != '.') || (c_path[len - 1U] != 'c'))
    {
        return 1;
    }
    if ((len + 1U) > header_path_size)
    {
        return 1;
    }
    memcpy(header_path_out, c_path, len - 1U);
    header_path_out[len - 1U] = 'h';
    header_path_out[len] = '\0';
    return 0;
}

/**
 *  @brief          生成ヘッダーのファイル名だけを返します (`gen/foo.h` → `foo.h`)。
 */
static const char *generated_header_include(const char *header_out)
{
    return path_basename(header_out);
}

static void fprint_c_string(FILE *out, const char *text)
{
    if (text == NULL)
    {
        fputs("NULL", out);
        return;
    }

    fputc('"', out);
    for (const char *p = text; *p != '\0'; p++)
    {
        if ((*p == '"') || (*p == '\\'))
        {
            fputc('\\', out);
            fputc(*p, out);
        }
        else if (*p == '\n')
        {
            fputs("\\n", out);
        }
        else
        {
            fputc(*p, out);
        }
    }
    fputc('"', out);
}

static int count_structs(const struct_meta_gen_struct_list *list)
{
    int count = 0;
    for (const struct_meta_gen_struct *s = list->head; s != NULL; s = s->next)
    {
        count++;
    }
    return count;
}

static int count_attributes(const struct_meta_gen_attribute *attributes)
{
    int count = 0;
    for (const struct_meta_gen_attribute *attribute = attributes; attribute != NULL; attribute = attribute->next)
    {
        count++;
    }
    return count;
}

static void emit_attributes(FILE *out, const char *symbol, const struct_meta_gen_attribute *attributes)
{
    if (attributes == NULL)
    {
        return;
    }

    fprintf(out, "static const struct_meta_attribute g_%s_attributes[] = {\n", symbol);
    for (const struct_meta_gen_attribute *attribute = attributes; attribute != NULL; attribute = attribute->next)
    {
        fputs("    { ", out);
        fprint_c_string(out, attribute->key);
        fputs(", ", out);
        fprint_c_string(out, attribute->value);
        fputs(" },\n", out);
    }
    fputs("};\n\n", out);
}

/**
 *  @brief          構造体 1 個分の記述子を再帰的に出力します。
 *
 *  ネスト構造体メンバーがあれば、参照先の記述子を先に出力してから
 *  (依存順)、`s` 自身の記述子を出力します。同じ構造体は 2 回出力しません。
 *  記述子はすべて `static` です。外部からは型一覧の取得関数だけが見えます。
 */
static void emit_struct(FILE *out, const struct_meta_gen_struct *s, emitted_name **emitted)
{
    if (is_emitted(*emitted, s->name))
    {
        return;
    }

    for (const struct_meta_gen_field *f = s->fields; f != NULL; f = f->next)
    {
        if (f->is_struct_type)
        {
            const struct_meta_gen_struct *nested =
                struct_meta_gen_struct_list_find(g_struct_meta_gen_structs, f->type_name);
            if (nested == NULL)
            {
                fprintf(stderr, "struct-meta-gen: %d: 未知の型です: %s\n", f->line, f->type_name);
                exit(1);
            }
            emit_struct(out, nested, emitted);
        }
    }

    for (const struct_meta_gen_field *f = s->fields; f != NULL; f = f->next)
    {
        validate_meta_attributes(f);
        char symbol[STRUCT_META_GEN_EMIT_PATH_BYTES];
        snprintf(symbol, sizeof(symbol), "%s_%s", s->name, f->name);
        emit_attributes(out, symbol, f->attributes);
    }
    emit_attributes(out, s->name, s->attributes);

    fprintf(out, "static const struct_meta_field g_%s_fields[] = {\n", s->name);
    for (const struct_meta_gen_field *f = s->fields; f != NULL; f = f->next)
    {
        int is_char_array = ((!f->is_struct_type) && (strcmp(f->type_name, "char") == 0) && (f->array_count > 0) &&
                             (field_is_byte_array(f) == 0));
        long array_count_out = 1;
        char char_buf_expr[256] = "0";
        char elem_size_expr[128] = "0";
        char nested_expr[256] = "NULL";
        const char *kind;

        if (is_char_array)
        {
            snprintf(char_buf_expr, sizeof(char_buf_expr), "sizeof(((%s *)0)->%s)", s->name, f->name);
        }
        else if (f->array_count > 0)
        {
            array_count_out = f->array_count;
        }

        if (f->is_struct_type)
        {
            kind = "STRUCT_META_FIELD_STRUCT";
            snprintf(elem_size_expr, sizeof(elem_size_expr), "sizeof(%s)", f->type_name);
            snprintf(nested_expr, sizeof(nested_expr), "&g_%s_desc", f->type_name);
        }
        else
        {
            kind = field_kind_name(f->type_name, is_char_array);
            snprintf(elem_size_expr, sizeof(elem_size_expr), "%s", elem_sizeof_expr(f->type_name));
        }

        int attribute_count = count_attributes(f->attributes);
        fprintf(out, "    { \"%s\", %s, 0, offsetof(%s, %s), %s, %ld, %s, %s, ", f->name, kind, s->name, f->name,
                elem_size_expr, array_count_out, char_buf_expr, nested_expr);
        fprint_c_string(out, f->brief);
        if (attribute_count == 0)
        {
            fputs(", NULL, 0 },\n", out);
        }
        else
        {
            fprintf(out, ", g_%s_%s_attributes, %d },\n", s->name, f->name, attribute_count);
        }
    }
    fprintf(out, "};\n\n");

    fprintf(out, "static const struct_meta_descriptor g_%s_desc = { \"%s\", sizeof(%s), g_%s_fields, %d, ", s->name,
            s->name, s->name, s->name, count_fields(s));
    fprint_c_string(out, s->brief);
    if (s->attributes == NULL)
    {
        fprintf(out, ", NULL, 0 };\n\n");
    }
    else
    {
        fprintf(out, ", g_%s_attributes, %d };\n\n", s->name, count_attributes(s->attributes));
    }

    emitted_name *node = (emitted_name *)calloc(1, sizeof(*node));
    node->name = s->name;
    node->next = *emitted;
    *emitted = node;
}

/**
 *  @brief          型一覧 enum と取得関数の宣言をヘッダーへ書き出します。
 */
static int emit_catalog_header(const char *header_out, const char *stem, const char *prefix,
                               const struct_meta_gen_struct_list *structs)
{
    FILE *out = cplat_fopen(header_out, "w", NULL);
    if (out == NULL)
    {
        fprintf(stderr, "struct-meta-gen: 出力ファイルを作成できません: %s\n", header_out);
        return 1;
    }

    int index = 0;
    int count = count_structs(structs);

    fprintf(out, "/* このファイルは struct-meta-gen が自動生成しました。手編集しないでください。 */\n");
    fprintf(out, "#ifndef %s_META_H\n", prefix);
    fprintf(out, "#define %s_META_H\n\n", prefix);
    fprintf(out, "#include <struct_meta/meta/meta.h>\n\n");
    fprintf(out, "#ifdef __cplusplus\n");
    fprintf(out, "extern \"C\"\n");
    fprintf(out, "{\n");
    fprintf(out, "#endif /* __cplusplus */\n\n");
    fprintf(out, "typedef enum %s_meta_id\n", stem);
    fprintf(out, "{\n");
    for (const struct_meta_gen_struct *s = structs->head; s != NULL; s = s->next)
    {
        fprintf(out, "    %s_META_", prefix);
        for (const char *p = s->name; *p != '\0'; p++)
        {
            fputc(toupper((unsigned char)*p), out);
        }
        fprintf(out, " = %d,\n", index);
        index++;
    }
    fprintf(out, "    %s_META_COUNT = %d,\n", prefix, count);
    fprintf(out, "} %s_meta_id;\n\n", stem);
    fprintf(out, "/**\n");
    fprintf(out, " *  @brief          カタログが持つ型の数を返します。\n");
    fprintf(out, " *  @return         型の数です。\n");
    fprintf(out, " *\n");
    fprintf(out, " *  @par            スレッド セーフ\n");
    fprintf(out, " *  本関数はスレッド セーフです。内部に共有状態を持ちません。\n");
    fprintf(out, " */\n");
    fprintf(out, "size_t %s_meta_count(void);\n\n", stem);

    fprintf(out, "/**\n");
    fprintf(out, " *  @brief          列挙 ID で記述子を取得します。\n");
    fprintf(out, " *  @param[in]      id 取得する型の ID です。\n");
    fprintf(out, " *  @return         記述子です。範囲外の ID では NULL を返します。\n");
    fprintf(out, " *\n");
    fprintf(out, " *  @par            スレッド セーフ\n");
    fprintf(out, " *  本関数はスレッド セーフです。内部に共有状態を持ちません。\n");
    fprintf(out, " */\n");
    fprintf(out, "const struct_meta_descriptor *%s_meta_get(%s_meta_id id);\n\n", stem, stem);

    fprintf(out, "/**\n");
    fprintf(out, " *  @brief          構造体名で記述子を検索します。\n");
    fprintf(out, " *  @param[in]      name 検索する構造体名です。NULL を渡せます。\n");
    fprintf(out, " *  @return         記述子です。NULL または未知の名前では NULL を返します。\n");
    fprintf(out, " *\n");
    fprintf(out, " *  検索には、生成時に構築した索引の埋め込みイメージを使います。\n");
    fprintf(out, " *  イメージへの接続は初回呼び出し時に 1 回だけ行います。\n");
    fprintf(out, " *\n");
    fprintf(out, " *  @attention      埋め込みイメージへ接続できない場合、本関数はプロセスを終了させます。\n");
    fprintf(out, " *                  同一ビルドで生成したイメージが読めないという不変条件の破れであり、\n");
    fprintf(out, " *                  検索を続けても記述子を返せないためです。\n");
    fprintf(out, " *\n");
    fprintf(out, " *  @par            スレッド セーフ\n");
    fprintf(out, " *  本関数はスレッド セーフです。初回の接続は 1 回だけ実行されます。\n");
    fprintf(out, " */\n");
    fprintf(out, "const struct_meta_descriptor *%s_meta_find(const char *name);\n\n", stem);
    fprintf(out, "#ifdef __cplusplus\n");
    fprintf(out, "}\n");
    fprintf(out, "#endif /* __cplusplus */\n\n");
    fprintf(out, "#endif /* %s_META_H */\n", prefix);

    fclose(out);
    return 0;
}

/**
 *  @brief          構造体名の最大バイト数 (NUL を含む) を求めます。
 */
static size_t max_struct_name_bytes(const struct_meta_gen_struct_list *structs)
{
    size_t longest = 0;
    for (const struct_meta_gen_struct *s = structs->head; s != NULL; s = s->next)
    {
        size_t length = strlen(s->name);
        if (length > longest)
        {
            longest = length;
        }
    }
    return longest + 1U;
}

/**
 *  @brief          構造体名から添字を引くハッシュ表を構築し、永続化イメージを書き出します。
 *  @return         成功なら 0、失敗なら 1 です。
 *
 *  レコード数もキーの最大長も生成時に確定するため、実行時に構築せず、ここで構築した
 *  テーブルの永続化イメージを生成コードへ埋め込みます。実行時は接続するだけになります。
 */
static int emit_catalog_index_image(FILE *out, const char *prefix, const struct_meta_gen_struct_list *structs)
{
    cplat_hashtable_config config = {0};
    cplat_hashtable *table = NULL;
    size_t name_bytes = max_struct_name_bytes(structs);
    size_t count = (size_t)count_structs(structs);

    config.capacity = count * 2U;
    config.key_type = CPLAT_HASHTABLE_FIELD_FIXED_STRING;
    config.key_size = name_bytes;
    config.value_type = CPLAT_HASHTABLE_FIELD_FIXED_BINARY;
    config.value_size = sizeof(size_t);
    config.value_align = sizeof(size_t);
    config.lifetime = CPLAT_HASHTABLE_LIFETIME_INFINITE;

    if (cplat_hashtable_create(&config, NULL, 0, NULL, 0, &table) != CPLAT_OK)
    {
        fprintf(stderr, "struct-meta-gen: 索引テーブルを構築できません\n");
        return 1;
    }

    size_t index = 0;
    for (const struct_meta_gen_struct *s = structs->head; s != NULL; s = s->next)
    {
        if (cplat_hashtable_add(table, s->name, &index, CPLAT_HASHTABLE_ADD_DELETED_OVERWRITE) != CPLAT_OK)
        {
            fprintf(stderr, "struct-meta-gen: 索引へ型名を登録できません: %s\n", s->name);
            cplat_hashtable_dispose(table);
            return 1;
        }
        index++;
    }

    size_t mgmt_size = 0;
    size_t data_size = 0;
    const void *mgmt = NULL;
    const void *data = NULL;

    if ((cplat_hashtable_buffer_size(table, &mgmt_size, &data_size) != CPLAT_OK) ||
        (cplat_hashtable_buffer_ref(table, &mgmt, &data) != CPLAT_OK))
    {
        fprintf(stderr, "struct-meta-gen: 索引の永続化イメージを取得できません\n");
        cplat_hashtable_dispose(table);
        return 1;
    }

    fprintf(out, "/* struct-meta-gen が構築済みの索引を永続化したイメージです。手編集しないでください。 */\n");
    struct_meta_gen_emit_uint64_array(out, "s_index_mgmt", mgmt, mgmt_size);
    struct_meta_gen_emit_uint64_array(out, "s_index_data", data, data_size);

    /* 埋め込みイメージが前提とする ABI を、生成先のコンパイル時に検査する。
       生成器が動作した環境での実測値を出力する。 */
    fprintf(out, "/* 埋め込みイメージが前提とする ABI。破れた場合はコンパイル時に落とす。 */\n");
    fprintf(out, "_Static_assert(sizeof(size_t) == %zu, \"%s: 埋め込み索引は size_t %zu バイトを前提とします\");\n",
            sizeof(size_t), prefix, sizeof(size_t));
    fprintf(out, "_Static_assert(sizeof(time_t) == %zu, \"%s: 埋め込み索引は time_t %zu バイトを前提とします\");\n",
            sizeof(time_t), prefix, sizeof(time_t));
    fprintf(out,
            "_Static_assert(sizeof(cplat_hashtable_config) == %zu,\n"
            "               \"%s: 埋め込み索引は生成時と同じ cplat_hashtable_config 配置を前提とします\");\n\n",
            sizeof(cplat_hashtable_config), prefix);

    cplat_hashtable_dispose(table);
    return 0;
}

/**
 *  @brief          型一覧テーブルと取得関数を C ソースへ書き出します。
 *  @return         成功なら 0、失敗なら 1 です。
 *
 *  テーブルの並びはヘッダーの宣言順 (enum と同じ) です。記述子の出力順
 *  (依存順) とは独立です。
 */
static int emit_catalog_source(FILE *out, const char *stem, const char *prefix,
                               const struct_meta_gen_struct_list *structs)
{
    fprintf(out, "static const struct_meta_descriptor *const s_descriptors[%s_META_COUNT] = {\n", prefix);
    for (const struct_meta_gen_struct *s = structs->head; s != NULL; s = s->next)
    {
        fprintf(out, "    &g_%s_desc,\n", s->name);
    }
    fprintf(out, "};\n\n");

    if (emit_catalog_index_image(out, prefix, structs) != 0)
    {
        return 1;
    }

    fprintf(out, "static cplat_once_flag s_index_once;\n");
    fprintf(out, "static cplat_hashtable *s_index;\n\n");

    fprintf(out, "static void detach_index(const cplat_shutdown_event *event, void *context)\n");
    fprintf(out, "{\n");
    fprintf(out, "    (void)event;\n");
    fprintf(out, "    (void)context;\n");
    fprintf(out, "    for (size_t i = 0; i < %s_META_COUNT; i++)\n", prefix);
    fprintf(out, "    {\n");
    fprintf(out, "        (void)struct_meta_index_unregister(s_descriptors[i]);\n");
    fprintf(out, "    }\n");
    fprintf(out, "    /* ハンドルだけを解放する。イメージは静的領域であり解放しない。 */\n");
    fprintf(out, "    cplat_hashtable_dispose(s_index);\n");
    fprintf(out, "    s_index = NULL;\n");
    fprintf(out, "}\n\n");

    fprintf(out, "static void attach_index(void)\n");
    fprintf(out, "{\n");
    fprintf(out, "    /* イメージは読み取り専用。cplat_hashtable_attach() は領域へ書き込まず、\n");
    fprintf(out, "       この表へ書き込み API を呼ぶこともないため、const を外して渡す。\n");
    fprintf(out, "       uintptr_t を経由するのは cplat と同じ書き方に揃えるため。\n");
    fprintf(out, "       see: app/c-platform/prod/libsrc/cplat/hashtable/hashtable_create.c の\n");
    fprintf(out, "            cplat_hashtable_attach() */\n");
    fprintf(out, "    if (cplat_hashtable_attach((void *)(uintptr_t)s_index_mgmt, sizeof(s_index_mgmt),\n");
    fprintf(out, "                               (void *)(uintptr_t)s_index_data, sizeof(s_index_data),\n");
    fprintf(out, "                               &s_index) != CPLAT_OK)\n");
    fprintf(out, "    {\n");
    fprintf(out, "        /* 同一ビルドで作ったイメージが読めないという不変条件の破れ。\n");
    fprintf(out, "           線形走査へ縮退させず、その場で落とす。 */\n");
    fprintf(out, "        (void)fprintf(stderr, \"%s: 埋め込み索引へ接続できません\\n\");\n", stem);
    fprintf(out, "        abort();\n");
    fprintf(out, "    }\n");
    fprintf(out, "    for (size_t i = 0; i < %s_META_COUNT; i++)\n", prefix);
    fprintf(out, "    {\n");
    fprintf(out, "        /* 登録は検索を速くするだけで、失敗しても未登録の記述子として\n");
    fprintf(out, "           正しく動作する。確保失敗で異常終了はさせず、原因だけ残す。 */\n");
    fprintf(out, "        if (struct_meta_index_register(s_descriptors[i]) != CPLAT_OK)\n");
    fprintf(out, "        {\n");
    fprintf(out, "            (void)fprintf(stderr, \"%s: 記述子を索引へ登録できません: %%s\\n\",\n", stem);
    fprintf(out, "                          s_descriptors[i]->name);\n");
    fprintf(out, "        }\n");
    fprintf(out, "    }\n");
    fprintf(out, "    (void)cplat_shutdown_register(detach_index, NULL);\n");
    fprintf(out, "}\n\n");

    fprintf(out, "size_t %s_meta_count(void)\n", stem);
    fprintf(out, "{\n");
    fprintf(out, "    return %s_META_COUNT;\n", prefix);
    fprintf(out, "}\n\n");

    fprintf(out, "const struct_meta_descriptor *%s_meta_get(%s_meta_id id)\n", stem, stem);
    fprintf(out, "{\n");
    fprintf(out, "    if (((int)id < 0) || (id >= %s_META_COUNT))\n", prefix);
    fprintf(out, "    {\n");
    fprintf(out, "        return NULL;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return s_descriptors[id];\n");
    fprintf(out, "}\n\n");

    fprintf(out, "const struct_meta_descriptor *%s_meta_find(const char *name)\n", stem);
    fprintf(out, "{\n");
    fprintf(out, "    if (name == NULL)\n");
    fprintf(out, "    {\n");
    fprintf(out, "        return NULL;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    cplat_call_once(&s_index_once, attach_index);\n\n");
    fprintf(out, "    const void *value = NULL;\n");
    fprintf(out, "    if (cplat_hashtable_find_value_ref(s_index, name, &value) != CPLAT_OK)\n");
    fprintf(out, "    {\n");
    fprintf(out, "        return NULL;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    /* value_align に sizeof(size_t) を指定しているため、型付きで参照できる。 */\n");
    fprintf(out, "    return s_descriptors[*(const size_t *)value];\n");
    fprintf(out, "}\n");
    return 0;
}

int struct_meta_gen_emit(const struct_meta_gen_struct_list *structs, const char *header_path, const char *out_path)
{
    char stem[STRUCT_META_GEN_EMIT_PATH_BYTES];
    char prefix[STRUCT_META_GEN_EMIT_PATH_BYTES];
    char header_out[STRUCT_META_GEN_EMIT_PATH_BYTES];

    if ((structs == NULL) || (structs->head == NULL) || (header_path == NULL) || (out_path == NULL))
    {
        fprintf(stderr, "struct-meta-gen: 生成に必要な引数がありません\n");
        return 1;
    }
    if (header_stem(header_path, stem, sizeof(stem)) != 0)
    {
        fprintf(stderr, "struct-meta-gen: ヘッダー名から識別子を作れません: %s\n", header_path);
        return 1;
    }
    if (stem_prefix(stem, prefix, sizeof(prefix)) != 0)
    {
        fprintf(stderr, "struct-meta-gen: ヘッダー名から接頭辞を作れません: %s\n", header_path);
        return 1;
    }
    if (header_out_path(out_path, header_out, sizeof(header_out)) != 0)
    {
        fprintf(stderr, "struct-meta-gen: 出力パスは .c で終わる必要があります: %s\n", out_path);
        return 1;
    }

    FILE *out = cplat_fopen(out_path, "w", NULL);
    if (out == NULL)
    {
        fprintf(stderr, "struct-meta-gen: 出力ファイルを作成できません: %s\n", out_path);
        return 1;
    }

    fprintf(out, "/* このファイルは struct-meta-gen が自動生成しました。手編集しないでください。 */\n");
    fprintf(out, "#include \"../%s\"\n", header_path);
    fprintf(out, "#include \"%s\"\n\n", generated_header_include(header_out));
    fprintf(out, "#include <struct_meta/meta/index.h>\n\n");
    fprintf(out, "#include <cplat/base/result.h>\n");
    fprintf(out, "#include <cplat/hashtable/hashtable.h>\n");
    fprintf(out, "#include <cplat/runtime/shutdown.h>\n");
    fprintf(out, "#include <cplat/sync/sync.h>\n\n");
    fprintf(out, "#include <stddef.h>\n");
    fprintf(out, "#include <limits.h>\n");
    fprintf(out, "#include <stdint.h>\n");
    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "#include <stdlib.h>\n");
    fprintf(out, "#include <time.h>\n\n");
    fprintf(out, "_Static_assert(CHAR_BIT == 8, \"struct-meta は8ビットの char を前提とします\");\n");
    fprintf(out, "_Static_assert(CHAR_MIN == INT8_MIN, \"struct-meta は符号付きの char を前提とします\");\n");
    fprintf(out,
            "_Static_assert(CHAR_MAX == INT8_MAX, \"struct-meta は char と int8_t の同じ範囲を前提とします\");\n\n");

    emitted_name *emitted = NULL;
    for (const struct_meta_gen_struct *s = structs->head; s != NULL; s = s->next)
    {
        emit_struct(out, s, &emitted);
    }
    int catalog_ret = emit_catalog_source(out, stem, prefix, structs);
    fclose(out);
    if (catalog_ret != 0)
    {
        return 1;
    }

    return emit_catalog_header(header_out, stem, prefix, structs);
}
