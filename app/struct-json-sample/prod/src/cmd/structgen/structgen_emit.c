/**
 *******************************************************************************
 *  @file           structgen_emit.c
 *  @brief          解析済みの構造体定義から、メタデータ記述子の C ソースを生成します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  フィールドのオフセットとサイズは、生成コードに埋め込んだ `offsetof`/`sizeof` を
 *  実ヘッダーに対してコンパイラへ計算させます。structgen 自身はレイアウトを
 *  計算しません (`docs/architecture.md` の設計方針を参照)。
 *
 *  ネスト構造体メンバーがある場合、参照先の構造体記述子を先に出力してから
 *  (依存順)、それを参照する構造体の記述子を出力します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include "structgen_emit.h"

#include <com_util/crt/stdio.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern sg_struct_list *g_structs;

/** 生成パス・識別子を組み立てる作業バッファーのバイト数です。 */
#define SG_EMIT_PATH_BYTES 512

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
 *  @brief          フィールドの型スペリングから @ref sj_field_kind の列挙定数名を求めます。
 */
static const char *field_kind_name(const char *type_name, int is_char_array)
{
    if (is_char_array)
    {
        return "SJ_FIELD_CHAR_ARRAY";
    }
    if (strcmp(type_name, "int") == 0)
    {
        return "SJ_FIELD_INT";
    }
    if (strcmp(type_name, "unsigned") == 0)
    {
        return "SJ_FIELD_UNSIGNED";
    }
    if (strcmp(type_name, "float") == 0)
    {
        return "SJ_FIELD_FLOAT";
    }
    return "SJ_FIELD_DOUBLE";
}

/**
 *  @brief          フィールドの型スペリングから、要素 1 個分の `sizeof` 式を求めます。
 */
static const char *elem_sizeof_expr(const char *type_name)
{
    if (strcmp(type_name, "int") == 0)
    {
        return "sizeof(int)";
    }
    if (strcmp(type_name, "unsigned") == 0)
    {
        return "sizeof(unsigned int)";
    }
    if (strcmp(type_name, "float") == 0)
    {
        return "sizeof(float)";
    }
    if (strcmp(type_name, "double") == 0)
    {
        return "sizeof(double)";
    }
    return "sizeof(char)";
}

static int count_fields(const sg_struct *s)
{
    int count = 0;
    for (const sg_field *f = s->fields; f != NULL; f = f->next)
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
    char raw[SG_EMIT_PATH_BYTES];
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

static int count_structs(const sg_struct_list *list)
{
    int count = 0;
    for (const sg_struct *s = list->head; s != NULL; s = s->next)
    {
        count++;
    }
    return count;
}

/**
 *  @brief          構造体 1 個分の記述子を再帰的に出力します。
 *
 *  ネスト構造体メンバーがあれば、参照先の記述子を先に出力してから
 *  (依存順)、`s` 自身の記述子を出力します。同じ構造体は 2 回出力しません。
 *  記述子はすべて `static` です。外部からは型一覧の取得関数だけが見えます。
 */
static void emit_struct(FILE *out, const sg_struct *s, emitted_name **emitted)
{
    if (is_emitted(*emitted, s->name))
    {
        return;
    }

    for (const sg_field *f = s->fields; f != NULL; f = f->next)
    {
        if (f->is_struct_type)
        {
            const sg_struct *nested = sg_struct_list_find(g_structs, f->type_name);
            if (nested == NULL)
            {
                fprintf(stderr, "structgen: %d: 未知の型です: %s\n", f->line, f->type_name);
                exit(1);
            }
            emit_struct(out, nested, emitted);
        }
    }

    fprintf(out, "static const sj_field_desc g_%s_fields[] = {\n", s->name);
    for (const sg_field *f = s->fields; f != NULL; f = f->next)
    {
        int is_char_array = ((!f->is_struct_type) && (strcmp(f->type_name, "char") == 0) && (f->array_count > 0));
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
            kind = "SJ_FIELD_STRUCT";
            snprintf(elem_size_expr, sizeof(elem_size_expr), "sizeof(%s)", f->type_name);
            snprintf(nested_expr, sizeof(nested_expr), "&g_%s_desc", f->type_name);
        }
        else
        {
            kind = field_kind_name(f->type_name, is_char_array);
            snprintf(elem_size_expr, sizeof(elem_size_expr), "%s", elem_sizeof_expr(f->type_name));
        }

        fprintf(out, "    { \"%s\", %s, 0, offsetof(%s, %s), %s, %ld, %s, %s, ", f->name, kind, s->name, f->name,
                elem_size_expr, array_count_out, char_buf_expr, nested_expr);
        fprint_c_string(out, f->brief);
        fprintf(out, ", ");
        fprint_c_string(out, f->json_name);
        fprintf(out, ", %d, %d },\n", f->json_ignore, f->json_required);
    }
    fprintf(out, "};\n\n");

    fprintf(out, "static const sj_struct_desc g_%s_desc = { \"%s\", sizeof(%s), g_%s_fields, %d, ", s->name, s->name,
            s->name, s->name, count_fields(s));
    fprint_c_string(out, s->brief);
    fprintf(out, " };\n\n");

    emitted_name *node = (emitted_name *)calloc(1, sizeof(*node));
    node->name = s->name;
    node->next = *emitted;
    *emitted = node;
}

/**
 *  @brief          型一覧 enum と取得関数の宣言をヘッダーへ書き出します。
 */
static int emit_catalog_header(const char *header_out, const char *stem, const char *prefix,
                               const sg_struct_list *structs)
{
    FILE *out = com_util_fopen(header_out, "w", NULL);
    if (out == NULL)
    {
        fprintf(stderr, "structgen: 出力ファイルを作成できません: %s\n", header_out);
        return 1;
    }

    int index = 0;
    int count = count_structs(structs);

    fprintf(out, "/* このファイルは structgen が自動生成しました。手編集しないでください。 */\n");
    fprintf(out, "#ifndef %s_META_H\n", prefix);
    fprintf(out, "#define %s_META_H\n\n", prefix);
    fprintf(out, "#include <struct_json/struct_json_meta.h>\n\n");
    fprintf(out, "#ifdef __cplusplus\n");
    fprintf(out, "extern \"C\"\n");
    fprintf(out, "{\n");
    fprintf(out, "#endif /* __cplusplus */\n\n");
    fprintf(out, "typedef enum %s_id\n", stem);
    fprintf(out, "{\n");
    for (const sg_struct *s = structs->head; s != NULL; s = s->next)
    {
        fprintf(out, "    %s_", prefix);
        for (const char *p = s->name; *p != '\0'; p++)
        {
            fputc(toupper((unsigned char)*p), out);
        }
        fprintf(out, " = %d,\n", index);
        index++;
    }
    fprintf(out, "    %s_COUNT = %d,\n", prefix, count);
    fprintf(out, "} %s_id;\n\n", stem);
    fprintf(out, "const sj_struct_desc *%s_desc(%s_id id);\n\n", stem, stem);
    fprintf(out, "#ifdef __cplusplus\n");
    fprintf(out, "}\n");
    fprintf(out, "#endif /* __cplusplus */\n\n");
    fprintf(out, "#endif /* %s_META_H */\n", prefix);

    fclose(out);
    return 0;
}

/**
 *  @brief          型一覧テーブルと取得関数を C ソースへ書き出します。
 *
 *  テーブルの並びはヘッダーの宣言順 (enum と同じ) です。記述子の出力順
 *  (依存順) とは独立です。
 */
static void emit_catalog_source(FILE *out, const char *stem, const char *prefix, const sg_struct_list *structs)
{
    fprintf(out, "static const sj_struct_desc *const s_descs[%s_COUNT] = {\n", prefix);
    for (const sg_struct *s = structs->head; s != NULL; s = s->next)
    {
        fprintf(out, "    &g_%s_desc,\n", s->name);
    }
    fprintf(out, "};\n\n");

    fprintf(out, "const sj_struct_desc *%s_desc(%s_id id)\n", stem, stem);
    fprintf(out, "{\n");
    fprintf(out, "    if (((int)id < 0) || (id >= %s_COUNT))\n", prefix);
    fprintf(out, "    {\n");
    fprintf(out, "        return NULL;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return s_descs[id];\n");
    fprintf(out, "}\n");
}

int sg_emit(const sg_struct_list *structs, const char *header_path, const char *out_path)
{
    char stem[SG_EMIT_PATH_BYTES];
    char prefix[SG_EMIT_PATH_BYTES];
    char header_out[SG_EMIT_PATH_BYTES];

    if ((structs == NULL) || (structs->head == NULL) || (header_path == NULL) || (out_path == NULL))
    {
        fprintf(stderr, "structgen: 生成に必要な引数がありません\n");
        return 1;
    }
    if (header_stem(header_path, stem, sizeof(stem)) != 0)
    {
        fprintf(stderr, "structgen: ヘッダー名から識別子を作れません: %s\n", header_path);
        return 1;
    }
    if (stem_prefix(stem, prefix, sizeof(prefix)) != 0)
    {
        fprintf(stderr, "structgen: ヘッダー名から接頭辞を作れません: %s\n", header_path);
        return 1;
    }
    if (header_out_path(out_path, header_out, sizeof(header_out)) != 0)
    {
        fprintf(stderr, "structgen: 出力パスは .c で終わる必要があります: %s\n", out_path);
        return 1;
    }

    FILE *out = com_util_fopen(out_path, "w", NULL);
    if (out == NULL)
    {
        fprintf(stderr, "structgen: 出力ファイルを作成できません: %s\n", out_path);
        return 1;
    }

    fprintf(out, "/* このファイルは structgen が自動生成しました。手編集しないでください。 */\n");
    fprintf(out, "#include \"../%s\"\n", header_path);
    fprintf(out, "#include \"%s\"\n", generated_header_include(header_out));
    fprintf(out, "#include <stddef.h>\n\n");

    emitted_name *emitted = NULL;
    for (const sg_struct *s = structs->head; s != NULL; s = s->next)
    {
        emit_struct(out, s, &emitted);
    }
    emit_catalog_source(out, stem, prefix, structs);
    fclose(out);

    return emit_catalog_header(header_out, stem, prefix, structs);
}
