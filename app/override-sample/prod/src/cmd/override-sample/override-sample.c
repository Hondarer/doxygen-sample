/**
 *******************************************************************************
 *  @file           override-sample.c
 *  @brief          関数の動的オーバーライドのサンプル コマンドを実装します。
 *  @author         c-modenization-kit sample team
 *  @date           2026/02/21
 *  @version        1.0.0
 *
 *  libbase の func を呼び出し、オーバーライド機能を示すサンプル プログラムです。
 *
 *  @copyright      Copyright (C) CompanyName, Ltd. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <base.h>
#include <com_util/argparser/argparser.h>
#include <com_util/base/error.h>
#include <com_util/console/console.h>
#include <com_util/crt/path.h>
#include <stdio.h>
#include <stdlib.h>

/**
 *  @brief          メイン エントリ ポイント。
 *  @param[in]      argc コマンド ライン引数の数。
 *  @param[in]      argv コマンド ライン引数の配列。
 *  @return         正常終了時は 0 を返します。
 */
int main(int argc, char *argv[])
{
    com_util_console_init();

    int need_help = 0;

    com_util_argparser_default_init("関数の動的オーバーライドのサンプルコマンド。");
    com_util_argparser_default_register_flag("-h", "--help", "ヘルプを表示します。", &need_help);

    if (com_util_argparser_default_get_register_error_count() > 0)
    {
        com_util_argparser_default_print_register_error_messages(stderr);
        return EXIT_FAILURE;
    }

    int parse_result = com_util_argparser_default_parse(argc, argv);

    if (need_help != 0)
    {
        com_util_argparser_default_print_usage(stdout);
        return EXIT_SUCCESS;
    }

    if (parse_result != COM_UTIL_OK)
    {
        com_util_argparser_default_print_error_messages(stderr);
        com_util_argparser_default_print_usage(stderr);
        return EXIT_FAILURE;
    }

    int result;
    int ret;
    char configpath[PLATFORM_PATH_MAX];

    {
        com_util_error error;
        char tmpdir[PLATFORM_PATH_MAX];
        if (com_util_get_temp_dir(tmpdir, sizeof(tmpdir), &error) == COM_UTIL_OK)
        {
            if (com_util_path_concat(configpath, sizeof(configpath), &error, tmpdir, PLATFORM_PATH_SEP,
                                     "libbase_extdef.json") != COM_UTIL_OK)
            {
                fprintf(stderr, "failed to build config path: exceeds PLATFORM_PATH_MAX\n");
                return EXIT_FAILURE;
            }
        }
        else if (com_util_error_is(&error, COM_UTIL_CAUSE_NAME_TOO_LONG) != 0)
        {
            fprintf(stderr, "failed to build config path: exceeds PLATFORM_PATH_MAX\n");
            return EXIT_FAILURE;
        }
        else
        {
            configpath[0] = '\0';
        }
    }

    printf("configpath: %s\n", configpath);
    printf("Processing will be extended if defines.\n");
    printf(" e.g.  printf '{\"sample_func\":{\"lib\":\"liboverride\",\"func\":\"override_func\"}}\\n'"
           " > \"%s\"\n",
           configpath);
#if defined(PLATFORM_LINUX)
    printf("       rm \"%s\"\n\n", configpath);
#elif defined(PLATFORM_WINDOWS)
    printf("       del \"%s\"\n\n", configpath);
#endif /* PLATFORM_ */

    printf("--- sym_loader info ---\n");
    ret = base_sym_loader_info();
    printf("ret: %d\n\n", ret);

    ret = sample_func(1, 2, &result);
    base_console_output("ret: %d\n", ret);
    if (ret != BASE_OK)
    {
        fprintf(stderr, "func failed (sample_func(1, 2, &result))\n");
    }
    else
    {
        printf("result: %d\n", result);
    }

    return EXIT_SUCCESS;
}
