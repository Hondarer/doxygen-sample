/**
 *******************************************************************************
 *  @file           structgen_main.c
 *  @brief          structgen (ヘッダー解析ツール) のエントリー ポイントです。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  使用方法:
    @code{.sh}
    structgen --header <ヘッダー パス> --out <出力 C ソース パス>
    @endcode
 *
 *  対象ヘッダーを解析し、ヘッダー内の全 `typedef struct` のメタデータ記述子
 *  (`sj_struct_desc`) と型一覧 (enum + 取得関数) を `--out` へ生成します。\n
 *  同名の `.h` も同じディレクトリへ書き出します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include "structgen_ast.h"
#include "structgen_emit.h"

#include <com_util/crt/stdio.h>

#include <stdio.h>
#include <string.h>

extern FILE *yyin;
extern sg_struct_list *g_structs;
int yyparse(void);

static void print_usage(const char *prog)
{
    fprintf(stderr, "usage: %s --header <header> --out <out.c>\n", prog);
}

int main(int argc, char **argv)
{
    const char *header_path = NULL;
    const char *out_path = NULL;

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "--header") == 0) && ((i + 1) < argc))
        {
            header_path = argv[++i];
        }
        else if ((strcmp(argv[i], "--out") == 0) && ((i + 1) < argc))
        {
            out_path = argv[++i];
        }
        else
        {
            fprintf(stderr, "structgen: 未知の引数です: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if ((header_path == NULL) || (out_path == NULL))
    {
        print_usage(argv[0]);
        return 1;
    }

    yyin = com_util_fopen(header_path, "r", NULL);
    if (yyin == NULL)
    {
        fprintf(stderr, "structgen: ヘッダーを開けません: %s\n", header_path);
        return 1;
    }

    if (yyparse() != 0)
    {
        fclose(yyin);
        return 1;
    }
    fclose(yyin);

    if ((g_structs == NULL) || (g_structs->head == NULL))
    {
        fprintf(stderr, "structgen: 構造体が見つかりません (ヘッダー: %s)\n", header_path);
        return 1;
    }

    return sg_emit(g_structs, header_path, out_path);
}
