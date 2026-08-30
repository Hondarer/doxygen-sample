/**
 *******************************************************************************
 *  @file           struct_meta_gen_ast.c
 *  @brief          struct-meta-gen の AST 構築ヘルパーを実装します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include "struct_meta_gen_ast.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 *  対応するスカラー型の表です。
 *
 *  幅が LP64 と LLP64 で一致する型だけを載せます。`long` と `unsigned long` は
 *  幅が異なるため載せず、文法規則で明示的に拒否します。
 *  see: app/struct-meta/docs/architecture.md
 */
static const struct_meta_gen_scalar_type g_scalar_types[] = {
    {"char", "STRUCT_META_FIELD_SIGNED_INTEGER", "sizeof(char)"},
    {"signed char", "STRUCT_META_FIELD_SIGNED_INTEGER", "sizeof(signed char)"},
    {"unsigned char", "STRUCT_META_FIELD_UNSIGNED_INTEGER", "sizeof(unsigned char)"},
    {"int", "STRUCT_META_FIELD_SIGNED_INTEGER", "sizeof(int)"},
    {"unsigned", "STRUCT_META_FIELD_UNSIGNED_INTEGER", "sizeof(unsigned int)"},
    {"long long", "STRUCT_META_FIELD_SIGNED_INTEGER", "sizeof(long long)"},
    {"unsigned long long", "STRUCT_META_FIELD_UNSIGNED_INTEGER", "sizeof(unsigned long long)"},
    {"int8_t", "STRUCT_META_FIELD_SIGNED_INTEGER", "sizeof(int8_t)"},
    {"int16_t", "STRUCT_META_FIELD_SIGNED_INTEGER", "sizeof(int16_t)"},
    {"int32_t", "STRUCT_META_FIELD_SIGNED_INTEGER", "sizeof(int32_t)"},
    {"int64_t", "STRUCT_META_FIELD_SIGNED_INTEGER", "sizeof(int64_t)"},
    {"uint8_t", "STRUCT_META_FIELD_UNSIGNED_INTEGER", "sizeof(uint8_t)"},
    {"uint16_t", "STRUCT_META_FIELD_UNSIGNED_INTEGER", "sizeof(uint16_t)"},
    {"uint32_t", "STRUCT_META_FIELD_UNSIGNED_INTEGER", "sizeof(uint32_t)"},
    {"uint64_t", "STRUCT_META_FIELD_UNSIGNED_INTEGER", "sizeof(uint64_t)"},
    {"float", "STRUCT_META_FIELD_FLOAT", "sizeof(float)"},
    {"double", "STRUCT_META_FIELD_DOUBLE", "sizeof(double)"},
};

/* Doxygen コメントは、ヘッダーに記載 */

const struct_meta_gen_scalar_type *struct_meta_gen_find_scalar_type(const char *name)
{
    if (name == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < (sizeof(g_scalar_types) / sizeof(g_scalar_types[0])); i++)
    {
        if (strcmp(g_scalar_types[i].name, name) == 0)
        {
            return &g_scalar_types[i];
        }
    }
    return NULL;
}

struct_meta_gen_field *struct_meta_gen_field_create(char *name, char *type_name, int is_struct_type, long array_count,
                                                    int line, char *brief, struct_meta_gen_attribute *attributes)
{
    struct_meta_gen_field *field = (struct_meta_gen_field *)calloc(1, sizeof(*field));
    if (field == NULL)
    {
        return NULL;
    }
    field->name = name;
    field->type_name = type_name;
    field->brief = brief;
    field->attributes = attributes;
    field->is_struct_type = is_struct_type;
    field->array_count = array_count;
    field->line = line;
    field->next = NULL;
    return field;
}

struct_meta_gen_field_list *struct_meta_gen_field_list_create(struct_meta_gen_field *first)
{
    struct_meta_gen_field_list *list = (struct_meta_gen_field_list *)calloc(1, sizeof(*list));
    if (list == NULL)
    {
        return NULL;
    }
    list->head = first;
    list->tail = first;
    return list;
}

struct_meta_gen_field_list *struct_meta_gen_field_list_append(struct_meta_gen_field_list *list,
                                                              struct_meta_gen_field *field)
{
    list->tail->next = field;
    list->tail = field;
    return list;
}

struct_meta_gen_struct *struct_meta_gen_struct_create(char *name, struct_meta_gen_field_list *fields, char *brief,
                                                      struct_meta_gen_attribute *attributes)
{
    struct_meta_gen_struct *s = (struct_meta_gen_struct *)calloc(1, sizeof(*s));
    if (s == NULL)
    {
        return NULL;
    }
    s->name = name;
    s->brief = brief;
    s->attributes = attributes;
    s->fields = fields->head;
    s->next = NULL;
    free(fields);
    return s;
}

void struct_meta_gen_struct_list_append(struct_meta_gen_struct_list **list, struct_meta_gen_struct *s)
{
    if (*list == NULL)
    {
        *list = (struct_meta_gen_struct_list *)calloc(1, sizeof(**list));
    }
    if ((*list)->head == NULL)
    {
        (*list)->head = s;
        (*list)->tail = s;
    }
    else
    {
        (*list)->tail->next = s;
        (*list)->tail = s;
    }
}

const struct_meta_gen_struct *struct_meta_gen_struct_list_find(const struct_meta_gen_struct_list *list,
                                                               const char *name)
{
    if (list == NULL)
    {
        return NULL;
    }
    for (const struct_meta_gen_struct *s = list->head; s != NULL; s = s->next)
    {
        if (strcmp(s->name, name) == 0)
        {
            return s;
        }
    }
    return NULL;
}

static const char *skip_doc_open(const char *s)
{
    if ((s == NULL) || (s[0] != '/'))
    {
        return s;
    }
    if (s[1] == '*')
    {
        if ((s[2] == '*') && (s[3] == '<'))
        {
            return s + 4;
        }
        if ((s[2] == '!') && (s[3] == '<'))
        {
            return s + 4;
        }
        if (s[2] == '*')
        {
            return s + 3;
        }
        if (s[2] == '!')
        {
            return s + 3;
        }
    }
    if (s[1] == '/')
    {
        if ((s[2] == '/') && (s[3] == '<'))
        {
            return s + 4;
        }
        if ((s[2] == '!') && (s[3] == '<'))
        {
            return s + 4;
        }
        if (s[2] == '/')
        {
            return s + 3;
        }
        if (s[2] == '!')
        {
            return s + 3;
        }
    }
    return s;
}

static char *strip_doc_text(const char *raw)
{
    const char *body;
    const char *end;
    size_t raw_len;
    char *out;
    size_t out_len;
    int at_line_start;

    if (raw == NULL)
    {
        return NULL;
    }

    body = skip_doc_open(raw);
    raw_len = strlen(body);
    end = body + raw_len;
    if ((raw_len >= 2U) && (end[-2] == '*') && (end[-1] == '/'))
    {
        end -= 2;
    }

    out = (char *)malloc((size_t)(end - body) + 1U);
    if (out == NULL)
    {
        return NULL;
    }

    out_len = 0;
    at_line_start = 1;
    while (body < end)
    {
        if (at_line_start != 0)
        {
            while ((body < end) && ((*body == ' ') || (*body == '\t')))
            {
                body++;
            }
            if ((body < end) && (*body == '*'))
            {
                body++;
                if ((body < end) && (*body == ' '))
                {
                    body++;
                }
            }
            at_line_start = 0;
            if (body >= end)
            {
                break;
            }
        }
        out[out_len] = *body;
        if (*body == '\n')
        {
            at_line_start = 1;
        }
        out_len++;
        body++;
    }
    out[out_len] = '\0';
    return out;
}

static const char *find_doc_tag(const char *text, const char *tag)
{
    size_t tag_len = strlen(tag);
    const char *p = text;
    int at_line_start = 1;

    while (*p != '\0')
    {
        if (at_line_start != 0)
        {
            while ((*p == ' ') || (*p == '\t'))
            {
                p++;
            }
            if ((*p == '@') && (strncmp(p + 1, tag, tag_len) == 0))
            {
                const char after = p[1U + tag_len];
                if ((after == '\0') || (isspace((unsigned char)after) != 0))
                {
                    return p;
                }
            }
        }
        at_line_start = (*p == '\n') ? 1 : 0;
        if (*p == '\0')
        {
            break;
        }
        p++;
    }
    return NULL;
}

static char *collapse_ws(const char *begin, const char *end)
{
    char *out;
    size_t out_len;
    int in_space;

    while ((begin < end) && (isspace((unsigned char)*begin) != 0))
    {
        begin++;
    }
    while ((end > begin) && (isspace((unsigned char)end[-1]) != 0))
    {
        end--;
    }
    if (begin >= end)
    {
        return NULL;
    }

    out = (char *)malloc((size_t)(end - begin) + 1U);
    if (out == NULL)
    {
        return NULL;
    }

    out_len = 0;
    in_space = 0;
    while (begin < end)
    {
        if (isspace((unsigned char)*begin) != 0)
        {
            if (in_space == 0)
            {
                out[out_len] = ' ';
                out_len++;
                in_space = 1;
            }
        }
        else
        {
            out[out_len] = *begin;
            out_len++;
            in_space = 0;
        }
        begin++;
    }
    out[out_len] = '\0';
    if (out_len == 0U)
    {
        free(out);
        return NULL;
    }
    return out;
}

static const char *find_doc_tag_last(const char *text, const char *tag)
{
    const char *found = NULL;
    const char *p = text;

    for (;;)
    {
        const char *hit = find_doc_tag(p, tag);
        if (hit == NULL)
        {
            return found;
        }
        found = hit;
        p = hit + 1;
    }
}

static char *extract_brief_tag(const char *stripped)
{
    const char *tag = find_doc_tag_last(stripped, "brief");
    const char *p;
    const char *begin;
    const char *end;

    if (tag == NULL)
    {
        return NULL;
    }

    p = tag + (sizeof("@brief") - 1U);
    while ((*p != '\0') && (isspace((unsigned char)*p) != 0))
    {
        p++;
    }
    begin = p;
    end = p;
    while (*end != '\0')
    {
        if (*end == '\n')
        {
            const char *next = end + 1;
            while ((*next == ' ') || (*next == '\t'))
            {
                next++;
            }
            if (*next == '@')
            {
                break;
            }
            if (*next == '\0')
            {
                break;
            }
        }
        end++;
    }
    return collapse_ws(begin, end);
}

int struct_meta_gen_doc_has_file_tag(const char *raw)
{
    char *stripped;
    int found;

    if (raw == NULL)
    {
        return 0;
    }
    stripped = strip_doc_text(raw);
    if (stripped == NULL)
    {
        return 0;
    }
    found = (find_doc_tag(stripped, "file") != NULL) ? 1 : 0;
    free(stripped);
    return found;
}

static int is_meta_cmd(const char *p)
{
    static const char command[] = "@struct_meta";
    const size_t command_len = sizeof(command) - 1U;

    return (strncmp(p, command, command_len) == 0) &&
           ((p[command_len] == '{') || (isspace((unsigned char)p[command_len]) != 0) || (p[command_len] == '\0'));
}

static char *copy_range(const char *begin, const char *end)
{
    const size_t len = (size_t)(end - begin);
    char *out = (char *)malloc(len + 1U);

    if (out == NULL)
    {
        return NULL;
    }
    memcpy(out, begin, len);
    out[len] = '\0';
    return out;
}

static int valid_attribute_key(const char *key)
{
    for (const char *p = key; *p != '\0'; p++)
    {
        if ((isalnum((unsigned char)*p) == 0) && (*p != '.') && (*p != '_') && (*p != '-'))
        {
            return 0;
        }
    }
    return key[0] != '\0';
}

static struct_meta_gen_attribute *find_attribute(struct_meta_gen_attribute *attributes, const char *key)
{
    for (struct_meta_gen_attribute *attribute = attributes; attribute != NULL; attribute = attribute->next)
    {
        if (strcmp(attribute->key, key) == 0)
        {
            return attribute;
        }
    }
    return NULL;
}

static void append_attribute(struct_meta_gen_attribute **attributes, struct_meta_gen_attribute *attribute)
{
    if (*attributes == NULL)
    {
        *attributes = attribute;
        return;
    }

    struct_meta_gen_attribute *tail = *attributes;
    while (tail->next != NULL)
    {
        tail = tail->next;
    }
    tail->next = attribute;
}

static int parse_attributes(const char *text, const int line, struct_meta_gen_attribute **attributes_out)
{
    static const size_t command_len = sizeof("@struct_meta") - 1U;
    const char *p = text;

    while (*p != '\0')
    {
        if ((*p != '@') || (is_meta_cmd(p) == 0))
        {
            p++;
            continue;
        }

        const char *open = p + command_len;
        while ((*open == ' ') || (*open == '\t'))
        {
            open++;
        }
        if (*open != '{')
        {
            fprintf(stderr, "struct-meta-gen: %d: @struct_meta に { がありません\n", line);
            return 1;
        }

        const char *close = strchr(open + 1, '}');
        if (close == NULL)
        {
            fprintf(stderr, "struct-meta-gen: %d: @struct_meta に } がありません\n", line);
            return 1;
        }
        for (const char *q = open + 1; q < close; q++)
        {
            if ((*q == '{') || (*q == '\n') || (*q == '\r'))
            {
                fprintf(stderr, "struct-meta-gen: %d: @struct_meta の属性は 1 行で記載してください\n", line);
                return 1;
            }
        }

        const char *begin = open + 1;
        const char *end = close;
        while ((begin < end) && (isspace((unsigned char)*begin) != 0))
        {
            begin++;
        }
        while ((end > begin) && (isspace((unsigned char)end[-1]) != 0))
        {
            end--;
        }

        const char *separator = memchr(begin, '=', (size_t)(end - begin));
        const char *key_end = (separator == NULL) ? end : separator;
        while ((key_end > begin) && (isspace((unsigned char)key_end[-1]) != 0))
        {
            key_end--;
        }
        char *key = copy_range(begin, key_end);
        if ((key == NULL) || (valid_attribute_key(key) == 0))
        {
            fprintf(stderr, "struct-meta-gen: %d: @struct_meta の属性名が不正です\n", line);
            free(key);
            return 1;
        }

        char *value = NULL;
        if (separator != NULL)
        {
            const char *value_begin = separator + 1;
            while ((value_begin < end) && (isspace((unsigned char)*value_begin) != 0))
            {
                value_begin++;
            }
            if (value_begin == end)
            {
                fprintf(stderr, "struct-meta-gen: %d: @struct_meta{%s=} の属性値が空です\n", line, key);
                free(key);
                return 1;
            }
            value = copy_range(value_begin, end);
            if (value == NULL)
            {
                free(key);
                return 1;
            }
        }

        if (find_attribute(*attributes_out, key) != NULL)
        {
            fprintf(stderr, "struct-meta-gen: %d: 属性が重複しています: %s\n", line, key);
            free(value);
            free(key);
            return 1;
        }

        struct_meta_gen_attribute *attribute = (struct_meta_gen_attribute *)calloc(1, sizeof(*attribute));
        if (attribute == NULL)
        {
            free(value);
            free(key);
            return 1;
        }
        attribute->key = key;
        attribute->value = value;
        append_attribute(attributes_out, attribute);
        p = close + 1;
    }
    return 0;
}

static char *copy_without_meta_cmds(const char *text)
{
    size_t len = strlen(text);
    char *out = (char *)malloc(len + 1U);
    size_t o = 0;
    const char *p = text;

    if (out == NULL)
    {
        return NULL;
    }
    while (*p != '\0')
    {
        if ((*p == '@') && (is_meta_cmd(p) != 0))
        {
            const char *open = p + (sizeof("@struct_meta") - 1U);
            while ((*open == ' ') || (*open == '\t'))
            {
                open++;
            }
            if (*open == '{')
            {
                const char *close = strchr(open + 1, '}');
                p = (close == NULL) ? (open + 1) : (close + 1);
                continue;
            }
        }
        out[o] = *p;
        o++;
        p++;
    }
    out[o] = '\0';
    return out;
}

char *struct_meta_gen_brief_from_doc(const char *raw, int is_postfix)
{
    char *stripped;
    char *without_meta;
    char *brief;

    if (raw == NULL)
    {
        return NULL;
    }

    stripped = strip_doc_text(raw);
    if (stripped == NULL)
    {
        return NULL;
    }

    without_meta = copy_without_meta_cmds(stripped);
    free(stripped);
    if (without_meta == NULL)
    {
        return NULL;
    }

    brief = extract_brief_tag(without_meta); /* 複数あるときは最後の @brief を使う */
    if ((brief == NULL) && (is_postfix != 0))
    {
        brief = collapse_ws(without_meta, without_meta + strlen(without_meta));
    }
    free(without_meta);
    return brief;
}

struct_meta_gen_doc_attrs struct_meta_gen_doc_attrs_from_raw(const char *raw, const int is_postfix, const int line)
{
    struct_meta_gen_doc_attrs attrs = {0};
    char *stripped;

    if (raw == NULL)
    {
        return attrs;
    }
    stripped = strip_doc_text(raw);
    if (stripped == NULL)
    {
        return attrs;
    }

    attrs.invalid = parse_attributes(stripped, line, &attrs.attributes);
    if (attrs.invalid == 0)
    {
        attrs.brief = struct_meta_gen_brief_from_doc(raw, is_postfix);
    }
    free(stripped);
    return attrs;
}

struct_meta_gen_doc_attrs struct_meta_gen_doc_attrs_choose(struct_meta_gen_doc_attrs prefix,
                                                           struct_meta_gen_doc_attrs postfix, const int line)
{
    struct_meta_gen_doc_attrs out = prefix;

    if (postfix.brief != NULL)
    {
        free(prefix.brief);
        out.brief = postfix.brief;
    }
    out.invalid = (prefix.invalid != 0) || (postfix.invalid != 0);
    for (struct_meta_gen_attribute *attribute = postfix.attributes; attribute != NULL; attribute = attribute->next)
    {
        if (find_attribute(prefix.attributes, attribute->key) != NULL)
        {
            fprintf(stderr, "struct-meta-gen: %d: 属性が重複しています: %s\n", line, attribute->key);
            out.invalid = 1;
        }
    }
    if (prefix.attributes == NULL)
    {
        out.attributes = postfix.attributes;
    }
    else
    {
        struct_meta_gen_attribute *tail = prefix.attributes;
        while (tail->next != NULL)
        {
            tail = tail->next;
        }
        tail->next = postfix.attributes;
    }
    return out;
}

char *struct_meta_gen_brief_choose(char *prefix_brief, char *postfix_brief)
{
    if (postfix_brief != NULL)
    {
        free(prefix_brief);
        return postfix_brief;
    }
    return prefix_brief;
}

char *struct_meta_gen_doc_concat(char *first, char *second)
{
    size_t first_len;
    size_t second_len;
    char *out;

    if (first == NULL)
    {
        return second;
    }
    if (second == NULL)
    {
        return first;
    }

    first_len = strlen(first);
    second_len = strlen(second);
    out = (char *)malloc(first_len + 1U + second_len + 1U);
    if (out == NULL)
    {
        free(first);
        free(second);
        return NULL;
    }
    memcpy(out, first, first_len);
    out[first_len] = '\n';
    memcpy(out + first_len + 1U, second, second_len + 1U);
    free(first);
    free(second);
    return out;
}
