/**
 * @file calculator.h
 * @brief 簡単な計算機のヘッダーファイル
 * @author あなたの名前
 * @date 2025-06-27
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

/**
 * @brief 二つの整数を加算する関数
 * @param a 第一オペランド
 * @param b 第二オペランド
 * @return 加算結果
 */
int add(int a, int b);

/**
 * @brief 二つの整数を減算する関数
 * @param a 被減数
 * @param b 減数
 * @return 減算結果
 */
int subtract(int a, int b);

/**
 * @brief 二つの整数を乗算する関数
 * @param a 第一因数
 * @param b 第二因数
 * @return 乗算結果
 */
int multiply(int a, int b);

/**
 * @brief 二つの整数を除算する関数
 * @param a 被除数
 * @param b 除数
 * @return 除算結果
 * @warning b が 0 の場合、結果は未定義です
 */
double divide(int a, int b);

#endif // CALCULATOR_H
