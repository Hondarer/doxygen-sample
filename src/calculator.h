/**
 *******************************************************************************
 *  @file           calculator.h
 *  @brief          計算機の定義を提供します。
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

#ifndef CALCULATOR_H
#define CALCULATOR_H

/**
 *******************************************************************************
 *  @defgroup       public_api 公開 API
 *  @brief          外部公開する API 群をカテゴライズします。
 *******************************************************************************
 */

/**
 *  @ingroup        public_api
 *  @brief          ユーザー情報を保持する構造体を定義します。
 */
typedef struct
{
    int id;           /*!< ユーザーID */
    const char *name; /*!< ユーザー名 */
} UserInfo;

/**
 *  @ingroup        public_api
 *  @brief          ゼロ除算の戻り値を定義します。
 */
#define ZERO_DEVIDE (0.0)

int add(int a, int b); /* 実装のコメントを参照 */

int subtract(int a, int b); /* 実装のコメントを参照 */

int multiply(int a, int b); /* 実装のコメントを参照 */

double divide(int a, int b); /* 実装のコメントを参照 */

#endif /* CALCULATOR_H */
