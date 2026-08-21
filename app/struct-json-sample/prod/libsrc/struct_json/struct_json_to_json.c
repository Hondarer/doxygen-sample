/**
 *******************************************************************************
 *  @file           struct_json_to_json.c
 *  @brief          構造体インスタンスを、記述子 (@ref sj_struct_desc) に従って cJSON オブジェクトへ変換します。
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

static int struct_to_json(const sj_struct_desc *desc, const unsigned char *base, cJSON **json_out);

/**
 *  @brief          スカラー値 1 個分のメモリー内容から cJSON アイテムを作成します。
 */
static int scalar_to_json(sj_field_kind kind, const unsigned char *field_ptr, size_t elem_size, cJSON **item_out)
{
    cJSON *item = NULL;

    switch (kind)
    {
    case SJ_FIELD_INT:
        if (elem_size != sizeof(int))
        {
            return COM_UTIL_ERR_UNSUPPORTED;
        }
        {
            int value;
            memcpy(&value, field_ptr, sizeof(value));
            item = cJSON_CreateNumber((double)value);
        }
        break;

    case SJ_FIELD_UNSIGNED:
        if (elem_size != sizeof(unsigned int))
        {
            return COM_UTIL_ERR_UNSUPPORTED;
        }
        {
            unsigned int value;
            memcpy(&value, field_ptr, sizeof(value));
            item = cJSON_CreateNumber((double)value);
        }
        break;

    case SJ_FIELD_FLOAT:
        if (elem_size != sizeof(float))
        {
            return COM_UTIL_ERR_UNSUPPORTED;
        }
        {
            float value;
            memcpy(&value, field_ptr, sizeof(value));
            item = cJSON_CreateNumber((double)value);
        }
        break;

    case SJ_FIELD_DOUBLE:
        if (elem_size != sizeof(double))
        {
            return COM_UTIL_ERR_UNSUPPORTED;
        }
        {
            double value;
            memcpy(&value, field_ptr, sizeof(value));
            item = cJSON_CreateNumber(value);
        }
        break;

    case SJ_FIELD_CHAR_ARRAY:
        /* field_ptr は char[N] の先頭を指す。NUL 終端文字列として扱う。 */
        item = cJSON_CreateString((const char *)field_ptr);
        break;

    case SJ_FIELD_STRUCT:
    default:
        /* SJ_FIELD_STRUCT は element_to_json() が先に振り分けるため、ここには到達しない。 */
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    if (item == NULL)
    {
        return COM_UTIL_ERR_OUT_OF_MEMORY;
    }
    *item_out = item;
    return COM_UTIL_OK;
}

/**
 *  @brief          フィールド 1 要素分 (配列要素、またはネスト構造体 1 個) を cJSON アイテムへ変換します。
 */
static int element_to_json(const sj_field_desc *field, const unsigned char *elem_ptr, cJSON **item_out)
{
    if (field->kind == SJ_FIELD_STRUCT)
    {
        return struct_to_json(field->nested, elem_ptr, item_out);
    }
    return scalar_to_json(field->kind, elem_ptr, field->elem_size, item_out);
}

/**
 *  @brief          フィールド記述子 1 個分 (スカラー、char 配列、固定長配列のいずれか) を cJSON アイテムへ変換します。
 */
static int field_to_json(const sj_field_desc *field, const unsigned char *base, cJSON **item_out)
{
    const unsigned char *field_ptr = base + field->offset;

    /* char[N] は「配列」だが単一の JSON 文字列として扱う (array_count は常に 1)。 */
    if ((field->kind == SJ_FIELD_CHAR_ARRAY) || (field->array_count <= 1U))
    {
        return element_to_json(field, field_ptr, item_out);
    }

    cJSON *array = cJSON_CreateArray();
    if (array == NULL)
    {
        return COM_UTIL_ERR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < field->array_count; i++)
    {
        cJSON *elem = NULL;
        int ret = element_to_json(field, field_ptr + (i * field->elem_size), &elem);
        if (ret != COM_UTIL_OK)
        {
            cJSON_Delete(array);
            return ret;
        }
        if (!cJSON_AddItemToArray(array, elem))
        {
            cJSON_Delete(elem);
            cJSON_Delete(array);
            return COM_UTIL_ERR_OUT_OF_MEMORY;
        }
    }

    *item_out = array;
    return COM_UTIL_OK;
}

/**
 *  @brief          構造体インスタンス 1 個分を cJSON オブジェクトへ変換します。
 */
static int struct_to_json(const sj_struct_desc *desc, const unsigned char *base, cJSON **json_out)
{
    cJSON *obj = cJSON_CreateObject();
    if (obj == NULL)
    {
        return COM_UTIL_ERR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < desc->field_count; i++)
    {
        const sj_field_desc *field = &desc->fields[i];
        cJSON *item = NULL;

        int ret = field_to_json(field, base, &item);
        if (ret != COM_UTIL_OK)
        {
            cJSON_Delete(obj);
            return ret;
        }
        if (!cJSON_AddItemToObject(obj, field->name, item))
        {
            cJSON_Delete(item);
            cJSON_Delete(obj);
            return COM_UTIL_ERR_OUT_OF_MEMORY;
        }
    }

    *json_out = obj;
    return COM_UTIL_OK;
}

int sj_to_json(const sj_struct_desc *desc, const void *instance, cJSON **json_out)
{
    if ((desc == NULL) || (instance == NULL) || (json_out == NULL))
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    *json_out = NULL;
    return struct_to_json(desc, (const unsigned char *)instance, json_out);
}
