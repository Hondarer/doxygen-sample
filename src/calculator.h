/**
 * @file calculator.h
 * @brief 簡単な計算機のヘッダーファイル
 * @details
 * 詳細な説明<br>
 * 詳細な説明 (2 行目)
 * @author あなたの名前
 * @date 2025-06-27
 * @version バージョン
 * @copyright コピーライト
 * @since
 * - version 1 の説明
 * - version 2 の説明
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

/**
 * @defgroup public_api 公開API
 * @brief 外部公開するAPI群
 */

/**
 * @ingroup public_api
 * @brief ユーザー情報を保持する構造体
 */
typedef struct {
    int id;             /**< ユーザーID */
    const char *name;   /**< ユーザー名 */
} UserInfo;

/**
 * @ingroup public_api
 * @brief 定数
 */
#define TEISU (1)

int add(int a, int b);

int subtract(int a, int b);

int multiply(int a, int b);

double divide(int a, int b);

#endif // CALCULATOR_H
