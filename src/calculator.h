/**
 * @file calculator.h
 * @brief 簡単な計算機のヘッダーファイル
 * @details
 * 詳細な説明<br>
 * 詳細な説明 (2 行目)
 * @author あなたの名前
 * @date 2025-06-27
 * @version
 * - version 1 の説明
 * - version 2 の説明
 * @copyright コピーライト
 * @since いつから利用可能かを示す。
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

/**
 * @defgroup public_api 公開API
 * @brief 外部公開するAPI群
 * @details 詳細な説明文
 * @note 補足
 * @warning 警告
 * @attention 注意
 */

/**
 * @ingroup public_api
 * @brief ユーザー情報を保持する構造体
 */
typedef struct {
    int id;             /*!< ユーザーID */
    const char *name;   /*!< ユーザー名 */
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
