/**
 *******************************************************************************
 *  @file           file.c
 *  @brief          構造体インスタンスと JSON テキスト ファイルを相互変換します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  ファイル I/O は `com_util/crt/stdio.h` の stdio ラッパーを使用します。\n
 *  JSON 設定ファイルの読み書きは単発の逐次アクセスが支配的な用途であるため、
 *  `com_util_file_*` (低レベル API) や mmap ではなく stdio ラッパーを選択します
 *  (`app/com_util/docs/fileio-api-selection-guideline.md` の結論 4)。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <struct_meta/json/file.h>
#include <struct_meta/json/json.h>

#include <com_util/base/result.h>
#include <com_util/crt/stdio.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Doxygen コメントは、ヘッダーに記載 */

int struct_meta_json_file_save(const struct_meta_descriptor *desc, const void *instance, const char *path)
{
    if ((desc == NULL) || (instance == NULL) || (path == NULL))
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    cJSON *json = NULL;
    int ret = struct_meta_json_encode(desc, instance, &json);
    if (ret != COM_UTIL_OK)
    {
        return ret;
    }

    char *text = cJSON_Print(json);
    cJSON_Delete(json);
    if (text == NULL)
    {
        return COM_UTIL_ERR_OUT_OF_MEMORY;
    }

    FILE *stream = com_util_fopen(path, "wb", NULL);
    if (stream == NULL)
    {
        cJSON_free(text);
        return COM_UTIL_ERR_NOT_FOUND;
    }

    size_t text_len = strlen(text);
    int needs_newline = (text_len == 0U) || (text[text_len - 1U] != '\n');
    if ((needs_newline != 0) && (text_len == SIZE_MAX))
    {
        (void)com_util_fclose(stream, NULL);
        cJSON_free(text);
        return COM_UTIL_ERR_OUT_OF_RANGE;
    }

    size_t expected_len = text_len + ((needs_newline != 0) ? 1U : 0U);
    size_t written = com_util_fwrite(text, 1, text_len, stream, NULL);
    if ((written == text_len) && (needs_newline != 0))
    {
        static const char newline[] = "\n";
        if (com_util_fwrite(newline, 1U, 1U, stream, NULL) == 1U)
        {
            written++;
        }
    }
    com_util_fclose(stream, NULL);
    cJSON_free(text);

    if (written != expected_len)
    {
        return COM_UTIL_ERR_UNKNOWN;
    }
    return COM_UTIL_OK;
}

/* Doxygen コメントは、ヘッダーに記載 */

int struct_meta_json_file_load(const struct_meta_descriptor *desc, const char *path, void *instance)
{
    if ((desc == NULL) || (instance == NULL) || (path == NULL))
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    FILE *stream = com_util_fopen(path, "rb", NULL);
    if (stream == NULL)
    {
        return COM_UTIL_ERR_NOT_FOUND;
    }

    if (com_util_fseek(stream, 0, SEEK_END) != 0)
    {
        com_util_fclose(stream, NULL);
        return COM_UTIL_ERR_UNKNOWN;
    }
    int64_t file_size = com_util_ftell(stream);
    if ((file_size < 0) || (com_util_fseek(stream, 0, SEEK_SET) != 0))
    {
        com_util_fclose(stream, NULL);
        return COM_UTIL_ERR_UNKNOWN;
    }

    /* JSON テキストのバイト列バッファー。要素型に対応しない生バイト確保のため sizeof(*p) は使わない。 */
    char *text = (char *)malloc((size_t)file_size + 1U);
    if (text == NULL)
    {
        com_util_fclose(stream, NULL);
        return COM_UTIL_ERR_OUT_OF_MEMORY;
    }

    size_t read_count = com_util_fread(text, 1, (size_t)file_size, stream, NULL);
    com_util_fclose(stream, NULL);
    if (read_count != (size_t)file_size)
    {
        free(text);
        return COM_UTIL_ERR_UNKNOWN;
    }
    text[file_size] = '\0';

    cJSON *json = cJSON_Parse(text);
    free(text);
    if (json == NULL)
    {
        return COM_UTIL_ERR_INVALID_ARGUMENT;
    }

    int ret = struct_meta_json_decode(desc, json, instance);
    cJSON_Delete(json);
    return ret;
}
