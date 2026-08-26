/**
 *******************************************************************************
 *  @file           src/cmd/add/add.c
 *  @brief          2 つの整数を加算するコマンドを実装します。
 *  @author         c-modenization-kit sample team
 *  @date           2025/11/22
 *  @version        1.0.0
 *
 *  コマンド ライン引数から 2 つの整数を受け取り、calcbase_add 関数を使用して
 *  加算結果を標準出力に出力します。
 *
 *  @copyright      Copyright (C) CompanyName, Ltd. 2025. All rights reserved.
 *
 *******************************************************************************
 */

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
    ./calcbase_add 10 20
    // 出力: 30
    @endcode
 *
 *  @attention      引数は正確に 2 つ必要です。
 */
int main(int argc, char *argv[])
{
    com_util_console_init();

    int need_help = 0;
    int arg1 = 0;
    int arg2 = 0;
    com_util_argparser_init("2 つの整数を加算します。");
    com_util_argparser_register_flag("-h", "--help", "ヘルプを表示します。", &need_help);
    com_util_argparser_register_positional_int("arg1", "第一オペランド。", COM_UTIL_ARGPARSER_REQUIRED, &arg1);
    com_util_argparser_register_positional_int("arg2", "第二オペランド。", COM_UTIL_ARGPARSER_REQUIRED, &arg2);
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
    if (parse_result != COM_UTIL_OK)
    {
        com_util_argparser_print_error_messages(stderr);
        com_util_argparser_print_usage(stderr);
        return EXIT_FAILURE;
    }

    int result;

    if (calcbase_add(arg1, arg2, &result) != CALC_OK)
    {
        fprintf(stderr, "Error: calcbase_add failed\n");
        return EXIT_FAILURE;
    }

    printf("%d\n", result);

    return EXIT_SUCCESS;
}
