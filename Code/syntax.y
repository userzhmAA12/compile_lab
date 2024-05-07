%{
    #include<stdio.h>
    #include "lex.yy.c"
    #include "tree.h"
    #include "semantics.h"
    #include "intercode.h"
    
    int yyerror(char* s);
    extern int has_error;
    TreeNode* ROOT;
    extern HashNode SymbolTable[TABLESIZE];
    extern Stack Top;
    extern FuncNode func_head;
    extern int error_num;
    extern int dep_now;
%}
%union {
    TreeNode* type_node;
}

%token <type_node> INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE


%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%type <type_node> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args

%%
Program : ExtDefList {
    $$ = creatNode("Program", $1->lineno, "");
    $$->first_child = $1;
    ROOT = $$;
}
    ;
ExtDefList : ExtDef ExtDefList {
    $$ = creatNode("ExtDefList", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
}
    | /* empty */ {$$=creatNode("NULL", -1, "");}
    ;
ExtDef : Specifier ExtDecList SEMI {
    $$ = creatNode("ExtDef", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Specifier SEMI {
    $$ = creatNode("ExtDef", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
}
    | Specifier FunDec CompSt {
    $$ = creatNode("ExtDef", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Specifier FunDec SEMI{
    $$ = creatNode("ExtDef", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Specifier ExtDecList {
    printf("Error type B at Line %d: Syntax error 1 Missing \'%s\'\n", @2.last_line, ";");
    has_error = 1;
}
    | Specifier {
    printf("Error type B at Line %d: Syntax error 2 Missing \'%s\'\n", @1.last_line, ";");
    has_error = 1;
}
    | error SEMI {
    printf("Error type B at Line %d: Syntax error 3 found.\n", @1.last_line);
    has_error = 1;
}
    | Specifier error CompSt {
    printf("Error type B at Line %d: Syntax error 4 found.\n", @2.last_line);
    has_error = 1;
}
    ;
ExtDecList : VarDec {
    $$ = creatNode("ExtDecList", $1->lineno, "");
    $$->first_child = $1;
}
    | VarDec COMMA ExtDecList {
    $$ = creatNode("ExtDecList", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    ;
Specifier : TYPE {
    $$ = creatNode("Specifier", $1->lineno, "");
    $$->first_child = $1;
}
    | StructSpecifier {
    $$ = creatNode("Specifier", $1->lineno, "");
    $$->first_child = $1;
}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {
    $$ = creatNode("StructSpecifier", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
    $3->next_brother = $4;
    $4->next_brother = $5;
}
    | STRUCT Tag {
    $$ = creatNode("StructSpecifier", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
}
    | STRUCT error RC {
    printf("Error type B at Line %d: Syntax error 5 found.\n", @2.last_line);
    has_error = 1;
}
    ;
OptTag : ID {
    $$ = creatNode("OptTag", $1->lineno, "");
    $$->first_child = $1;
}
    | /* empty */ {$$=creatNode("NULL", -1, "");}
    ;
Tag : ID {
    $$ = creatNode("Tag", $1->lineno, "");
    $$->first_child = $1;
}
    ;
VarDec : ID {
    $$ = creatNode("VarDec", $1->lineno, "");
    $$->first_child = $1;
}
    | VarDec LB INT RB {
    $$ = creatNode("VarDec", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
    $3->next_brother = $4;
}
    | VarDec LB error RB {
    printf("Error type B at Line %d: Syntax error 6 found.\n", @3.last_line);
    has_error = 1;
}
    ;
FunDec : ID LP VarList RP {
    $$ = creatNode("FunDec", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
    $3->next_brother = $4;
}
    | ID LP RP {
    $$ = creatNode("FunDec", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | ID LP error RP {
    printf("Error type B at Line %d: Syntax error 7 found.\n", @3.last_line);
    has_error = 1;
}
    ;
VarList : ParamDec COMMA VarList {
    $$ = creatNode("VarList", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | ParamDec {
    $$ = creatNode("VarList", $1->lineno, "");
    $$->first_child = $1;
}
    ;
ParamDec : Specifier VarDec {
    $$ = creatNode("ParamDec", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
}
    ;
CompSt : LC DefList StmtList RC {
    $$ = creatNode("CompSt", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
    $3->next_brother = $4;
}
    | LC DefList StmtList {
    printf("Error type B at Line %d: Syntax error 8 Missing \'%s\'\n", @3.last_line, "}");
    has_error = 1;
}
    | error RC {
    printf("Error type B at Line %d: Syntax error 9 found.\n", @1.last_line);
    has_error = 1;
}
    ;
StmtList : Stmt StmtList {
    $$ = creatNode("StmtList", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
}
    | /* empty */ {$$=creatNode("NULL", -1, "");}
    ;
Stmt : Exp SEMI {
    $$ = creatNode("Stmt", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
}
    | CompSt {
    $$ = creatNode("Stmt", $1->lineno, "");
    $$->first_child = $1;
}
    | RETURN Exp SEMI {
    $$ = creatNode("Stmt", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3; 
}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
    $$ = creatNode("Stmt", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
    $3->next_brother = $4;
    $4->next_brother = $5;
}
    | IF LP Exp RP Stmt ELSE Stmt {
    $$ = creatNode("Stmt", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
    $3->next_brother = $4;
    $4->next_brother = $5;
    $5->next_brother = $6;
    $6->next_brother = $7;
}
    | WHILE LP Exp RP Stmt {
    $$ = creatNode("Stmt", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
    $3->next_brother = $4;
    $4->next_brother = $5;
}
    | IF error ELSE {
    printf("Error type B at Line %d: Syntax error 10 found.\n", @2.last_line);
    has_error = 1;
}
    | WHILE error ELSE Stmt {
    printf("Error type B at Line %d: Syntax error 11 found.\n", @2.last_line);
    has_error = 1;
}
    | WHILE error RP {
    printf("Error type B at Line %d: Syntax error 12 found.\n", @2.last_line);
    has_error = 1;
}
    | error SEMI {
    printf("Error type B at Line %d: Syntax error 13 found.\n", @1.last_line);
    has_error = 1;
}
    | Exp {
    printf("Error type B at Line %d: Syntax error 14 Missing \'%s\'\n", @1.last_line, ";");
    has_error = 1;
}
    | RETURN Exp {
    printf("Error type B at Line %d: Syntax error 15 Missing \'%s\'\n", @2.last_line, ";");
    has_error = 1;
}
    | STAR DIV {
    printf("Error type B at Line %d: Syntax error 20 found.\n", @1.last_line);
}
    ;
DefList : Def DefList {
    $$ = creatNode("DefList", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
}
    | /* empty */ {$$=creatNode("NULL", -1, "");}
    ;
Def : Specifier DecList SEMI {
    $$ = creatNode("Def", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Specifier error SEMI {
    printf("Error type B at Line %d: Syntax error 16 found.\n", @2.last_line);
    has_error = 1;
}
    | Specifier DecList {
    printf("Error type B at Line %d: Syntax error 17 Missing \'%s\'\n", @2.last_line, ";");
    has_error = 1;
}
    | STAR DIV {
    printf("Error type B at Line %d: Syntax error 21 found.\n", @1.last_line);
    has_error = 1;
}
    ;
DecList : Dec {
    $$ = creatNode("DecList", $1->lineno, "");
    $$->first_child = $1;
}
    | Dec COMMA DecList {
    $$ = creatNode("DecList", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    ;
Dec : VarDec {
    $$ = creatNode("Dec", $1->lineno, "");
    $$->first_child = $1;
}
    | VarDec ASSIGNOP Exp {
    $$ = creatNode("Dec", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    ;
Exp : Exp ASSIGNOP Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Exp AND Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Exp OR Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Exp RELOP Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Exp PLUS Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Exp MINUS Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Exp STAR Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Exp DIV Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | LP Exp RP {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | MINUS Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
}
    | NOT Exp {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
}
    | ID LP Args RP {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
    $3->next_brother = $4;
}
    | ID LP RP {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Exp LB Exp RB {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
    $3->next_brother = $4;
}
    | Exp DOT ID {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | ID {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
}
    | INT {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
}
    | FLOAT {
    $$ = creatNode("Exp", $1->lineno, "");
    $$->first_child = $1;
}
    | error RP {
    printf("Error type B at Line %d: Syntax error 18 found.\n", @1.last_line);
    has_error = 1;
}
    | Exp LB error RB {
    printf("Error type B at Line %d: Syntax error 19 found.\n", @3.last_line);
    has_error = 1;
}
    ;
Args : Exp COMMA Args {
    $$ = creatNode("Args", $1->lineno, "");
    $$->first_child = $1;
    $1->next_brother = $2;
    $2->next_brother = $3;
}
    | Exp {
    $$ = creatNode("Args", $1->lineno, "");
    $$->first_child = $1;
}
    ;
%%
int main(int argc, char** argv) 
{ 
    if (argc <= 1) return 1; 
    FILE* f = fopen(argv[1], "r"); 
    if (!f) 
    { 
    perror(argv[1]); 
    return 1; 
    } 
    yyrestart(f); 
    yyparse(); 
    // if(has_error == 0)printTree(ROOT, 0);
    Program(ROOT);
    while(func_head!=NULL)
    {
        error_num++;
        printf("Error type 18 at Line %d: Undefined function \"%s\".\n", func_head->lineno, func_head->func->name);
        func_head = func_head->next;
    }
    return 0; 
}
int yyerror(char* s){
    return 0;
}