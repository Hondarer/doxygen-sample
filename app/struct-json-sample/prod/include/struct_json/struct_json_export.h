/**
 *******************************************************************************
 *  @file           struct_json_export.h
 *  @brief          struct_json の Windows DLL エクスポートおよび呼び出し規約マクロを定義します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *  @hideincludedbygraph
 *
 *******************************************************************************
 */

/* NOTE: このヘッダーは多数のソース ファイルから参照されるため、            */
/*       @hideincludedbygraph によって "Included by" グラフを無効にします。 */

#ifndef STRUCT_JSON_EXPORT_H
#define STRUCT_JSON_EXPORT_H

/**
 *  @ingroup        STRUCT_JSON_PUBLIC_API
 *  @{
 */

#ifdef DOXYGEN

    /**
     *  @brief          DLL エクスポート/インポート制御マクロ。
     *
     *  ビルド条件に応じて以下の値を取ります。
     *
     *  | 条件                                                | 値                                        |
     *  | ---------------------------------------------------- | ----------------------------------------- |
     *  | Linux / `SJ_STATIC` 定義時 (静的リンク)             | (空)                                      |
     *  | Linux / 共有ライブラリ (静的リンクでない)           | `__attribute__((visibility("default")))` |
     *  | Windows / `__INTELLISENSE__` 定義時                 | (空)                                      |
     *  | Windows / `SJ_STATIC` 定義時 (静的リンク)           | (空)                                      |
     *  | Windows / `SJ_EXPORTS` 定義時 (DLL ビルド)          | `__declspec(dllexport)`                   |
     *  | Windows / `SJ_EXPORTS` 未定義時 (DLL 利用側)        | `__declspec(dllimport)`                   |
     */
    #define SJ_EXPORT

    /**
     *  @brief          呼び出し規約マクロ。
     *
     *  Windows 環境では `__stdcall` 呼び出し規約を指定します。\n
     *  Linux (非 Windows) 環境では空に展開されます。
     */
    #define SJ_API

#else /* !DOXYGEN */

    #ifndef SJ_STATIC
        #define SJ_STATIC 0
    #endif /* SJ_STATIC */
    #ifndef SJ_EXPORTS
        #define SJ_EXPORTS 0
    #endif /* SJ_EXPORTS */
    #include <com_util/base/dll_exports.h>
    #define SJ_EXPORT COM_UTIL_DLL_EXPORT(SJ)
    #define SJ_API    COM_UTIL_DLL_API(SJ)

#endif /* DOXYGEN */

/** @} */

#endif /* STRUCT_JSON_EXPORT_H */
