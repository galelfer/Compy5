%{
	#include "parser.h"
	#include "parser.tab.hpp"
%}

%option yylineno
%option noyywrap

%%


void														{yylval= new Node(yytext, "void", ""); return VOID;}
int														    {yylval= new Node(yytext, "int", "");return INT;}
byte														{yylval= new Node(yytext, "byte", "");return BYTE;}
b														    return B;
bool														{yylval= new Node(yytext, "bool", "");return BOOL;}
and														    return AND;
or														    return OR;
not														    return NOT;
true														{yylval = new Node(yytext, "bool", "true"); return TRUE;}
false														{yylval = new Node(yytext, "bool", "false"); return FALSE;}
return														return RETURN;
if														    return IF;
else 														return ELSE;
while														return WHILE;
break														return BREAK;
continue 													return CONTINUE;
;														    return SC;
, 														    return COMMA;
"(" 														return LPAREN;
")" 														return RPAREN;
"{" 														return LBRACE;
"}"														    return RBRACE;
= 														    return ASSIGN;
(==)|(!=)                                                   {yylval = new Node(yytext, "prerelop"); return PRERELOP;}
(<)|(>)|(<=)|(>=) 								            {yylval = new Node(yytext, "postrelop"); return POSTRELOP;}
[+]|[-]														{yylval = new Node(yytext, "prebinop"); return PREBINOP;}
[*]|[/]														{yylval = new Node(yytext, "postbinop"); return POSTBINOP;}
[a-zA-Z]([a-zA-Z0-9])* 										{yylval = new Node(yytext, yytext); return ID;}
0|[1-9]([0-9])* 											{yylval = new Node(yytext, "int"); return NUM;}
(\")([^\n\r\"\\]|\\[rnt"\\])+(\")							{yylval = new Node(yytext, "string"); return STRING;}
\/\/[^\r\n]*[\r|\n|\r\n]?									;
([\t\n\r ])   												;

.														    {output::errorLex(yylineno); exit(0);}

%%
