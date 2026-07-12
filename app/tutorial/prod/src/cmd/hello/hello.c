/**
 *******************************************************************************
 *  @file           hello.c
 *  @brief          メッセージを表示するサンプル コマンドを実装します。
 *  @author         Tetsuo Honda
 *  @date           2026/04/23
 *  @version        1.0.0
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <com_util/argparser/argparser.h>
#include <com_util/console/console.h>
#include <stdio.h>
#include <stdlib.h>

/**
 *  @brief          アプリケーションのエントリ ポイント。
 *
 *  @param[in]      argc コマンド ライン引数の個数。
 *  @param[in]      argv コマンド ライン引数の文字列の配列。
 *  @return         常に 0 を返します。
 */
int main(int argc, char *argv[])
{
    com_util_console_init();

    int need_help = 0;

    com_util_argparser_init("メッセージを表示します。");
    com_util_argparser_register_flag("-h", "--help", "ヘルプを表示します。", &need_help);

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

    printf("✨ Hello, c-modernization-kit! ✨\n");

    return EXIT_SUCCESS;
}
