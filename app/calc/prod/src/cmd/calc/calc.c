/**
 *******************************************************************************
 *  @file           src/cmd/calc/calc.c
 *  @brief          指定された演算子に基づいて 2 つの整数を計算するコマンドを実装します。
 *  @author         c-modenization-kit sample team
 *  @date           2025/11/22
 *  @version        1.0.0
 *
 *  コマンド ライン引数から 2 つの整数と演算子を受け取り、calc_handler 関数を使用して
 *  計算結果を標準出力に出力します。
 *
 *  @copyright      Copyright (C) CompanyName, Ltd. 2025. All rights reserved.
 *
 *******************************************************************************
 */

#include <calc.h>
#include <com_util/argparser/argparser.h>
#include <com_util/console/console.h>
#include <stdio.h>
#include <stdlib.h>

/**
 *  @brief          プログラムのエントリ ポイント。
 *  @param[in]      argc コマンド ライン引数の数。
 *  @param[in]      argv コマンド ライン引数の配列。
 *  @return         成功時は 0、失敗時は 0 以外の値を返します。
 *
 *  使用例:
 *
    @code{.c}
    ./calcbase_add 10 + 20
    // 出力: 30
    @endcode
 *
 *  @attention      引数は正確に 3 つ必要です。
 */
int main(int argc, char *argv[])
{
    com_util_console_init();

    int need_help = 0;
    int arg1 = 0;
    int arg3 = 0;
    const char *operator_value = NULL;
    com_util_argparser_init(argc, argv, "指定された演算子に基づいて 2 つの整数を計算します。");
    com_util_argparser_register_flag("-h", "--help", "ヘルプを表示します。", &need_help);
    com_util_argparser_register_positional_int("num1", "第一オペランド。", COM_UTIL_ARGPARSER_REQUIRED, &arg1);
    com_util_argparser_register_positional_string("operator", "+、-、x、/ のいずれか。", COM_UTIL_ARGPARSER_REQUIRED,
                                                  &operator_value);
    com_util_argparser_register_positional_int("num2", "第二オペランド。", COM_UTIL_ARGPARSER_REQUIRED, &arg3);
    if (com_util_argparser_get_register_error_count() > 0)
    {
        com_util_argparser_print_register_error_messages(stderr);
        return EXIT_FAILURE;
    }

    int parse_result = com_util_argparser_parse();
    if (need_help != 0)
    {
        com_util_argparser_print_usage(stdout);
        return EXIT_SUCCESS;
    }
    if (parse_result != COM_UTIL_OK)
    {
        com_util_argparser_print_error_messages(stderr);
        com_util_argparser_print_usage(stderr);
        return EXIT_FAILURE;
    }
    if (operator_value[0] == 0x00 || operator_value[1] != 0x00)
    {
        fprintf(stderr, "Error: operator must be one of +, -, x, /.\n\n");
        com_util_argparser_print_usage(stderr);
        return EXIT_FAILURE;
    }
    int result;

    int kind;
    switch (operator_value[0])
    {
    case '+':
        kind = CALC_KIND_ADD;
        break;
    case '-':
        kind = CALC_KIND_SUBTRACT;
        break;
    case 'x':
        kind = CALC_KIND_MULTIPLY;
        break;
    case '/':
        kind = CALC_KIND_DIVIDE;
        break;
    default:
        fprintf(stderr, "Error: operator must be one of +, -, x, /.\n\n");
        com_util_argparser_print_usage(stderr);
        return EXIT_FAILURE;
    }

    if (calc_handler(kind, arg1, arg3, &result) != CALC_OK)
    {
        fprintf(stderr, "Error: calc_handler failed\n");
        return EXIT_FAILURE;
    }

    printf("%d\n", result);

    return EXIT_SUCCESS;
}
