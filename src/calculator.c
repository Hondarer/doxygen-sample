/**
 * @file calculator.c
 * @brief 計算機の実装ファイル
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
 * 詳細な説明や、特定の使用例などの追加情報を<br>
 * 複数行にわたって記述することができます。
 *
 * - 箇条書き1
 * - 箇条書き2
 *
 * このように、箇条書きも記載できます。
 * @todo
 * - コメントを記載する
 * - ログ機能を実装する
 * - 将来的に浮動小数点対応を追加する
 */
int add(int a, int b) {
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
int subtract(int a, int b) {
    return a - b;
}

/**
 * @ingroup public_api
 * @brief 二つの整数を乗算する関数
 * @param[in] a 第一因数
 * @param[in] b 第二因数
 * @return 乗算結果
 */
int multiply(int a, int b) {
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
double divide(int a, int b) {
    if (b == 0) {
        return 0.0; // エラー処理は簡略化
    }
    return (double)a / b;
}
