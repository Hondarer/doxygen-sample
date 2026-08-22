/**
 *******************************************************************************
 *  @file           file.h
 *  @brief          構造体と JSON テキスト ファイルを相互変換します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#ifndef STRUCT_META_JSON_FILE_H
#define STRUCT_META_JSON_FILE_H

#include <struct_meta/meta/meta.h>

/**
 *  @addtogroup STRUCT_META_PUBLIC_API
 *  @{
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    STRUCT_META_EXPORT int STRUCT_META_API struct_meta_json_file_save(const struct_meta_descriptor *descriptor,
                                                                      const void *instance, const char *path);
    STRUCT_META_EXPORT int STRUCT_META_API struct_meta_json_file_load(const struct_meta_descriptor *descriptor,
                                                                      const char *path, void *instance_out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* STRUCT_META_JSON_FILE_H */
