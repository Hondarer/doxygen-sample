/**
 *******************************************************************************
 *  @file           structgen_ast.c
 *  @brief          structgen の AST 構築ヘルパーを実装します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include "structgen_ast.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

sg_field *sg_field_create(char *name, char *type_name, int is_struct_type, long array_count, int line, char *brief,
                          char *json_name, int json_ignore, int json_required)
{
    sg_field *field = (sg_field *)calloc(1, sizeof(*field));
    if (field == NULL)
    {
        return NULL;
    }
    field->name = name;
    field->type_name = type_name;
    field->brief = brief;
    field->json_name = json_name;
    field->json_ignore = json_ignore;
    field->json_required = json_required;
    field->is_struct_type = is_struct_type;
    field->array_count = array_count;
    field->line = line;
    field->next = NULL;
    return field;
}

sg_field_list *sg_field_list_create(sg_field *first)
{
    sg_field_list *list = (sg_field_list *)calloc(1, sizeof(*list));
    if (list == NULL)
    {
        return NULL;
    }
    list->head = first;
    list->tail = first;
    return list;
}

sg_field_list *sg_field_list_append(sg_field_list *list, sg_field *field)
{
    list->tail->next = field;
    list->tail = field;
    return list;
}

sg_struct *sg_struct_create(char *name, sg_field_list *fields, char *brief)
{
    sg_struct *s = (sg_struct *)calloc(1, sizeof(*s));
    if (s == NULL)
    {
        return NULL;
    }
    s->name = name;
    s->brief = brief;
    s->fields = fields->head;
    s->next = NULL;
    free(fields);
    return s;
}

void sg_struct_list_append(sg_struct_list **list, sg_struct *s)
{
    if (*list == NULL)
    {
        *list = (sg_struct_list *)calloc(1, sizeof(**list));
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

const sg_struct *sg_struct_list_find(const sg_struct_list *list, const char *name)
{
    if (list == NULL)
    {
        return NULL;
    }
    for (const sg_struct *s = list->head; s != NULL; s = s->next)
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

int sg_doc_has_file_tag(const char *raw)
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

static int cmd_ends_here(char after)
{
    return (after == '\0') || (isspace((unsigned char)after) != 0) || (after == '{') || (after == '*');
}

static const char *find_cmd_last(const char *text, const char *cmd)
{
    const char *found = NULL;
    const char *p = text;
    size_t cmd_len = strlen(cmd);

    while (*p != '\0')
    {
        if ((*p == '@') && (strncmp(p + 1, cmd, cmd_len) == 0) && (cmd_ends_here(p[1U + cmd_len]) != 0))
        {
            found = p;
        }
        p++;
    }
    return found;
}

static char *extract_json_name_value(const char *text)
{
    const char *tag = find_cmd_last(text, "json_name");
    const char *p;
    const char *end;
    size_t len;
    char *out;

    if (tag == NULL)
    {
        return NULL;
    }
    p = tag + (sizeof("@json_name") - 1U);
    while ((*p == ' ') || (*p == '\t'))
    {
        p++;
    }
    if (*p != '{')
    {
        return NULL;
    }
    p++;
    end = strchr(p, '}');
    if (end == NULL)
    {
        return NULL;
    }
    while ((p < end) && (isspace((unsigned char)*p) != 0))
    {
        p++;
    }
    while ((end > p) && (isspace((unsigned char)end[-1]) != 0))
    {
        end--;
    }
    if (p >= end)
    {
        return NULL;
    }
    len = (size_t)(end - p);
    out = (char *)malloc(len + 1U);
    if (out == NULL)
    {
        return NULL;
    }
    memcpy(out, p, len);
    out[len] = '\0';
    return out;
}

static char *copy_without_json_cmds(const char *text)
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
        if (*p == '@')
        {
            if ((strncmp(p + 1, "json_name", 9) == 0) && (cmd_ends_here(p[10]) != 0))
            {
                p += 10;
                while ((*p == ' ') || (*p == '\t'))
                {
                    p++;
                }
                if (*p == '{')
                {
                    const char *end = strchr(p, '}');
                    p = (end != NULL) ? (end + 1) : (p + 1);
                }
                continue;
            }
            if ((strncmp(p + 1, "json_ignore", 11) == 0) && (cmd_ends_here(p[12]) != 0))
            {
                p += 12;
                continue;
            }
            if ((strncmp(p + 1, "json_required", 13) == 0) && (cmd_ends_here(p[14]) != 0))
            {
                p += 14;
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

char *sg_brief_from_doc(const char *raw, int is_postfix)
{
    char *stripped;
    char *without_json;
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

    without_json = copy_without_json_cmds(stripped);
    free(stripped);
    if (without_json == NULL)
    {
        return NULL;
    }

    brief = extract_brief_tag(without_json); /* 複数あるときは最後の @brief を使う */
    if ((brief == NULL) && (is_postfix != 0))
    {
        brief = collapse_ws(without_json, without_json + strlen(without_json));
    }
    free(without_json);
    return brief;
}

sg_doc_attrs sg_doc_attrs_from_raw(const char *raw, int is_postfix)
{
    sg_doc_attrs attrs = {0};
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

    attrs.brief = sg_brief_from_doc(raw, is_postfix);
    attrs.json_name = extract_json_name_value(stripped);
    attrs.has_json_name = (find_cmd_last(stripped, "json_name") != NULL) ? 1 : 0;
    attrs.json_ignore = (find_cmd_last(stripped, "json_ignore") != NULL) ? 1 : 0;
    attrs.has_json_ignore = attrs.json_ignore;
    attrs.json_required = (find_cmd_last(stripped, "json_required") != NULL) ? 1 : 0;
    attrs.has_json_required = attrs.json_required;
    free(stripped);
    return attrs;
}

sg_doc_attrs sg_doc_attrs_choose(sg_doc_attrs prefix, sg_doc_attrs postfix)
{
    sg_doc_attrs out = prefix;

    if (postfix.brief != NULL)
    {
        free(prefix.brief);
        out.brief = postfix.brief;
    }
    if (postfix.has_json_name != 0)
    {
        free(prefix.json_name);
        out.json_name = postfix.json_name;
        out.has_json_name = 1;
    }
    else
    {
        free(postfix.json_name);
    }
    if (postfix.has_json_ignore != 0)
    {
        out.json_ignore = postfix.json_ignore;
        out.has_json_ignore = 1;
    }
    if (postfix.has_json_required != 0)
    {
        out.json_required = postfix.json_required;
        out.has_json_required = 1;
    }
    return out;
}

char *sg_brief_choose(char *prefix_brief, char *postfix_brief)
{
    if (postfix_brief != NULL)
    {
        free(prefix_brief);
        return postfix_brief;
    }
    return prefix_brief;
}

char *sg_doc_concat(char *first, char *second)
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
