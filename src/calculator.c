/**
 *******************************************************************************
 *  @file           calculator.c
 *  @brief          計算機の実装を提供します。
 *  @author         初版作成者
 *  @date           2025/07/08
 *  @version        1.0
 *  @par            History
 *                  - 2025/07/08 新規作成
 *
 *  @copyright Copyright (C) CompanyName, Ltd. 2025. All rights reserved.
 *
 *******************************************************************************
 */

#include "calculator.h"

/**
 *  @ingroup        public_api
 *  @brief          @p a と @p b を加算します。
 *  @param[in]      a 加算数a
 *  @param[in]      b 加算数b
 *  @return         加算結果。
 *  @remarks        この関数はスレッド セーフです。
 */
int add(int a, int b)
{
    return a + b;
}

/**
 *  @ingroup        public_api
 *  @brief          @p a から @p b を減算します。
 *  @param[in]      a 被減数
 *  @param[in]      b 減数
 *  @return         減算結果。
 *  @remarks        この関数はスレッド セーフです。
 */
int subtract(int a, int b)
{
    return a - b;
}

/**
 *  @ingroup        public_api
 *  @brief          @p a と @p b を乗算します。
 *  @param[in]      a 因数a
 *  @param[in]      b 因数b
 *  @return         乗算結果。
 *  @remarks        この関数はスレッド セーフです。
 */
int multiply(int a, int b)
{
    return a * b;
}

/**
 *  @ingroup        public_api
 *  @brief          @p a を @p b で除算します。
 *  @param[in]      a 被除数
 *  @param[in]      b 除数
 *  @return         除算結果。
 *  @warning        @p b が 0 の場合、結果は未定義です。
 *  @remarks        この関数はスレッド セーフです。
 *
    @startuml
        caption 図のテスト
        circle a
        circle b
        rectangle "a/b" as devide
        circle return
        a -> devide : 被除数
        b -> devide : 除数
        devide -> return
    @enduml
 */
double divide(int a, int b)
{
    if (b == 0)
    {
        return ZERO_DEVIDE; // エラー処理は簡略化
    }
    return (double)(a / b);
}
