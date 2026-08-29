/**
 * @file main.c
 * @brief サブフォルダーを含む make ビルドを検証するエントリ ポイントを提供します。
 */

#include <cplat/argparser/argparser.h>
#include <cplat/console/console.h>
#include <stdio.h>
#include <stdlib.h>

#include "sample-app.h"

/**
 * @brief メイン関数
 * @param[in] argc コマンド ライン引数の数。
 * @param[in] argv コマンド ライン引数の配列。
 * @return 0: 正常終了
 */
int main(int argc, char *argv[])
{
    cplat_console_init();

    int need_help = 0;

    cplat_argparser_init(argc, argv, "サブフォルダーを含む make ビルドを検証します。");
    cplat_argparser_register_flag("-h", "--help", "ヘルプを表示します。", &need_help);

    if (cplat_argparser_get_register_error_count() > 0)
    {
        cplat_argparser_print_register_error_messages(stderr);
        return EXIT_FAILURE;
    }

    int parse_result = cplat_argparser_parse();
    if (need_help != 0)
    {
        cplat_argparser_print_usage(stdout);
        return EXIT_SUCCESS;
    }

    if (parse_result != CPLAT_OK)
    {
        cplat_argparser_print_error_messages(stderr);
        cplat_argparser_print_usage(stderr);
        return EXIT_FAILURE;
    }

    int a = 10;
    int b = 20;

    printf("Testing subfolder make for src\n");
    printf("helper_a(%d) = %d\n", a, helper_a(a));
    printf("helper_b(%d) = %d\n", b, helper_b(b));
    printf("helper_a(%d) + helper_b(%d) = %d\n", a, b, helper_a(a) + helper_b(b));

    return EXIT_SUCCESS;
}
