/**
 *******************************************************************************
 *  @file           struct_meta_sample.c
 *  @brief          struct_meta ライブラリの動作確認コマンドです。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  使用方法:
    @code{.sh}
    struct-meta-sample
    struct-meta-sample --help
    @endcode
 *
 *  起動後は対話でサブコマンドを発行します。\n
 *  `load <path>`、`save <path>`、`cat <path>` はファイル名を引数に取ります。\n
 *  `patch` はメニュー形式、`patch <path>` はパス指定で編集対象を選びます。\n
 *  その他は `init` / `dump` / `help` / `exit` です。\n
 *  ルートメニューの空行は `help` と同じです。終了は `exit` です。\n
 *  記述子は型一覧の @c SAMPLE_TYPES_PERSON だけを使います。\n
 *  領域は記述子のサイズで確保し、`init` はゼロ初期化します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <struct_meta/json/file.h>
#include <struct_meta/patch/patch.h>
#include <struct_meta/print/print.h>

#include <cplat/base/result.h>
#include <cplat/base/error.h>
#include <cplat/argparser/argparser.h>
#include <cplat/console/console.h>
#include <cplat/crt/stdio.h>
#include <cplat/prompt/prompt.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gen/sample_types_meta.h"

/** サブコマンド 1 行を受けるバッファーのバイト数です。 */
#define SAMPLE_CMD_LINE_BYTES 256

/** cat コマンドが一度に読み取るバイト数です。 */
#define SAMPLE_CAT_BUFFER_BYTES 4096

/**
 *  @brief          型一覧から person の記述子を取得します。
 */
static const struct_meta_descriptor *person_desc(void)
{
    const struct_meta_descriptor *desc = sample_types_meta_find("person");
    if (desc == NULL)
    {
        fprintf(stderr, "struct-meta-sample: person の記述子を取得できません\n");
    }
    return desc;
}

static void print_commands(void)
{
    fprintf(stderr, "commands: init  load <path>  patch [field-path]  save <path>  cat <path>  dump  help  exit\n");
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
        fprintf(stderr, "struct-meta-sample: ファイル名を指定してください\n");
        return NULL;
    }
    return args;
}

static int ensure_ready(int has_data)
{
    if (has_data == 0)
    {
        fprintf(stderr, "struct-meta-sample: 先に init または load してください\n");
        return 0;
    }
    return 1;
}

static void cmd_init(const struct_meta_descriptor *desc, void *instance, int *has_data)
{
    memset(instance, 0, desc->size);
    *has_data = 1;
}

static void cmd_load(const struct_meta_descriptor *desc, void *instance, const char *path, int *has_data)
{
    int ret = struct_meta_json_file_load(desc, path, instance);
    if (ret != CPLAT_OK)
    {
        fprintf(stderr, "struct-meta-sample: 読み込みに失敗しました (結果コード %d): %s\n", ret, path);
        return;
    }
    *has_data = 1;
}

static void cmd_save(const struct_meta_descriptor *desc, const void *instance, const char *path, int has_data)
{
    int ret;

    if (ensure_ready(has_data) == 0)
    {
        return;
    }
    ret = struct_meta_json_file_save(desc, instance, path);
    if (ret != CPLAT_OK)
    {
        fprintf(stderr, "struct-meta-sample: 保存に失敗しました (結果コード %d): %s\n", ret, path);
    }
}

/**
 *  @brief          指定ファイルの内容を解釈せず標準出力へ表示します。
 */
static void cmd_cat(const char *path)
{
    unsigned char buffer[SAMPLE_CAT_BUFFER_BYTES];
    unsigned char last_byte = 0U;
    int has_output = 0;
    cplat_error error;
    FILE *stream = cplat_fopen(path, "rb", &error);
    if (stream == NULL)
    {
        fprintf(stderr, "struct-meta-sample: cat の入力ファイルを開けません (結果コード %d): %s\n",
                cplat_error_to_result(&error), path);
        return;
    }

    for (;;)
    {
        size_t read_count = cplat_fread(buffer, 1U, sizeof(buffer), stream, &error);
        if (read_count == 0U)
        {
            if (cplat_error_is_set(&error) != 0)
            {
                fprintf(stderr, "struct-meta-sample: cat の入力ファイルを読み取れません (結果コード %d): %s\n",
                        cplat_error_to_result(&error), path);
            }
            break;
        }

        /* 要求サイズ未満の最終ブロックも、EOF 判定より先に必ず出力します。 */
        {
            size_t written_count = cplat_fwrite(buffer, 1U, read_count, stdout, &error);
            if (written_count != read_count)
            {
                fprintf(stderr, "struct-meta-sample: cat の標準出力へ書き込めません (結果コード %d): %s\n",
                        cplat_error_to_result(&error), path);
                (void)cplat_fclose(stream, NULL);
                return;
            }
            last_byte = buffer[read_count - 1U];
            has_output = 1;
        }

        if (read_count < sizeof(buffer))
        {
            break;
        }
    }

    /* 最終行に改行がない場合も、次の対話プロンプトと連結しないよう改行します。 */
    if ((has_output != 0) && (last_byte != (unsigned char)'\n'))
    {
        static const unsigned char newline[] = {'\n'};
        if (cplat_fwrite(newline, 1U, sizeof(newline), stdout, &error) != sizeof(newline))
        {
            fprintf(stderr, "struct-meta-sample: cat の最終改行を出力できません (結果コード %d): %s\n",
                    cplat_error_to_result(&error), path);
            (void)cplat_fclose(stream, NULL);
            return;
        }
    }
    (void)cplat_fflush(stdout, NULL);

    if (cplat_fclose(stream, &error) != 0)
    {
        fprintf(stderr, "struct-meta-sample: cat の入力ファイルを閉じられません (結果コード %d): %s\n",
                cplat_error_to_result(&error), path);
    }
}

static void cmd_patch(const struct_meta_descriptor *desc, void *instance, const char *path, int has_data)
{
    int ret;

    if (ensure_ready(has_data) == 0)
    {
        return;
    }
    if (path[0] == '\0')
    {
        ret = struct_meta_patch_interactive(desc, instance);
    }
    else
    {
        ret = struct_meta_patch_path_interactive(desc, instance, path);
    }
    if (ret != CPLAT_OK)
    {
        if (path[0] == '\0')
        {
            fprintf(stderr, "struct-meta-sample: 対話パッチが中断されました (結果コード %d)\n", ret);
        }
        else
        {
            fprintf(stderr, "struct-meta-sample: パス指定の対話パッチに失敗しました (結果コード %d): %s\n", ret, path);
        }
    }
}

static void cmd_dump(const struct_meta_descriptor *desc, const void *instance, int has_data)
{
    int ret;

    if (ensure_ready(has_data) == 0)
    {
        return;
    }
    ret = struct_meta_print_write(desc, instance, stdout);
    if (ret != CPLAT_OK)
    {
        fprintf(stderr, "struct-meta-sample: 表示に失敗しました (結果コード %d)\n", ret);
    }
}

int main(int argc, char **argv)
{
    const struct_meta_descriptor *desc;
    void *instance = NULL;
    int has_data = 0;
    int exit_code = 0;
    cplat_prompt *prompt = NULL;
    char line[SAMPLE_CMD_LINE_BYTES];
    int need_help = 0;
    int parse_result;

    cplat_console_init();
    cplat_argparser_init(argc, argv, "struct-meta の動作確認コマンドです。起動後は対話コマンドを入力します。");
    (void)cplat_argparser_register_flag("-h", "--help", "ヘルプを表示します。", &need_help);
    if (cplat_argparser_get_register_error_count() > 0U)
    {
        (void)cplat_argparser_print_register_error_messages(stderr);
        return EXIT_FAILURE;
    }
    parse_result = cplat_argparser_parse();
    if (need_help != 0)
    {
        (void)cplat_argparser_print_usage(stdout);
        return EXIT_SUCCESS;
    }
    if (parse_result != CPLAT_OK)
    {
        (void)cplat_argparser_print_error_messages(stderr);
        (void)cplat_argparser_print_usage(stderr);
        return EXIT_FAILURE;
    }

    desc = person_desc();
    if (desc == NULL)
    {
        return 1;
    }

    instance = malloc(desc->size);
    if (instance == NULL)
    {
        fprintf(stderr, "struct-meta-sample: 領域を確保できません\n");
        return 1;
    }

    prompt = cplat_prompt_create(NULL);
    if (prompt == NULL)
    {
        fprintf(stderr, "struct-meta-sample: プロンプトを作成できません\n");
        free(instance);
        return 1;
    }

    print_commands();

    for (;;)
    {
        const char *args = NULL;
        int ret = cplat_prompt_readline(prompt, line, sizeof(line), "struct-meta-sample> ");
        if ((ret == CPLAT_ERR_EOF) || (ret == CPLAT_ERR_CANCELED))
        {
            exit_code = (ret == CPLAT_ERR_EOF) ? 0 : 1;
            break;
        }
        if (ret != CPLAT_OK)
        {
            fprintf(stderr, "struct-meta-sample: 入力に失敗しました (結果コード %d)\n", ret);
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
            cmd_patch(desc, instance, args, has_data);
        }
        else if (match_command(line, "save", &args) != 0)
        {
            const char *path = require_path(args);
            if (path != NULL)
            {
                cmd_save(desc, instance, path, has_data);
            }
        }
        else if (match_command(line, "cat", &args) != 0)
        {
            const char *path = require_path(args);
            if (path != NULL)
            {
                cmd_cat(path);
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

    cplat_prompt_dispose(prompt);
    free(instance);
    return exit_code;
}
