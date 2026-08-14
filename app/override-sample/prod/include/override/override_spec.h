/**
 *******************************************************************************
 *  @file           override_spec.h
 *  @brief          動的リンク用 override ライブラリの API を公開します。
 *  @author         c-modenization-kit sample team
 *  @date           2026/02/21
 *  @version        1.0.0
 *
 *  このライブラリは libbase から動的にロードされ、
 *  処理を引き受けるオーバーライド関数を提供します。
 *
 *  @copyright      Copyright (C) CompanyName, Ltd. 2026. All rights reserved.
 *
 *  @hideincludedbygraph
 *
 *******************************************************************************
 */

/* NOTE: このヘッダーは多数のソース ファイルから参照されるため、            */
/*       @hideincludedbygraph によって "Included by" グラフを無効にします。 */

#ifndef OVERRIDE_SPEC_H
#define OVERRIDE_SPEC_H

#include <base/base_const.h>
#include <override/override_export.h>

/**
 *  @ingroup        OVERRIDE_PUBLIC_API
 *  @{
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     *  @brief          sample_func のオーバーライド実装。
     *  @param[in]      a 第一オペランド。
     *  @param[in]      b 第二オペランド。
     *  @param[out]     result 計算結果を格納するポインター。NULL を渡してはなりません。
     *  @return         成功時は @ref BASE_OK を返します。
     *  @return         @p result が NULL の場合は @ref BASE_ERR_INVALID_ARGUMENT を返します。
     *
     *                  libbase の sample_func から動的にロードされ呼び出されます。\n
     *                  a * b を計算して result に格納します。
     *
     *  @par            使用例
        @code{.c}
         int result;
         if (override_func(1, 2, &result) == BASE_OK)
         {
             base_console_output("result: %d\n", result);  // 出力: result: 2
         }
        @endcode
     *
     *  @par            スレッド セーフ
     *  本関数はスレッド セーフではありません。
     *  本関数と同じ共有状態へアクセスする API の呼び出しを、呼び出し側で直列化してください。
     */
    BASE_EXT_EXPORT extern int BASE_EXT_API override_func(int a, int b, int *result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* OVERRIDE_SPEC_H */
