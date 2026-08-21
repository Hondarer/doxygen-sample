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
    struct-json-sample
    @endcode
 *
 *  起動後は対話でサブコマンドを発行します。\n
 *  `load <path>` と `save <path>` はファイル名を引数に取ります。\n
 *  その他は `init` / `patch` / `dump` / `help` / `exit` です。\n
 *  ルートメニューの空行は `help` と同じです。終了は `exit` です。\n
 *  記述子は型一覧の @c SAMPLE_TYPES_PERSON だけを使います。\n
 *  領域は記述子のサイズで確保し、`init` はゼロ初期化します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <struct_json/struct_json.h>

#include <com_util/base/result.h>
#include <com_util/prompt/prompt.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gen/sample_types_meta.h"

/** サブコマンド 1 行を受けるバッファーのバイト数です。 */
#define SAMPLE_CMD_LINE_BYTES 256

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
    fprintf(stderr, "usage: %s\n", prog);
}

static void print_commands(void)
{
    fprintf(stderr, "commands: init  load <path>  patch  save <path>  dump  help  exit\n");
    fprintf(stderr, "          (空行は help、終了は exit)\n");
}

/**
 *  @brief          行先頭がコマンド名と一致するかを判定し、残りの引数を返します。
 *
 *  @param[in]      line        入力行です。
 *  @param[in]      cmd         コマンド名です。
 *  @param[out]     args_out    コマンド名の後の引数 (先行空白は除く) です。
 *  @return         一致すれば 1、しなければ 0 です。
 */
static int match_command(const char *line, const char *cmd, const char **args_out)
{
    size_t cmd_len = strlen(cmd);

    if (strncmp(line, cmd, cmd_len) != 0)
    {
        return 0;
    }
    if (line[cmd_len] == '\0')
    {
        *args_out = "";
        return 1;
    }
    if ((line[cmd_len] == ' ') || (line[cmd_len] == '\t'))
    {
        const char *args = line + cmd_len;
        while ((*args == ' ') || (*args == '\t'))
        {
            args++;
        }
        *args_out = args;
        return 1;
    }
    return 0;
}

static const char *require_path(const char *args)
{
    if ((args == NULL) || (args[0] == '\0'))
    {
        fprintf(stderr, "struct-json-sample: ファイル名を指定してください\n");
        return NULL;
    }
    return args;
}

static int ensure_ready(int has_data)
{
    if (has_data == 0)
    {
        fprintf(stderr, "struct-json-sample: 先に init または load してください\n");
        return 0;
    }
    return 1;
}

static void cmd_init(const sj_struct_desc *desc, void *instance, int *has_data)
{
    memset(instance, 0, desc->size);
    *has_data = 1;
}

static void cmd_load(const sj_struct_desc *desc, void *instance, const char *path, int *has_data)
{
    int ret = sj_load_file(desc, instance, path);
    if (ret != COM_UTIL_OK)
    {
        fprintf(stderr, "struct-json-sample: 読み込みに失敗しました (結果コード %d): %s\n", ret, path);
        return;
    }
    *has_data = 1;
}

static void cmd_save(const sj_struct_desc *desc, const void *instance, const char *path, int has_data)
{
    int ret;

    if (ensure_ready(has_data) == 0)
    {
        return;
    }
    ret = sj_save_file(desc, instance, path);
    if (ret != COM_UTIL_OK)
    {
        fprintf(stderr, "struct-json-sample: 保存に失敗しました (結果コード %d): %s\n", ret, path);
    }
}

static void cmd_patch(const sj_struct_desc *desc, void *instance, int has_data)
{
    int ret;

    if (ensure_ready(has_data) == 0)
    {
        return;
    }
    ret = sj_patch_interactive(desc, instance);
    if (ret != COM_UTIL_OK)
    {
        fprintf(stderr, "struct-json-sample: 対話パッチが中断されました (結果コード %d)\n", ret);
    }
}

static void cmd_dump(const sj_struct_desc *desc, const void *instance, int has_data)
{
    int ret;

    if (ensure_ready(has_data) == 0)
    {
        return;
    }
    ret = sj_print(desc, instance, stdout);
    if (ret != COM_UTIL_OK)
    {
        fprintf(stderr, "struct-json-sample: 表示に失敗しました (結果コード %d)\n", ret);
    }
}

int main(int argc, char **argv)
{
    const sj_struct_desc *desc;
    void *instance = NULL;
    int has_data = 0;
    int exit_code = 0;
    com_util_prompt *prompt = NULL;
    char line[SAMPLE_CMD_LINE_BYTES];

    if (argc != 1)
    {
        print_usage(argv[0]);
        return 1;
    }

    desc = person_desc();
    if (desc == NULL)
    {
        return 1;
    }

    instance = malloc(desc->size);
    if (instance == NULL)
    {
        fprintf(stderr, "struct-json-sample: 領域を確保できません\n");
        return 1;
    }

    prompt = com_util_prompt_create(NULL);
    if (prompt == NULL)
    {
        fprintf(stderr, "struct-json-sample: プロンプトを作成できません\n");
        free(instance);
        return 1;
    }

    print_commands();

    for (;;)
    {
        const char *args = NULL;
        int ret = com_util_prompt_readline(prompt, line, sizeof(line), "struct-json-sample> ");
        if ((ret == COM_UTIL_ERR_EOF) || (ret == COM_UTIL_ERR_CANCELED))
        {
            exit_code = (ret == COM_UTIL_ERR_EOF) ? 0 : 1;
            break;
        }
        if (ret != COM_UTIL_OK)
        {
            fprintf(stderr, "struct-json-sample: 入力に失敗しました (結果コード %d)\n", ret);
            exit_code = 1;
            break;
        }
        if (line[0] == '\0')
        {
            print_commands();
            continue;
        }

        if (match_command(line, "exit", &args) != 0)
        {
            if (args[0] != '\0')
            {
                print_commands();
            }
            else
            {
                break;
            }
        }
        else if (match_command(line, "help", &args) != 0)
        {
            print_commands();
        }
        else if (match_command(line, "init", &args) != 0)
        {
            if (args[0] != '\0')
            {
                print_commands();
            }
            else
            {
                cmd_init(desc, instance, &has_data);
            }
        }
        else if (match_command(line, "load", &args) != 0)
        {
            const char *path = require_path(args);
            if (path != NULL)
            {
                cmd_load(desc, instance, path, &has_data);
            }
        }
        else if (match_command(line, "patch", &args) != 0)
        {
            if (args[0] != '\0')
            {
                print_commands();
            }
            else
            {
                cmd_patch(desc, instance, has_data);
            }
        }
        else if (match_command(line, "save", &args) != 0)
        {
            const char *path = require_path(args);
            if (path != NULL)
            {
                cmd_save(desc, instance, path, has_data);
            }
        }
        else if (match_command(line, "dump", &args) != 0)
        {
            if (args[0] != '\0')
            {
                print_commands();
            }
            else
            {
                cmd_dump(desc, instance, has_data);
            }
        }
        else
        {
            print_commands();
        }
    }

    com_util_prompt_dispose(prompt);
    free(instance);
    return exit_code;
}
