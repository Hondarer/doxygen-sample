/**
 *******************************************************************************
 *  @file           validate.c
 *  @brief          構造体メタデータの整合性を検査します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <struct_meta/meta/meta.h>

#include <com_util/base/result.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct validation_context
{
    const struct_meta_descriptor **stack;
    size_t depth;
    size_t capacity;
} validation_context;

static int push_descriptor(validation_context *context, const struct_meta_descriptor *descriptor)
{
    for (size_t i = 0; i < context->depth; i++)
    {
        if (context->stack[i] == descriptor)
        {
            return COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
        }
    }

    if (context->depth == context->capacity)
    {
        size_t capacity = (context->capacity == 0U) ? 8U : context->capacity * 2U;
        if (capacity < context->capacity || capacity > (SIZE_MAX / sizeof(*context->stack)))
        {
            return COM_UTIL_ERR_OUT_OF_MEMORY;
        }
        const struct_meta_descriptor **stack =
            (const struct_meta_descriptor **)realloc(context->stack, capacity * sizeof(*context->stack));
        if (stack == NULL)
        {
            return COM_UTIL_ERR_OUT_OF_MEMORY;
        }
        context->stack = stack;
        context->capacity = capacity;
    }

    context->stack[context->depth] = descriptor;
    context->depth++;
    return COM_UTIL_OK;
}

static int validate_attributes(const struct_meta_field *field)
{
    if ((field->attribute_count > 0U) && (field->attributes == NULL))
    {
        return COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
    }

    for (size_t i = 0; i < field->attribute_count; i++)
    {
        const struct_meta_attribute *attribute = &field->attributes[i];
        if ((attribute->key == NULL) || (attribute->key[0] == '\0'))
        {
            return COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
        }
    }

    for (size_t i = 0; i < field->attribute_count; i++)
    {
        const struct_meta_attribute *attribute = &field->attributes[i];
        for (size_t j = i + 1U; j < field->attribute_count; j++)
        {
            if (strcmp(attribute->key, field->attributes[j].key) == 0)
            {
                return COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
            }
        }
    }
    return COM_UTIL_OK;
}

static int validate_descriptor(validation_context *context, const struct_meta_descriptor *descriptor)
{
    if ((descriptor->name == NULL) || (descriptor->name[0] == '\0') || (descriptor->size == 0U) ||
        ((descriptor->field_count > 0U) && (descriptor->fields == NULL)))
    {
        return COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
    }

    int ret = push_descriptor(context, descriptor);
    if (ret != COM_UTIL_OK)
    {
        return ret;
    }

    for (size_t i = 0; i < descriptor->field_count; i++)
    {
        const struct_meta_field *field = &descriptor->fields[i];
        size_t field_size;

        if ((field->name == NULL) || (field->name[0] == '\0') || (field->kind < STRUCT_META_FIELD_INT) ||
            (field->kind > STRUCT_META_FIELD_STRUCT) || (field->element_size == 0U) || (field->element_count == 0U))
        {
            ret = COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
            break;
        }

        if (field->kind == STRUCT_META_FIELD_CHAR_ARRAY)
        {
            if ((field->char_buffer_size == 0U) || (field->element_count != 1U) || (field->nested != NULL))
            {
                ret = COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
                break;
            }
            field_size = field->char_buffer_size;
        }
        else
        {
            if ((field->char_buffer_size != 0U) || (field->element_count > (SIZE_MAX / field->element_size)))
            {
                ret = COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
                break;
            }
            field_size = field->element_count * field->element_size;
        }

        if ((field->offset > descriptor->size) || (field_size > (descriptor->size - field->offset)))
        {
            ret = COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
            break;
        }

        if (field->kind == STRUCT_META_FIELD_STRUCT)
        {
            if ((field->nested == NULL) || (field->element_size != field->nested->size))
            {
                ret = COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
                break;
            }
            ret = validate_descriptor(context, field->nested);
            if (ret != COM_UTIL_OK)
            {
                break;
            }
        }
        else if (field->nested != NULL)
        {
            ret = COM_UTIL_ERR_CORRUPT_DESCRIPTOR;
            break;
        }

        ret = validate_attributes(field);
        if (ret != COM_UTIL_OK)
        {
            break;
        }
    }

    context->depth--;
    return ret;
}

/* Doxygen コメントは、ヘッダーに記載 */

int struct_meta_descriptor_validate(const struct_meta_descriptor *descriptor)
{
    if (descriptor == NULL)
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    validation_context context = {0};
    int ret = validate_descriptor(&context, descriptor);

    free(context.stack);
    return ret;
}
