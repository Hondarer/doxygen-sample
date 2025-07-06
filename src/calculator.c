/**
 *******************************************************************************
 *  @file           calculator.c
 *  @brief          ファイルの概要を表します。
 *  @author         初版作成者を表します。
 *  @date           yyyy/mm/dd (初版作成年月日を表します。)
 *  @version        現在のバージョンを表します。
 *  @note           一般的な注釈を記載します。
 *  @remarks        詳細な説明や補足情報を記載します。
 *  @attention      注意を記載します。
 *  @warning        警告を記載します。
 *  @deprecated     非推奨であることを記載します。
 *  @since          コードや API がいつから利用可能になったかを記載します。
 *  @par            History
 *                  - yyyy/mm/dd [修正ID](https://example.com/id/1234) 修正の概要
 *                      - 子リスト1
 *                      - 子リスト2
 *                  - yyyy/mm/dd [修正ID](https://example.com/id/5678) 修正の概要
 *                      - 子リスト1  
 *                        子リスト1の続き
 *  @todo           Todo リストを記載します。この行はリスト名となります (省略可能)。
 *                  - 子リスト1
 *                  - 子リスト2
 *                  - 子リスト3
 * 
 *  タグのないコメントは、details として扱われます。
 * 
 *  PlantUML の図を挿入することができます。<br>
 *  VSCode の PlantUML プラグインを使用するために、行頭の * は記載しないことを推奨します。
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
 * 
 *  @copyright Copyright (c) YYYY Sample Inc. All Rights reserved.
 * 
 *******************************************************************************
 */

#include "calculator.h"

/**
 * @ingroup public_api
 * @brief 二つの整数を加算する関数
 * @param[in] a 第一オペランド
 * @param[in] b 第二オペランド
 * @return 加算結果
 * @pre 関数の前提条件。
 * @post 関数の後の保証。
 * @deprecated 非推奨の機能であることを示す。
 * @since いつから利用可能かを示す。
 * @remarks この関数はスレッド セーフです。
 * @details
 * 詳細な説明や、特定の使用例などの追加情報を  
 * 複数行にわたって記述することができます。
 * 
 * 改行は、`br` タグ<br>
 * と 2 個の空白どちらでも動作します。
 *
 * - 箇条書き1
 *     - 箇条書き1-1
 *     - 箇条書き1-2
 * - 箇条書き2
 *
 * このように、箇条書きも記載できます。
 * @todo
 * - コメントを記載する
 * - ログ機能を実装する
 * - 将来的に浮動小数点対応を追加する
 */
int add(int a, int b)
{
    return a + b;
}

/**
 * @ingroup public_api
 * @brief 二つの整数を減算する関数
 * @param[in] a 被減数
 * @param[in] b 減数
 * @todo 単項目の Todo
 * @return 減算結果
 */
int subtract(int a, int b)
{
    return a - b;
}

/**
 * @ingroup public_api
 * @brief 二つの整数を乗算する関数
 * @param[in] a 第一因数
 * @param[in] b 第二因数
 * @return 乗算結果
 */
int multiply(int a, int b)
{
    return a * b;
}

/**
 * @ingroup public_api
 * @brief 二つの整数を除算する関数
 * @param[in] a 被除数
 * @param[in] b 除数
 * @return 除算結果
 * @details
 * PlantUML の図を挿入することができます。<br>
 * VSCode の PlantUML プラグインを使用するために、行頭の * は記載しないことを推奨します。
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
 * @note 特別な注意事項を示します。
 * @warning b が 0 の場合、結果は未定義です
 */
double divide(int a, int b)
{
    if (b == 0)
    {
        return 0.0; // エラー処理は簡略化
    }
    return (double)a / b;
}
