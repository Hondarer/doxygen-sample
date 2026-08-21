/**
 *******************************************************************************
 *  @file           struct-json-sample.c
 *  @brief          struct_json エンジンの動作確認コマンドです。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  使用方法:
    @code{.sh}
    struct-json-sample --save <path>
    struct-json-sample --load <path> [--dump]
    struct-json-sample --patch <path>
    @endcode
 *
 *  `--save` は組み込みのサンプル値を、structgen が `sample_types.h` から
 *  生成した型一覧の @c SAMPLE_TYPES_PERSON をキーに取得した記述子を使って
 *  JSON ファイルへ書き出します。\n
 *  `--load` は JSON ファイルを読み込み、`--dump` 指定時は内容を表示します。\n
 *  `--patch` は JSON ファイルを読み込み、対話形式でフィールドを編集してから
 *  同じファイルへ書き戻します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <struct_json/struct_json.h>

#include <com_util/base/result.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "gen/sample_types_meta.h"
#include "sample_types.h"

/**
 *  @brief          型一覧から person の記述子を取得します。
 */
static const sj_struct_desc *person_desc(void)
{
    const sj_struct_desc *desc = sample_types_desc(SAMPLE_TYPES_PERSON);
    if (desc == NULL)
    {
        fprintf(stderr, "struct-json-sample: person の記述子を取得できません\n");
    }
    return desc;
}

static void print_usage(const char *prog)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s --save <path>\n", prog);
    fprintf(stderr, "  %s --load <path> [--dump]\n", prog);
    fprintf(stderr, "  %s --patch <path>\n", prog);
}

static void fill_sample_address(address *a, const char *city, int zip)
{
    snprintf(a->city, sizeof(a->city), "%s", city);
    a->zip = zip;
}

static void fill_sample_person(person *p)
{
    p->id = 1;
    p->age = 30U;
    p->score = 92.5;
    snprintf(p->name, sizeof(p->name), "Alice");
    fill_sample_address(&p->home, "Tokyo", 1000000);
    fill_sample_address(&p->addresses[0], "Osaka", 5300000);
    fill_sample_address(&p->addresses[1], "Kyoto", 6000000);
    p->scores[0] = 80;
    p->scores[1] = 90;
    p->scores[2] = 70;
}

static void dump_address(const address *a)
{
    printf("  city=%s zip=%d\n", a->city, a->zip);
}

static void dump_person(const person *p)
{
    printf("id=%d age=%u score=%g name=%s\n", p->id, p->age, p->score, p->name);
    printf("home:\n");
    dump_address(&p->home);
    printf("addresses:\n");
    for (size_t i = 0; i < (sizeof(p->addresses) / sizeof(p->addresses[0])); i++)
    {
        dump_address(&p->addresses[i]);
    }
    printf("scores:");
    for (size_t i = 0; i < (sizeof(p->scores) / sizeof(p->scores[0])); i++)
    {
        printf(" %d", p->scores[i]);
    }
    printf("\n");
}

static int run_save(const char *path)
{
    person p = {0};
    const sj_struct_desc *desc = person_desc();
    if (desc == NULL)
    {
        return 1;
    }
    fill_sample_person(&p);

    int ret = sj_save_file(desc, &p, path);
    if (ret != COM_UTIL_OK)
    {
        fprintf(stderr, "struct-json-sample: 保存に失敗しました (結果コード %d): %s\n", ret, path);
        return 1;
    }
    dump_person(&p);
    return 0;
}

static int run_load(const char *path, int dump)
{
    person p = {0};
    const sj_struct_desc *desc = person_desc();
    if (desc == NULL)
    {
        return 1;
    }

    int ret = sj_load_file(desc, &p, path);
    if (ret != COM_UTIL_OK)
    {
        fprintf(stderr, "struct-json-sample: 読み込みに失敗しました (結果コード %d): %s\n", ret, path);
        return 1;
    }
    if (dump != 0)
    {
        dump_person(&p);
    }
    return 0;
}

static int run_patch(const char *path)
{
    person p = {0};
    const sj_struct_desc *desc = person_desc();
    if (desc == NULL)
    {
        return 1;
    }

    int ret = sj_load_file(desc, &p, path);
    if (ret != COM_UTIL_OK)
    {
        fprintf(stderr, "struct-json-sample: 読み込みに失敗しました (結果コード %d): %s\n", ret, path);
        return 1;
    }

    ret = sj_patch_interactive(desc, &p);
    if (ret != COM_UTIL_OK)
    {
        fprintf(stderr, "struct-json-sample: 対話パッチが中断されました (結果コード %d)\n", ret);
        return 1;
    }

    ret = sj_save_file(desc, &p, path);
    if (ret != COM_UTIL_OK)
    {
        fprintf(stderr, "struct-json-sample: 保存に失敗しました (結果コード %d): %s\n", ret, path);
        return 1;
    }
    dump_person(&p);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--save") == 0)
    {
        return run_save(argv[2]);
    }

    if (strcmp(argv[1], "--load") == 0)
    {
        int dump = 0;
        if ((argc >= 4) && (strcmp(argv[3], "--dump") == 0))
        {
            dump = 1;
        }
        return run_load(argv[2], dump);
    }

    if (strcmp(argv[1], "--patch") == 0)
    {
        return run_patch(argv[2]);
    }

    print_usage(argv[0]);
    return 1;
}
