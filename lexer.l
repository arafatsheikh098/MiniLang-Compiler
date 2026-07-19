%{
#include "ast.h"
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void lex_error(const char *msg);

int g_dump_tokens = 0;
static int tok_num = 0;

static void tok_log(int tok, const char *name, const char *val)
{
    if (!g_dump_tokens) return;
    printf("  %-4d  %-6d  %-20s  %s\n", ++tok_num, yylineno, name, val);
}
%}

%option noyywrap
%option yylineno

DIGIT   [0-9]
ALPHA   [a-zA-Z_]
ALNUM   [a-zA-Z0-9_]

%%

[ \t\r\n]+              { }

"//"[^\n]*              { }

"/*"                    {
                            int c;
                            while (1) {
                                c = input();
                                if (c == EOF) {
                                    lex_error("unterminated block comment");
                                    break;
                                }
                                if (c == '*') {
                                    c = input();
                                    if (c == '/') break;
                                    if (c != EOF) unput(c);
                                }
                            }
                        }

"int"                   { tok_log(TOK_INT,    "TOK_INT",    "int");   return TOK_INT;   }
"bool"                  { tok_log(TOK_BOOL,   "TOK_BOOL",   "bool");  return TOK_BOOL;  }
"true"                  { yylval.ival = 1;  tok_log(TOK_TRUE,  "TOK_TRUE",  "true");   return TOK_TRUE;  }
"false"                 { yylval.ival = 0;  tok_log(TOK_FALSE, "TOK_FALSE", "false");  return TOK_FALSE; }
"if"                    { tok_log(TOK_IF,    "TOK_IF",    "if");      return TOK_IF;    }
"else"                  { tok_log(TOK_ELSE,  "TOK_ELSE",  "else");    return TOK_ELSE;  }
"while"                 { tok_log(TOK_WHILE, "TOK_WHILE", "while");   return TOK_WHILE; }
"print"                 { tok_log(TOK_PRINT, "TOK_PRINT", "print");   return TOK_PRINT; }

{DIGIT}+                {
                            yylval.ival = atoi(yytext);
                            tok_log(TOK_INTEGER, "TOK_INTEGER", yytext);
                            return TOK_INTEGER;
                        }

{ALPHA}{ALNUM}*         {
                            yylval.sval = strdup(yytext);
                            tok_log(TOK_IDENT, "TOK_IDENT", yytext);
                            return TOK_IDENT;
                        }

"+"                     { tok_log(TOK_PLUS,   "TOK_PLUS",   "+");  return TOK_PLUS;   }
"-"                     { tok_log(TOK_MINUS,  "TOK_MINUS",  "-");  return TOK_MINUS;  }
"*"                     { tok_log(TOK_STAR,   "TOK_STAR",   "*");  return TOK_STAR;   }
"/"                     { tok_log(TOK_SLASH,  "TOK_SLASH",  "/");  return TOK_SLASH;  }
"<"                     { tok_log(TOK_LT,     "TOK_LT",     "<");  return TOK_LT;     }
">"                     { tok_log(TOK_GT,     "TOK_GT",     ">");  return TOK_GT;     }
"=="                    { tok_log(TOK_EQ,     "TOK_EQ",     "=="); return TOK_EQ;     }
"!="                    { tok_log(TOK_NEQ,    "TOK_NEQ",    "!="); return TOK_NEQ;    }
"="                     { tok_log(TOK_ASSIGN, "TOK_ASSIGN", "=");  return TOK_ASSIGN; }
"("                     { tok_log(TOK_LPAREN, "TOK_LPAREN", "(");  return TOK_LPAREN; }
")"                     { tok_log(TOK_RPAREN, "TOK_RPAREN", ")");  return TOK_RPAREN; }
"{"                     { tok_log(TOK_LBRACE, "TOK_LBRACE", "{");  return TOK_LBRACE; }
"}"                     { tok_log(TOK_RBRACE, "TOK_RBRACE", "}");  return TOK_RBRACE; }
";"                     { tok_log(TOK_SEMI,   "TOK_SEMI",   ";");  return TOK_SEMI;   }

.                       {
                            char buf[64];
                            snprintf(buf, sizeof(buf),
                                     "unexpected character '%s'", yytext);
                            lex_error(buf);
                        }

%%

void lex_error(const char *msg)
{
    fprintf(stderr, "Lexical error (line %d): %s\n", yylineno, msg);
}
