/*
 * structgen.y
 *
 * typedef struct { field-decl* } Name; の形に限定した宣言だけを解析する
 * bison 文法です。フル C パーサーではありません。
 * 対応スコープの詳細は app/struct-json-sample/docs/architecture.md を参照してください。
 *
 * Phase 2: フィールドの型に同一ヘッダー内の他の typedef struct 名を指定できる
 * (ネスト構造体)。また char[] 以外の型にも固定長配列を指定できる。
 */

%{
#include "structgen_ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int g_line;
int yylex(void);
static void yyerror(const char *msg);

sg_struct_list *g_structs = NULL;
%}

%union {
    char *str;
    long num;
    sg_typespec typespec;
    sg_doc_attrs doc;
    struct sg_field *field;
    struct sg_field_list *field_list;
    struct sg_struct *strct;
}

%token TYPEDEF STRUCT
%token T_INT T_UNSIGNED T_CHAR T_FLOAT T_DOUBLE
%token LBRACE RBRACE LBRACKET RBRACKET SEMI COMMA STAR
%token <str> IDENT
%token <str> DOC_PREFIX DOC_POSTFIX
%token <num> INTEGER
%token OTHER

%type <strct> typedef_struct_decl
%type <field_list> field_decl_list
%type <field> field_decl
%type <typespec> type_spec
%type <str> doc_prefix_tokens
%type <doc> doc_prefix doc_postfix

%%

translation_unit:
      /* empty */
    | translation_unit typedef_struct_decl { sg_struct_list_append(&g_structs, $2); }
    | translation_unit SEMI
    | translation_unit error SEMI { yyerrok; }
    ;

doc_prefix:
      /* empty */ { sg_doc_attrs empty = {0}; $$ = empty; }
    | doc_prefix_tokens
        {
            $$ = sg_doc_attrs_from_raw($1, 0);
            free($1);
        }
    ;

doc_prefix_tokens:
      DOC_PREFIX { $$ = $1; }
    | doc_prefix_tokens DOC_PREFIX { $$ = sg_doc_concat($1, $2); }
    ;

doc_postfix:
      /* empty */ { sg_doc_attrs empty = {0}; $$ = empty; }
    | DOC_POSTFIX
        {
            $$ = sg_doc_attrs_from_raw($1, 1);
            free($1);
        }
    ;

typedef_struct_decl:
      doc_prefix TYPEDEF STRUCT LBRACE field_decl_list RBRACE IDENT SEMI doc_postfix
        {
            sg_doc_attrs attrs = sg_doc_attrs_choose($1, $9);
            $$ = sg_struct_create($7, $5, attrs.brief);
            free(attrs.json_name);
        }
    | doc_prefix TYPEDEF STRUCT IDENT LBRACE field_decl_list RBRACE IDENT SEMI doc_postfix
        {
            sg_doc_attrs attrs = sg_doc_attrs_choose($1, $10);
            free($4);
            $$ = sg_struct_create($8, $6, attrs.brief);
            free(attrs.json_name);
        }
    ;

field_decl_list:
      field_decl { $$ = sg_field_list_create($1); }
    | field_decl_list field_decl { $$ = sg_field_list_append($1, $2); }
    ;

field_decl:
      doc_prefix type_spec IDENT SEMI doc_postfix
        {
            sg_doc_attrs attrs = sg_doc_attrs_choose($1, $5);
            $$ = sg_field_create($3, $2.name, $2.is_struct, 0, g_line, attrs.brief, attrs.json_name, attrs.json_ignore,
                                 attrs.json_required);
        }
    | doc_prefix type_spec IDENT LBRACKET INTEGER RBRACKET SEMI doc_postfix
        {
            sg_doc_attrs attrs = sg_doc_attrs_choose($1, $8);
            $$ = sg_field_create($3, $2.name, $2.is_struct, $5, g_line, attrs.brief, attrs.json_name, attrs.json_ignore,
                                 attrs.json_required);
        }
    | doc_prefix type_spec STAR IDENT SEMI doc_postfix
        {
            fprintf(stderr, "structgen: %d: ポインター メンバーは非対応です: %s\n", g_line, $4);
            exit(1);
        }
    ;

type_spec:
      T_INT { $$.name = strdup("int"); $$.is_struct = 0; }
    | T_UNSIGNED { $$.name = strdup("unsigned"); $$.is_struct = 0; }
    | T_UNSIGNED T_INT { $$.name = strdup("unsigned"); $$.is_struct = 0; }
    | T_CHAR { $$.name = strdup("char"); $$.is_struct = 0; }
    | T_FLOAT { $$.name = strdup("float"); $$.is_struct = 0; }
    | T_DOUBLE { $$.name = strdup("double"); $$.is_struct = 0; }
    | IDENT { $$.name = $1; $$.is_struct = 1; }
    ;

%%

static void yyerror(const char *msg)
{
    fprintf(stderr, "structgen: %d: %s\n", g_line, msg);
}
