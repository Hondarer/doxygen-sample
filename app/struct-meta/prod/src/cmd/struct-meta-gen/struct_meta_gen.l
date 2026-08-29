/*
 * struct-meta-gen.l
 *
 * typedef struct { field-decl* } Name; の形に限定した宣言だけを解析する
 * ための字句規則です。struct-meta-gen.y (bison) の文法と対で使用します。
 *
 * 入力ヘッダーは、コメント・#include 等の前処理行・
 * typedef struct 宣言のみで構成されている前提とします
 * (app/struct-meta/docs/architecture.md 参照)。
 */

%{
#include "struct_meta_gen_ast.h"
#include "struct_meta_gen.tab.h"

#include <cplat/crt/string.h>

int g_line = 1;
%}

%option noyywrap
%option nounput
%option noinput

%%

"/**<"([^*]|"*"+[^*/])*"*"+"/"  {
                                    for (const char *p = yytext; *p != '\0'; p++) { if (*p == '\n') { g_line++; } }
                                    yylval.str = cplat_strdup(yytext);
                                    return DOC_POSTFIX;
                                }
"/*!<"([^*]|"*"+[^*/])*"*"+"/"  {
                                    for (const char *p = yytext; *p != '\0'; p++) { if (*p == '\n') { g_line++; } }
                                    yylval.str = cplat_strdup(yytext);
                                    return DOC_POSTFIX;
                                }
"///<".*                        { yylval.str = cplat_strdup(yytext); return DOC_POSTFIX; }
"//!<".*                        { yylval.str = cplat_strdup(yytext); return DOC_POSTFIX; }
"/**"([^*]|"*"+[^*/])*"*"+"/"   {
                                    for (const char *p = yytext; *p != '\0'; p++) { if (*p == '\n') { g_line++; } }
                                    if (struct_meta_gen_doc_has_file_tag(yytext) == 0)
                                    {
                                        yylval.str = cplat_strdup(yytext);
                                        return DOC_PREFIX;
                                    }
                                }
"/*!"([^*]|"*"+[^*/])*"*"+"/"   {
                                    for (const char *p = yytext; *p != '\0'; p++) { if (*p == '\n') { g_line++; } }
                                    if (struct_meta_gen_doc_has_file_tag(yytext) == 0)
                                    {
                                        yylval.str = cplat_strdup(yytext);
                                        return DOC_PREFIX;
                                    }
                                }
"///".*                         { yylval.str = cplat_strdup(yytext); return DOC_PREFIX; }
"//!".*                         { yylval.str = cplat_strdup(yytext); return DOC_PREFIX; }
"/*"([^*]|"*"+[^*/])*"*"+"/"    { for (const char *p = yytext; *p != '\0'; p++) { if (*p == '\n') { g_line++; } } }
"//".*                          { /* 行コメント */ }
^[ \t]*#.*                      { /* 前処理行 (#include, #ifndef 等) */ }
[ \t]+                          { /* 空白 */ }
\n                               { g_line++; }

"typedef"                       { return TYPEDEF; }
"struct"                        { return STRUCT; }
"int"                           { return T_INT; }
"unsigned"                      { return T_UNSIGNED; }
"long"                          { return T_LONG; }
"char"                          { return T_CHAR; }
"float"                         { return T_FLOAT; }
"double"                        { return T_DOUBLE; }

[A-Za-z_][A-Za-z0-9_]*          { yylval.str = cplat_strdup(yytext); return IDENT; }
[0-9]+                          { yylval.num = strtol(yytext, NULL, 10); return INTEGER; }

"{"                              { return LBRACE; }
"}"                              { return RBRACE; }
"["                              { return LBRACKET; }
"]"                              { return RBRACKET; }
";"                              { return SEMI; }
","                              { return COMMA; }
"*"                              { return STAR; }

.                                 { return OTHER; }

%%
