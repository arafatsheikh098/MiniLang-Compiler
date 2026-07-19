%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

extern int  yylex(void);
extern int  yylineno;
void        yyerror(const char *msg);

ASTNode *parse_root = NULL;
%}

%union {
    int      ival;
    char    *sval;
    ASTNode *node;
    DataType dtype;
}

%token <ival>  TOK_INTEGER
%token <ival>  TOK_TRUE TOK_FALSE
%token <sval>  TOK_IDENT
%token         TOK_INT TOK_BOOL
%token         TOK_IF TOK_ELSE TOK_WHILE TOK_PRINT
%token         TOK_PLUS TOK_MINUS TOK_STAR TOK_SLASH
%token         TOK_LT TOK_GT TOK_EQ TOK_NEQ
%token         TOK_ASSIGN
%token         TOK_LPAREN TOK_RPAREN
%token         TOK_LBRACE TOK_RBRACE
%token         TOK_SEMI

%type <node>  program stmt_list stmt
%type <node>  decl_stmt assign_stmt if_stmt while_stmt print_stmt block
%type <node>  expr term factor
%type <dtype> type

%left  TOK_EQ TOK_NEQ
%left  TOK_LT TOK_GT
%left  TOK_PLUS TOK_MINUS
%left  TOK_STAR TOK_SLASH
%right UMINUS

%right TOK_ELSE

%start program

%%

program
    : stmt_list
        { parse_root = ast_block($1, 1); }
    ;

stmt_list
    : 
        { $$ = NULL; }
    | stmt stmt_list
        { $$ = ast_stmt_list($1, $2); }
    ;

stmt
    : decl_stmt   { $$ = $1; }
    | assign_stmt { $$ = $1; }
    | if_stmt     { $$ = $1; }
    | while_stmt  { $$ = $1; }
    | print_stmt  { $$ = $1; }
    | block       { $$ = $1; }
    ;

type
    : TOK_INT   { $$ = TYPE_INT;  }
    | TOK_BOOL  { $$ = TYPE_BOOL; }
    ;

decl_stmt
    : type TOK_IDENT TOK_SEMI
        { $$ = ast_decl($2, $1, yylineno); free($2); }
    | type TOK_IDENT TOK_ASSIGN expr TOK_SEMI
        { $$ = ast_decl_assign($2, $1, $4, yylineno); free($2); }
    ;

assign_stmt
    : TOK_IDENT TOK_ASSIGN expr TOK_SEMI
        { $$ = ast_assign($1, $3, yylineno); free($1); }
    ;

if_stmt
    : TOK_IF TOK_LPAREN expr TOK_RPAREN block
        { $$ = ast_if($3, $5, yylineno); }
    | TOK_IF TOK_LPAREN expr TOK_RPAREN block TOK_ELSE block
        { $$ = ast_if_else($3, $5, $7, yylineno); }
    ;

while_stmt
    : TOK_WHILE TOK_LPAREN expr TOK_RPAREN block
        { $$ = ast_while($3, $5, yylineno); }
    ;

print_stmt
    : TOK_PRINT expr TOK_SEMI
        { $$ = ast_print_stmt($2, yylineno); }
    ;

block
    : TOK_LBRACE stmt_list TOK_RBRACE
        { $$ = ast_block($2, yylineno); }
    ;

expr
    : expr TOK_PLUS  expr   { $$ = ast_binop(OP_ADD, $1, $3, yylineno); }
    | expr TOK_MINUS expr   { $$ = ast_binop(OP_SUB, $1, $3, yylineno); }
    | expr TOK_STAR  expr   { $$ = ast_binop(OP_MUL, $1, $3, yylineno); }
    | expr TOK_SLASH expr   { $$ = ast_binop(OP_DIV, $1, $3, yylineno); }
    | expr TOK_LT    expr   { $$ = ast_binop(OP_LT,  $1, $3, yylineno); }
    | expr TOK_GT    expr   { $$ = ast_binop(OP_GT,  $1, $3, yylineno); }
    | expr TOK_EQ    expr   { $$ = ast_binop(OP_EQ,  $1, $3, yylineno); }
    | expr TOK_NEQ   expr   { $$ = ast_binop(OP_NEQ, $1, $3, yylineno); }
    | term                  { $$ = $1; }
    ;

term
    : factor                { $$ = $1; }
    ;

factor
    : TOK_MINUS factor %prec UMINUS
        { $$ = ast_unary_minus($2, yylineno); }
    | TOK_LPAREN expr TOK_RPAREN
        { $$ = $2; }
    | TOK_INTEGER
        { $$ = ast_int_lit($1, yylineno); }
    | TOK_TRUE
        { $$ = ast_bool_lit(1, yylineno); }
    | TOK_FALSE
        { $$ = ast_bool_lit(0, yylineno); }
    | TOK_IDENT
        { $$ = ast_ident($1, yylineno); free($1); }
    ;

%%

void yyerror(const char *msg)
{
    fprintf(stderr, "Syntax error (line %d): %s\n", yylineno, msg);
}
