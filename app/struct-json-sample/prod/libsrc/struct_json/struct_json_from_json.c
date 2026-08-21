/**
 *******************************************************************************
 *  @file           struct_json_from_json.c
 *  @brief          cJSON オブジェクトの内容を、記述子 (@ref sj_struct_desc) に従って構造体インスタンスへ書き戻します。
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

#include <string.h>

static int struct_from_json(const sj_struct_desc *desc, const cJSON *json, unsigned char *base);

/**
 *  @brief          cJSON アイテム 1 個分をスカラー値としてメモリーへ書き戻します。
 */
static int scalar_from_json(sj_field_kind kind, const cJSON *item, unsigned char *field_ptr, size_t elem_size,
                            size_t char_buf_size)
{
    switch (kind)
    {
    case SJ_FIELD_INT:
        if ((elem_size != sizeof(int)) || (!cJSON_IsNumber(item)))
        {
            return COM_UTIL_ERR_INVALID_ARGUMENT;
        }
        {
            int value = (int)cJSON_GetNumberValue(item);
            memcpy(field_ptr, &value, sizeof(value));
        }
        break;

    case SJ_FIELD_UNSIGNED:
        if ((elem_size != sizeof(unsigned int)) || (!cJSON_IsNumber(item)))
        {
            return COM_UTIL_ERR_INVALID_ARGUMENT;
        }
        {
            double raw = cJSON_GetNumberValue(item);
            if (raw < 0.0)
            {
                return COM_UTIL_ERR_INVALID_ARGUMENT;
            }
            unsigned int value = (unsigned int)raw;
            memcpy(field_ptr, &value, sizeof(value));
        }
        break;

    case SJ_FIELD_FLOAT:
        if ((elem_size != sizeof(float)) || (!cJSON_IsNumber(item)))
        {
            return COM_UTIL_ERR_INVALID_ARGUMENT;
        }
        {
            float value = (float)cJSON_GetNumberValue(item);
            memcpy(field_ptr, &value, sizeof(value));
        }
        break;

    case SJ_FIELD_DOUBLE:
        if ((elem_size != sizeof(double)) || (!cJSON_IsNumber(item)))
        {
            return COM_UTIL_ERR_INVALID_ARGUMENT;
        }
        {
            double value = cJSON_GetNumberValue(item);
            memcpy(field_ptr, &value, sizeof(value));
        }
        break;

    case SJ_FIELD_CHAR_ARRAY:
        if (!cJSON_IsString(item))
        {
            return COM_UTIL_ERR_INVALID_ARGUMENT;
        }
        {
            const char *text = cJSON_GetStringValue(item);
            size_t text_len = strlen(text);

            /* char_buf_size は NUL 終端を含むバッファー全体のサイズ。 */
            if ((char_buf_size == 0U) || (text_len >= char_buf_size))
            {
                return COM_UTIL_ERR_BUFFER_TOO_SMALL;
            }
            memcpy(field_ptr, text, text_len + 1U);
        }
        break;

    case SJ_FIELD_STRUCT:
    default:
        /* SJ_FIELD_STRUCT は element_from_json() が先に振り分けるため、ここには到達しない。 */
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    return COM_UTIL_OK;
}

/**
 *  @brief          cJSON アイテム 1 個分 (配列要素、またはネスト構造体 1 個) をメモリーへ書き戻します。
 */
static int element_from_json(const sj_field_desc *field, const cJSON *item, unsigned char *elem_ptr)
{
    if (field->kind == SJ_FIELD_STRUCT)
    {
        return struct_from_json(field->nested, item, elem_ptr);
    }
    return scalar_from_json(field->kind, item, elem_ptr, field->elem_size, field->char_buf_size);
}

/**
 *  @brief          フィールド記述子 1 個分 (スカラー、char 配列、固定長配列のいずれか) を cJSON から書き戻します。
 */
static const char *json_key(const sj_field_desc *field)
{
    if ((field->json_name != NULL) && (field->json_name[0] != '\0'))
    {
        return field->json_name;
    }
    return field->name;
}

static int field_from_json(const sj_field_desc *field, const cJSON *json, unsigned char *base)
{
    const cJSON *item;

    if (field->json_ignore != 0)
    {
        return COM_UTIL_OK;
    }

    item = cJSON_GetObjectItemCaseSensitive(json, json_key(field));
    if (item == NULL)
    {
        if (field->json_required != 0)
        {
            return COM_UTIL_ERR_MISSING_REQUIRED;
        }
        return COM_UTIL_OK;
    }

    unsigned char *field_ptr = base + field->offset;

    if ((field->kind == SJ_FIELD_CHAR_ARRAY) || (field->array_count <= 1U))
    {
        return element_from_json(field, item, field_ptr);
    }

    if (!cJSON_IsArray(item))
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    int array_size = cJSON_GetArraySize(item);
    if ((array_size < 0) || ((size_t)array_size != field->array_count))
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < field->array_count; i++)
    {
        const cJSON *elem = cJSON_GetArrayItem(item, (int)i);
        int ret = element_from_json(field, elem, field_ptr + (i * field->elem_size));
        if (ret != COM_UTIL_OK)
        {
            return ret;
        }
    }

    return COM_UTIL_OK;
}

/**
 *  @brief          cJSON オブジェクト 1 個分を構造体インスタンスへ書き戻します。
 */
static int struct_from_json(const sj_struct_desc *desc, const cJSON *json, unsigned char *base)
{
    if (!cJSON_IsObject(json))
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < desc->field_count; i++)
    {
        int ret = field_from_json(&desc->fields[i], json, base);
        if (ret != COM_UTIL_OK)
        {
            return ret;
        }
    }

    return COM_UTIL_OK;
}

int sj_from_json(const sj_struct_desc *desc, const cJSON *json, void *instance)
{
    if ((desc == NULL) || (json == NULL) || (instance == NULL))
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    return struct_from_json(desc, json, (unsigned char *)instance);
}
