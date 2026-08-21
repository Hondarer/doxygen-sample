/**
 *******************************************************************************
 *  @file           struct_json.h
 *  @brief          メタデータ記述子を使い、構造体と JSON を相互変換する API を提供します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  @ref sj_struct_desc (`struct_json_meta.h`、通常は `structgen` が生成) を歩いて、
 *  構造体インスタンスと cJSON オブジェクトを相互変換します。\n
 *  戻り値は `com_util/base/result.h` の共通結果コード (@c COM_UTIL_OK およびその他の
 *  @c COM_UTIL_ERR_* ) です。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#ifndef STRUCT_JSON_H
#define STRUCT_JSON_H

#include <cJSON.h>

#include <struct_json/struct_json_export.h>
#include <struct_json/struct_json_meta.h>

/**
 *  @ingroup        STRUCT_JSON_PUBLIC_API
 *  @{
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     *  @brief          構造体インスタンスから cJSON オブジェクトを構築します。
     *
     *  @param[in]      desc        構造体の記述子です。
     *  @param[in]      instance    変換元の構造体インスタンスです。
     *  @param[out]     json_out    構築した cJSON オブジェクトの格納先です。\n
     *                              呼び出し元が @c cJSON_Delete() で解放してください。
     *  @return         @c COM_UTIL_OK (成功)、または負値のエラー コードです。
     */
    SJ_EXPORT int SJ_API sj_to_json(const sj_struct_desc *desc, const void *instance, cJSON **json_out);

    /**
     *  @brief          cJSON オブジェクトの内容を構造体インスタンスへ書き戻します。
     *
     *  @param[in]      desc        構造体の記述子です。
     *  @param[in]      json        変換元の cJSON オブジェクトです。
     *  @param[out]     instance    書き戻し先の構造体インスタンスです。\n
     *                              失敗時、途中まで書き込まれている場合があります。
     *  @return         @c COM_UTIL_OK (成功)、または負値のエラー コードです。\n
     *                  キー欠落は @c COM_UTIL_ERR_MISSING_REQUIRED、型不一致や NULL 引数は
     *                  @c COM_UTIL_ERR_INVALID_ARGUMENT、文字列がバッファーを超える場合は
     *                  @c COM_UTIL_ERR_BUFFER_TOO_SMALL を返します。
     */
    SJ_EXPORT int SJ_API sj_from_json(const sj_struct_desc *desc, const cJSON *json, void *instance);

    /**
     *  @brief          構造体インスタンスを JSON テキスト ファイルへ書き出します。
     *
     *  @param[in]      desc        構造体の記述子です。
     *  @param[in]      instance    書き出し元の構造体インスタンスです。
     *  @param[in]      path        書き出し先のファイル パスです。
     *  @return         @c COM_UTIL_OK (成功)、または負値のエラー コードです。
     */
    SJ_EXPORT int SJ_API sj_save_file(const sj_struct_desc *desc, const void *instance, const char *path);

    /**
     *  @brief          JSON テキスト ファイルを読み込み、構造体インスタンスへ書き戻します。
     *
     *  @param[in]      desc        構造体の記述子です。
     *  @param[out]     instance    書き戻し先の構造体インスタンスです。
     *  @param[in]      path        読み込み元のファイル パスです。
     *  @return         @c COM_UTIL_OK (成功)、または負値のエラー コードです。
     */
    SJ_EXPORT int SJ_API sj_load_file(const sj_struct_desc *desc, void *instance, const char *path);

    /**
     *  @brief          記述子の階層をメニュー形式で辿り、対話形式で構造体インスタンスの値を編集します。
     *
     *  フィールド一覧を番号付きで表示し、番号入力でネスト構造体/配列要素へ降り、
     *  スカラー フィールドで値を入力すると即座に @p instance へ書き込みます。\n
     *  ファイルへの保存は行いません。呼び出し元が必要に応じて @ref sj_save_file を
     *  呼び出してください。
     *
     *  @param[in]      desc        構造体の記述子です。NULL を渡してはなりません。
     *  @param[in,out]  instance    編集対象の構造体インスタンスです。NULL を渡してはなりません。
     *  @return         @c COM_UTIL_OK (成功、正常に対話セッションを終了)、または負値のエラー
     *                  コードです。@c COM_UTIL_ERR_EOF / @c COM_UTIL_ERR_CANCELED は、
     *                  標準入力の EOF または Ctrl+C による中断を表します。
     */
    SJ_EXPORT int SJ_API sj_patch_interactive(const sj_struct_desc *desc, void *instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* STRUCT_JSON_H */
