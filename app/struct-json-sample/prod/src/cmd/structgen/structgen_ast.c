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

#include <stdlib.h>
#include <string.h>

sg_field *sg_field_create(char *name, char *type_name, int is_struct_type, long array_count, int line)
{
    sg_field *field = (sg_field *)calloc(1, sizeof(*field));
    if (field == NULL)
    {
        return NULL;
    }
    field->name = name;
    field->type_name = type_name;
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

sg_struct *sg_struct_create(char *name, sg_field_list *fields)
{
    sg_struct *s = (sg_struct *)calloc(1, sizeof(*s));
    if (s == NULL)
    {
        return NULL;
    }
    s->name = name;
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
