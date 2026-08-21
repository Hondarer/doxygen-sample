/**
 *******************************************************************************
 *  @file           struct_json_patch.c
 *  @brief          記述子の階層をメニュー形式で辿り、対話形式で構造体インスタンスの値を編集します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  `com_util_prompt` (`com_util/prompt/prompt.h`) を使用します。TTY でない場合
 *  (パイプ・リダイレクト等) は `com_util_prompt` 自身が `fgets` にフォールバック
 *  するため、本ファイルは対話端末かどうかを意識しません。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <struct_json/struct_json.h>

#include <com_util/base/result.h>
#include <com_util/prompt/prompt.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** 1 回の入力行を受けるバッファーのバイト数です。 */
#define SJ_PATCH_LINE_BYTES 256

static int patch_struct(com_util_prompt *prompt, const sj_struct_desc *desc, unsigned char *base);

/**
 *  @brief          フィールド 1 個分の現在値を、一覧表示用に短く整形します。
 */
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

/**
 *  @brief          入力行を int へ変換します。空行は「変更なし」を表す @c COM_UTIL_ERR_EOF を返します。
 */
static int parse_int(const char *text, int *value_out)
{
    if (text[0] == '\0')
    {
        return COM_UTIL_ERR_EOF;
    }
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if ((end == text) || (*end != '\0') || (errno != 0) || (value < INT_MIN) || (value > INT_MAX))
    {
        return COM_UTIL_ERR_INVALID_INTEGER;
    }
    *value_out = (int)value;
    return COM_UTIL_OK;
}

static int parse_unsigned(const char *text, unsigned int *value_out)
{
    if (text[0] == '\0')
    {
        return COM_UTIL_ERR_EOF;
    }
    if (text[0] == '-')
    {
        return COM_UTIL_ERR_INVALID_INTEGER;
    }
    char *end = NULL;
    errno = 0;
    unsigned long value = strtoul(text, &end, 10);
    if ((end == text) || (*end != '\0') || (errno != 0) || (value > UINT_MAX))
    {
        return COM_UTIL_ERR_INVALID_INTEGER;
    }
    *value_out = (unsigned int)value;
    return COM_UTIL_OK;
}

static int parse_double(const char *text, double *value_out)
{
    if (text[0] == '\0')
    {
        return COM_UTIL_ERR_EOF;
    }
    char *end = NULL;
    errno = 0;
    double value = strtod(text, &end);
    if ((end == text) || (*end != '\0') || (errno != 0))
    {
        return COM_UTIL_ERR_INVALID_INTEGER;
    }
    *value_out = value;
    return COM_UTIL_OK;
}

/**
 *  @brief          スカラー フィールド 1 個分の値を対話入力で書き換えます。
 *  @return         @c COM_UTIL_OK (書き換えた)、@c COM_UTIL_ERR_EOF (空行、変更なしで戻る)、
 *                  それ以外は @p prompt からのエラー。
 */
static int patch_scalar(com_util_prompt *prompt, const sj_field_desc *field, unsigned char *field_ptr)
{
    char line[SJ_PATCH_LINE_BYTES];
    char current[64];

    format_scalar_value(field->kind, field_ptr, current, sizeof(current));

    for (;;)
    {
        int ret = com_util_prompt_readline_fmt(prompt, line, sizeof(line), "%s (現在値 %s、空行で変更なし)> ",
                                               field->name, current);
        if (ret != COM_UTIL_OK)
        {
            return ret;
        }

        if (line[0] == '\0')
        {
            return COM_UTIL_ERR_EOF;
        }

        switch (field->kind)
        {
        case SJ_FIELD_INT:
        {
            int value;
            int parse_ret = parse_int(line, &value);
            if (parse_ret == COM_UTIL_OK)
            {
                memcpy(field_ptr, &value, sizeof(value));
                return COM_UTIL_OK;
            }
            printf("整数として解釈できません: %s\n", line);
            break;
        }
        case SJ_FIELD_UNSIGNED:
        {
            unsigned int value;
            int parse_ret = parse_unsigned(line, &value);
            if (parse_ret == COM_UTIL_OK)
            {
                memcpy(field_ptr, &value, sizeof(value));
                return COM_UTIL_OK;
            }
            printf("符号なし整数として解釈できません: %s\n", line);
            break;
        }
        case SJ_FIELD_FLOAT:
        {
            double parsed;
            int parse_ret = parse_double(line, &parsed);
            if (parse_ret == COM_UTIL_OK)
            {
                float value = (float)parsed;
                memcpy(field_ptr, &value, sizeof(value));
                return COM_UTIL_OK;
            }
            printf("数値として解釈できません: %s\n", line);
            break;
        }
        case SJ_FIELD_DOUBLE:
        {
            double value;
            int parse_ret = parse_double(line, &value);
            if (parse_ret == COM_UTIL_OK)
            {
                memcpy(field_ptr, &value, sizeof(value));
                return COM_UTIL_OK;
            }
            printf("数値として解釈できません: %s\n", line);
            break;
        }
        case SJ_FIELD_CHAR_ARRAY:
        {
            size_t text_len = strlen(line);
            if ((field->char_buf_size == 0U) || (text_len >= field->char_buf_size))
            {
                printf("文字列がバッファー サイズ (%zu バイト) を超えています。\n", field->char_buf_size);
                break;
            }
            memcpy(field_ptr, line, text_len + 1U);
            return COM_UTIL_OK;
        }
        case SJ_FIELD_STRUCT:
        default:
            /* ここには到達しない (呼び出し元がスカラーだけを渡す)。 */
            return COM_UTIL_ERR_INVALID_ARGUMENT;
        }
    }
}

/**
 *  @brief          フィールド 1 要素分 (配列要素、またはネスト構造体) を編集します。
 */
static int patch_element(com_util_prompt *prompt, const sj_field_desc *field, unsigned char *elem_ptr)
{
    if (field->kind == SJ_FIELD_STRUCT)
    {
        return patch_struct(prompt, field->nested, elem_ptr);
    }
    int ret = patch_scalar(prompt, field, elem_ptr);
    if (ret == COM_UTIL_ERR_EOF)
    {
        /* 空行 (変更なし) は呼び出し元にとってはエラーではない。 */
        return COM_UTIL_OK;
    }
    return ret;
}

/**
 *  @brief          配列フィールドの要素インデックスをメニュー形式で選択させ、選択した要素を編集します。
 */
static int patch_array_field(com_util_prompt *prompt, const sj_field_desc *field, unsigned char *field_ptr)
{
    char line[SJ_PATCH_LINE_BYTES];

    for (;;)
    {
        printf("-- %s (配列、要素数 %zu) --\n", field->name, field->array_count);
        for (size_t i = 0; i < field->array_count; i++)
        {
            printf("  [%zu]\n", i);
        }

        int ret = com_util_prompt_readline_fmt(prompt, line, sizeof(line), "要素番号を選択 (空行で戻る)> ");
        if (ret != COM_UTIL_OK)
        {
            return ret;
        }
        if (line[0] == '\0')
        {
            return COM_UTIL_OK;
        }

        int index;
        if ((parse_int(line, &index) != COM_UTIL_OK) || (index < 0) || ((size_t)index >= field->array_count))
        {
            printf("0 から %zu の範囲で入力してください。\n", field->array_count - 1U);
            continue;
        }

        unsigned char *elem_ptr = field_ptr + ((size_t)index * field->elem_size);
        ret = patch_element(prompt, field, elem_ptr);
        if (ret != COM_UTIL_OK)
        {
            return ret;
        }
    }
}

/**
 *  @brief          構造体インスタンス 1 個分のフィールド一覧をメニュー形式で辿ります。
 */
static int patch_struct(com_util_prompt *prompt, const sj_struct_desc *desc, unsigned char *base)
{
    char line[SJ_PATCH_LINE_BYTES];

    for (;;)
    {
        if ((desc->brief != NULL) && (desc->brief[0] != '\0'))
        {
            printf("-- %s --  %s\n", desc->name, desc->brief);
        }
        else
        {
            printf("-- %s --\n", desc->name);
        }
        for (size_t i = 0; i < desc->field_count; i++)
        {
            const sj_field_desc *field = &desc->fields[i];
            unsigned char *field_ptr = base + field->offset;
            const char *brief = "";
            const char *brief_sep = "";

            if ((field->brief != NULL) && (field->brief[0] != '\0'))
            {
                brief_sep = "  ";
                brief = field->brief;
            }

            if (field->kind == SJ_FIELD_STRUCT)
            {
                if (field->array_count > 1U)
                {
                    printf("  %zu) %s [配列 %zu 件]%s%s\n", i + 1U, field->name, field->array_count, brief_sep, brief);
                }
                else
                {
                    printf("  %zu) %s {...}%s%s\n", i + 1U, field->name, brief_sep, brief);
                }
            }
            else if ((field->kind != SJ_FIELD_CHAR_ARRAY) && (field->array_count > 1U))
            {
                printf("  %zu) %s [配列 %zu 件]%s%s\n", i + 1U, field->name, field->array_count, brief_sep, brief);
            }
            else
            {
                char current[64];
                format_scalar_value(field->kind, field_ptr, current, sizeof(current));
                printf("  %zu) %s = %s%s%s\n", i + 1U, field->name, current, brief_sep, brief);
            }
        }

        int ret = com_util_prompt_readline_fmt(prompt, line, sizeof(line), "フィールド番号を選択 (空行で戻る)> ");
        if (ret != COM_UTIL_OK)
        {
            return ret;
        }
        if (line[0] == '\0')
        {
            return COM_UTIL_OK;
        }

        int index;
        if ((parse_int(line, &index) != COM_UTIL_OK) || (index < 1) || ((size_t)index > desc->field_count))
        {
            printf("1 から %zu の範囲で入力してください。\n", desc->field_count);
            continue;
        }

        const sj_field_desc *field = &desc->fields[(size_t)index - 1U];
        unsigned char *field_ptr = base + field->offset;

        if ((field->kind != SJ_FIELD_CHAR_ARRAY) && (field->array_count > 1U))
        {
            ret = patch_array_field(prompt, field, field_ptr);
        }
        else
        {
            ret = patch_element(prompt, field, field_ptr);
        }
        if (ret != COM_UTIL_OK)
        {
            return ret;
        }
    }
}

int sj_patch_interactive(const sj_struct_desc *desc, void *instance)
{
    if ((desc == NULL) || (instance == NULL))
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    com_util_prompt *prompt = com_util_prompt_create(NULL);
    if (prompt == NULL)
    {
        return COM_UTIL_ERR_OUT_OF_MEMORY;
    }

    int ret = patch_struct(prompt, desc, (unsigned char *)instance);

    com_util_prompt_dispose(prompt);

    if (ret == COM_UTIL_ERR_EOF)
    {
        /* 対話セッションの正常終了 (EOF/空行での終了) は成功として扱う。 */
        return COM_UTIL_OK;
    }
    return ret;
}
