/*#include<stdio.h>
#include"syntax.tab.h"
#include"tree.h"
extern int has_error;
extern TreeNode* ROOT;
extern void yyrestart  (FILE * input_file );
extern int yyparse (void);
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
    if(has_error == 0)printTree(ROOT, 0);
    return 0; 
}*/