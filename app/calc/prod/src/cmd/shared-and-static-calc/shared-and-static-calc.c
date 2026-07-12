/**
 *******************************************************************************
 *  @file           src/cmd/shared-and-static-calc/shared-and-static-calc.c
 *  @brief          動的リンクと静的リンクの計算関数を呼び出すコマンドを実装します。
 *  @author         c-modenization-kit sample team
 *  @date           2025/11/22
 *  @version        1.0.0
 *
 *  コマンド ライン引数から 2 つの整数を受け取り、
 *  calc 関数、add, subtract, multiply, divide 関数を使用して
 *  加算結果を標準出力に出力します。
 *
 *  @copyright      Copyright (C) CompanyName, Ltd. 2025. All rights reserved.
 *
 *******************************************************************************
 */

#include <calc.h>
#include <calcbase.h>
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
    ./add 10 + 20
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
    com_util_argparser_init("動的リンクと静的リンクの計算結果を比較します。");
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

    int parse_result = com_util_argparser_parse(argc, argv);
    if (need_help != 0)
    {
        com_util_argparser_print_usage(stdout);
        return EXIT_SUCCESS;
    }
    if (parse_result != COM_UTIL_ARGPARSER_OK)
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

    switch (operator_value[0])
    {
    case '+':
    {
        int result_shared;
        if (calcHandler(CALC_KIND_ADD, arg1, arg3, &result_shared) != 0)
        {
            fprintf(stderr, "Error: calcHandler failed\n");
            return EXIT_FAILURE;
        }
        printf("result_shared: %d\n", result_shared);

        int result_static;
        if (add(arg1, arg3, &result_static) != 0)
        {
            fprintf(stderr, "Error: add failed\n");
            return EXIT_FAILURE;
        }
        printf("result_static: %d\n", result_static);

        break;
    }
    case '-':
    {
        int result_shared;
        if (calcHandler(CALC_KIND_SUBTRACT, arg1, arg3, &result_shared) != 0)
        {
            fprintf(stderr, "Error: calcHandler failed\n");
            return EXIT_FAILURE;
        }
        printf("result_shared: %d\n", result_shared);

        int result_static;
        if (subtract(arg1, arg3, &result_static) != 0)
        {
            fprintf(stderr, "Error: subtract failed\n");
            return EXIT_FAILURE;
        }
        printf("result_static: %d\n", result_static);

        break;
    }
    case 'x':
    {
        int result_shared;
        if (calcHandler(CALC_KIND_MULTIPLY, arg1, arg3, &result_shared) != 0)
        {
            fprintf(stderr, "Error: calcHandler failed\n");
            return EXIT_FAILURE;
        }
        printf("result_shared: %d\n", result_shared);

        int result_static;
        if (multiply(arg1, arg3, &result_static) != 0)
        {
            fprintf(stderr, "Error: multiply failed\n");
            return EXIT_FAILURE;
        }
        printf("result_static: %d\n", result_static);

        break;
    }
    case '/':
    {
        int result_shared;
        if (calcHandler(CALC_KIND_DIVIDE, arg1, arg3, &result_shared) != 0)
        {
            fprintf(stderr, "Error: calcHandler failed\n");
            return EXIT_FAILURE;
        }
        printf("result_shared: %d\n", result_shared);

        int result_static;
        if (divide(arg1, arg3, &result_static) != 0)
        {
            fprintf(stderr, "Error: divide failed\n");
            return EXIT_FAILURE;
        }
        printf("result_static: %d\n", result_static);

        break;
    }
    default:
        fprintf(stderr, "Error: operator must be one of +, -, x, /.\n\n");
        com_util_argparser_print_usage(stderr);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
