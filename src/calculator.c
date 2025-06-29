/**
 * @file calculator.c
 * @brief 計算機の実装ファイル
 */

#include "calculator.h"

/**
 * @ingroup public_api
 * @brief 二つの整数を加算する関数
 * @param a 第一オペランド
 * @param b 第二オペランド
 * @return 加算結果
 */
int add(int a, int b) {
    return a + b;
}

/**
 * @ingroup public_api
 * @brief 二つの整数を減算する関数
 * @param a 被減数
 * @param b 減数
 * @return 減算結果
 */
int subtract(int a, int b) {
    return a - b;
}

/**
 * @ingroup public_api
 * @brief 二つの整数を乗算する関数
 * @param a 第一因数
 * @param b 第二因数
 * @return 乗算結果
 */
int multiply(int a, int b) {
    return a * b;
}

/**
 * @ingroup public_api
 * @brief 二つの整数を除算する関数
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
 * @param a 被除数
 * @param b 除数
 * @return 除算結果
 * @warning b が 0 の場合、結果は未定義です
 */
double divide(int a, int b) {
    if (b == 0) {
        return 0.0; // エラー処理は簡略化
    }
    return (double)a / b;
}
