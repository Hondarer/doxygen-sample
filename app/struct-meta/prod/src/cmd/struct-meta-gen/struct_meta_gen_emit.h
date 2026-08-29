/**
 *******************************************************************************
 *  @file           struct_meta_gen_emit.h
 *  @brief          解析済みの構造体定義から、メタデータ記述子の C ソースを生成します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/16
 *  @version        1.0.0
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#ifndef STRUCT_META_GEN_EMIT_PRIVATE_H
#define STRUCT_META_GEN_EMIT_PRIVATE_H

#include "struct_meta_gen_ast.h"

/**
 *  @brief          ヘッダー内の全構造体の記述子と型一覧を生成し、ファイルへ書き出します。
 *
 *  @param[in]      structs     解析済みの構造体一覧です。1 件以上必要です。
 *  @param[in]      header_path struct-meta-gen の呼び出し元カレント ディレクトリから見た、
 *                              解析元ヘッダーへの相対パスです。生成コードは
 *                              `$(GENDIR)` (呼び出し元のカレント ディレクトリ直下)
 *                              に置かれる前提で、1 階層上として `../<header_path>`
 *                              を `#include` します。
 *  @param[in]      out_path    生成する C ソースの出力先パスです。拡張子は `.c` です。\n
 *                              同名の `.h` (型一覧 enum と取得関数の宣言) も同じ
 *                              ディレクトリへ書き出します。
 *  @return         成功時は 0、失敗時は 0 以外です。
 */
int struct_meta_gen_emit(const struct_meta_gen_struct_list *structs, const char *header_path, const char *out_path);

#endif /* STRUCT_META_GEN_EMIT_PRIVATE_H */
