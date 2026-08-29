/*
 * struct-meta-gen.y
 *
 * typedef struct { field-decl* } Name; の形に限定した宣言だけを解析する
 * bison 文法です。フル C パーサーではありません。
 * 対応スコープの詳細は app/struct-meta/docs/architecture.md を参照してください。
 *
 * Phase 2: フィールドの型に同一ヘッダー内の他の typedef struct 名を指定できる
 * (ネスト構造体)。また char[] 以外の型にも固定長配列を指定できる。
 */

%{
#include "struct_meta_gen_ast.h"

#include <cplat/crt/string.h>

#include <stdio.h>
#include <stdlib.h>

extern int g_line;
int yylex(void);
static void yyerror(const char *msg);

struct_meta_gen_struct_list *g_struct_meta_gen_structs = NULL;
%}

%union {
    char *str;
    long num;
    struct_meta_gen_typespec typespec;
    struct_meta_gen_doc_attrs doc;
    struct struct_meta_gen_field *field;
    struct struct_meta_gen_field_list *field_list;
    struct struct_meta_gen_struct *strct;
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
    | translation_unit typedef_struct_decl { struct_meta_gen_struct_list_append(&g_struct_meta_gen_structs, $2); }
    | translation_unit SEMI
    | translation_unit error SEMI { yyerrok; }
    ;

doc_prefix:
      /* empty */ { struct_meta_gen_doc_attrs empty = {0}; $$ = empty; }
    | doc_prefix_tokens
        {
            $$ = struct_meta_gen_doc_attrs_from_raw($1, 0, g_line);
            free($1);
            if ($$.invalid != 0) { YYABORT; }
        }
    ;

doc_prefix_tokens:
      DOC_PREFIX { $$ = $1; }
    | doc_prefix_tokens DOC_PREFIX { $$ = struct_meta_gen_doc_concat($1, $2); }
    ;

doc_postfix:
      /* empty */ { struct_meta_gen_doc_attrs empty = {0}; $$ = empty; }
    | DOC_POSTFIX
        {
            $$ = struct_meta_gen_doc_attrs_from_raw($1, 1, g_line);
            free($1);
            if ($$.invalid != 0) { YYABORT; }
        }
    ;

typedef_struct_decl:
      doc_prefix TYPEDEF STRUCT LBRACE field_decl_list RBRACE IDENT SEMI doc_postfix
        {
            struct_meta_gen_doc_attrs attrs = struct_meta_gen_doc_attrs_choose($1, $9, g_line);
            if (attrs.invalid != 0) { YYABORT; }
            $$ = struct_meta_gen_struct_create($7, $5, attrs.brief, attrs.attributes);
        }
    | doc_prefix TYPEDEF STRUCT IDENT LBRACE field_decl_list RBRACE IDENT SEMI doc_postfix
        {
            struct_meta_gen_doc_attrs attrs = struct_meta_gen_doc_attrs_choose($1, $10, g_line);
            if (attrs.invalid != 0) { YYABORT; }
            free($4);
            $$ = struct_meta_gen_struct_create($8, $6, attrs.brief, attrs.attributes);
        }
    ;

field_decl_list:
      field_decl { $$ = struct_meta_gen_field_list_create($1); }
    | field_decl_list field_decl { $$ = struct_meta_gen_field_list_append($1, $2); }
    ;

field_decl:
      doc_prefix type_spec IDENT SEMI doc_postfix
        {
            struct_meta_gen_doc_attrs attrs = struct_meta_gen_doc_attrs_choose($1, $5, g_line);
            if (attrs.invalid != 0) { YYABORT; }
            $$ = struct_meta_gen_field_create($3, $2.name, $2.is_struct, 0, g_line, attrs.brief, attrs.attributes);
        }
    | doc_prefix type_spec IDENT LBRACKET INTEGER RBRACKET SEMI doc_postfix
        {
            struct_meta_gen_doc_attrs attrs = struct_meta_gen_doc_attrs_choose($1, $8, g_line);
            if (attrs.invalid != 0) { YYABORT; }
            $$ = struct_meta_gen_field_create($3, $2.name, $2.is_struct, $5, g_line, attrs.brief, attrs.attributes);
        }
    | doc_prefix type_spec STAR IDENT SEMI doc_postfix
        {
            fprintf(stderr, "struct-meta-gen: %d: ポインター メンバーは非対応です: %s\n", g_line, $4);
            exit(1);
        }
    ;

type_spec:
      T_INT { $$.name = cplat_strdup("int"); $$.is_struct = 0; }
    | T_UNSIGNED { $$.name = cplat_strdup("unsigned"); $$.is_struct = 0; }
    | T_UNSIGNED T_INT { $$.name = cplat_strdup("unsigned"); $$.is_struct = 0; }
    | T_CHAR { $$.name = cplat_strdup("char"); $$.is_struct = 0; }
    | T_FLOAT { $$.name = cplat_strdup("float"); $$.is_struct = 0; }
    | T_DOUBLE { $$.name = cplat_strdup("double"); $$.is_struct = 0; }
    | IDENT { $$.name = $1; $$.is_struct = 1; }
    ;

%%

static void yyerror(const char *msg)
{
    fprintf(stderr, "struct-meta-gen: %d: %s\n", g_line, msg);
}
