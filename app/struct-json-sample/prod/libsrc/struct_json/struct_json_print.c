/**
 *******************************************************************************
 *  @file           struct_json_print.c
 *  @brief          記述子を辿り、構造体インスタンスの値をテキストへ書き出します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <struct_json/struct_json.h>

#include <com_util/base/result.h>

#include <stdio.h>
#include <string.h>

static void print_indent(FILE *out, int indent)
{
    int i;
    for (i = 0; i < indent; i++)
    {
        fputc(' ', out);
    }
}

static void format_scalar_value(sj_field_kind kind, const unsigned char *field_ptr, char *dest, size_t dest_size)
{
    switch (kind)
    {
    case SJ_FIELD_INT:
    {
        int value;
        memcpy(&value, field_ptr, sizeof(value));
        snprintf(dest, dest_size, "%d", value);
        break;
    }
    case SJ_FIELD_UNSIGNED:
    {
        unsigned int value;
        memcpy(&value, field_ptr, sizeof(value));
        snprintf(dest, dest_size, "%u", value);
        break;
    }
    case SJ_FIELD_FLOAT:
    {
        float value;
        memcpy(&value, field_ptr, sizeof(value));
        snprintf(dest, dest_size, "%g", (double)value);
        break;
    }
    case SJ_FIELD_DOUBLE:
    {
        double value;
        memcpy(&value, field_ptr, sizeof(value));
        snprintf(dest, dest_size, "%g", value);
        break;
    }
    case SJ_FIELD_CHAR_ARRAY:
        snprintf(dest, dest_size, "\"%s\"", (const char *)field_ptr);
        break;
    case SJ_FIELD_STRUCT:
    default:
        snprintf(dest, dest_size, "{...}");
        break;
    }
}

static int print_struct(const sj_struct_desc *desc, const unsigned char *base, FILE *out, int indent);

static int print_element(const sj_field_desc *field, const unsigned char *elem_ptr, FILE *out, int indent,
                         const char *label)
{
    if (field->kind == SJ_FIELD_STRUCT)
    {
        if (field->nested == NULL)
        {
            return COM_UTIL_ERR_INVALID_ARGUMENT;
        }
        print_indent(out, indent);
        fprintf(out, "%s:\n", label);
        return print_struct(field->nested, elem_ptr, out, indent + 2);
    }

    {
        char current[64];
        format_scalar_value(field->kind, elem_ptr, current, sizeof(current));
        print_indent(out, indent);
        fprintf(out, "%s = %s\n", label, current);
    }
    return COM_UTIL_OK;
}

static int print_field(const sj_field_desc *field, const unsigned char *base, FILE *out, int indent)
{
    const unsigned char *field_ptr = base + field->offset;

    if ((field->kind == SJ_FIELD_CHAR_ARRAY) || (field->array_count <= 1U))
    {
        return print_element(field, field_ptr, out, indent, field->name);
    }

    {
        size_t i;
        char label[128];
        for (i = 0; i < field->array_count; i++)
        {
            int ret;
            snprintf(label, sizeof(label), "%s[%zu]", field->name, i);
            ret = print_element(field, field_ptr + (i * field->elem_size), out, indent, label);
            if (ret != COM_UTIL_OK)
            {
                return ret;
            }
        }
    }
    return COM_UTIL_OK;
}

static int print_struct(const sj_struct_desc *desc, const unsigned char *base, FILE *out, int indent)
{
    size_t i;
    for (i = 0; i < desc->field_count; i++)
    {
        int ret = print_field(&desc->fields[i], base, out, indent);
        if (ret != COM_UTIL_OK)
        {
            return ret;
        }
    }
    return COM_UTIL_OK;
}

int sj_print(const sj_struct_desc *desc, const void *instance, FILE *out)
{
    if ((desc == NULL) || (instance == NULL) || (out == NULL))
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    print_indent(out, 0);
    fprintf(out, "%s:\n", (desc->name != NULL) ? desc->name : "(unnamed)");
    return print_struct(desc, (const unsigned char *)instance, out, 2);
}
