/**
 *******************************************************************************
 *  @file           print.h
 *  @brief          メタデータを使って構造体の内容をテキスト出力します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#ifndef STRUCT_META_PRINT_H
#define STRUCT_META_PRINT_H

#include <stdio.h>

#include <struct_meta/meta/meta.h>

/**
 *  @addtogroup STRUCT_META_PUBLIC_API
 *  @{
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    STRUCT_META_EXPORT int STRUCT_META_API struct_meta_print_write(const struct_meta_descriptor *descriptor,
                                                                   const void *instance, FILE *stream);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* STRUCT_META_PRINT_H */
