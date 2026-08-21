/*
 * structgen.l
 *
 * typedef struct { field-decl* } Name; の形に限定した宣言だけを解析する
 * ための字句規則です。structgen.y (bison) の文法と対で使用します。
 *
 * 入力ヘッダーは、コメント・#include 等の前処理行・
 * typedef struct 宣言のみで構成されている前提とします
 * (app/struct-json-sample/docs/architecture.md 参照)。
 */

%{
#include "structgen_ast.h"
#include "structgen.tab.h"

#include <stdlib.h>
#include <string.h>

int g_line = 1;
%}

%option noyywrap
%option nounput
%option noinput

%%

"/*"([^*]|"*"+[^*/])*"*"+"/"   { for (const char *p = yytext; *p != '\0'; p++) { if (*p == '\n') { g_line++; } } }
"//".*                          { /* 行コメント */ }
^[ \t]*#.*                      { /* 前処理行 (#include, #ifndef 等) */ }
[ \t]+                          { /* 空白 */ }
\n                               { g_line++; }

"typedef"                       { return TYPEDEF; }
"struct"                        { return STRUCT; }
"int"                           { return T_INT; }
"unsigned"                      { return T_UNSIGNED; }
"char"                          { return T_CHAR; }
"float"                         { return T_FLOAT; }
"double"                        { return T_DOUBLE; }

[A-Za-z_][A-Za-z0-9_]*          { yylval.str = strdup(yytext); return IDENT; }
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
