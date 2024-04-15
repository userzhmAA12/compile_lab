%{
    #include<stdio.h>
    #include "tree.h"
    #include "syntax.tab.h"
    
    #define YY_USER_ACTION \
        yylloc.first_line=yylineno;\
        yylloc.last_line=yylineno;
    int has_error = 0;
%}
%option yylineno
%%
([1-9][0-9]*)|0 {yylval.type_node = creatNode("INT", yylineno, yytext); return INT;}
0[0-7]* {yylval.type_node = creatNode("INT", yylineno, yytext); return INT;}
(0x|0X)[0-9a-fA-F]* {yylval.type_node = creatNode("INT", yylineno, yytext); return INT;}
[0-9]+\.[0-9]+ {yylval.type_node = creatNode("FLOAT", yylineno, yytext); return FLOAT;}
([0-9]+\.|\.[0-9]+|[0-9]+\.[0-9]+)(e|E)(\+|\-)?[0-9]+ {yylval.type_node = creatNode("FLOAT", yylineno, yytext); return FLOAT;}
"int"|"float" {yylval.type_node = creatNode("TYPE", yylineno, yytext); return TYPE;}
struct {yylval.type_node = creatNode("STRUCT", yylineno, yytext); return STRUCT;}
return {yylval.type_node = creatNode("RETURN", yylineno, yytext); return RETURN;}
if {yylval.type_node = creatNode("IF", yylineno, yytext); return IF;}
else {yylval.type_node = creatNode("ELSE", yylineno, yytext); return ELSE;}
while {yylval.type_node = creatNode("WHILE", yylineno, yytext); return WHILE;}
[_a-zA-Z][_a-zA-Z0-9]* {yylval.type_node = creatNode("ID", yylineno, yytext); return ID;}
";" {yylval.type_node = creatNode("SEMI", yylineno, yytext); return SEMI;}
"," {yylval.type_node = creatNode("COMMA", yylineno, yytext); return COMMA;}
>|<|>=|<=|==|!= {yylval.type_node = creatNode("RELOP", yylineno, yytext); return RELOP;}
"=" {yylval.type_node = creatNode("ASSIGNOP", yylineno, yytext); return ASSIGNOP;}
"+" {yylval.type_node = creatNode("PLUS", yylineno, yytext); return PLUS;}
"-" {yylval.type_node = creatNode("MINUS", yylineno, yytext); return MINUS;}
"*" {yylval.type_node = creatNode("STAR", yylineno, yytext); return STAR;}
"/" {yylval.type_node = creatNode("DIV", yylineno, yytext); return DIV;}
"&&" {yylval.type_node = creatNode("AND", yylineno, yytext); return AND;}
"||" {yylval.type_node = creatNode("OR", yylineno, yytext); return OR;}
"." {yylval.type_node = creatNode("DOT", yylineno, yytext); return DOT;}
"!" {yylval.type_node = creatNode("NOT", yylineno, yytext); return NOT;}
"(" {yylval.type_node = creatNode("LP", yylineno, yytext); return LP;}
")" {yylval.type_node = creatNode("RP", yylineno, yytext); return RP;}
"[" {yylval.type_node = creatNode("LB", yylineno, yytext); return LB;}
"]" {yylval.type_node = creatNode("RB", yylineno, yytext); return RB;}
"{" {yylval.type_node = creatNode("LC", yylineno, yytext); return LC;}
"}" {yylval.type_node = creatNode("RC", yylineno, yytext); return RC;}
"//".* ;
"/*" {
    int c = input();
    while(1)
    {
        if(c=='*')
        {
            c = input();
            if(c=='/')
            {
                break;
            }
        }
        c = input();
    }
}
[ \r\t\n]+ ;
. {printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); has_error = 1;}
%%