#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

/* int s2i(char* str)
{
    if(str[0]=='0')
    {
        if(str[1]=='x'||str[1]=='X') // 16
        {

        }
        else // 8
        {

        }
    }
    else // 10
    {

    }
} */
TreeNode* creatNode(char* a, int b, char* c)
{
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    if (newNode != NULL) {
        strcpy(newNode->type, a);
        newNode->lineno = b;
        strcpy(newNode->val, c);
        // printf("%s: %s\n", a, c);
        // printf("%s: %s\n", newNode->type, newNode->val);
        newNode->first_child = NULL;
        newNode->next_brother = NULL;
        newNode->is_left = 0;
    }
    return newNode;
}
void printTree(TreeNode* root, int depth)
{
    if(root!=NULL)
    {
        if(root->lineno!=-1)
        {
            // printf("%s: %s\n", root->type, root->val);
            for(int i=0;i<depth;i++)
            {
                printf("  ");
            }
            printf("%s", root->type);
            if(strlen(root->val)==0)printf(" (%d)",root->lineno);
            else 
            {
                
                if(strcmp(root->type, "INT")==0)
                {
                    printf(": ");
                    printf("%ld", strtol(root->val, NULL, 0));
                }
                else if(strcmp(root->type, "FLOAT")==0)
                {
                    printf(": ");
                    printf("%f", atof(root->val));
                }
                else if(strcmp(root->type, "ID")==0)
                {
                    printf(": ");
                    printf("%s", root->val);
                }
                else if(strcmp(root->type, "TYPE")==0)
                {
                    printf(": ");
                    printf("%s", root->val);
                }
            }
            printf("\n");
            printTree(root->first_child, depth+1);
        }
        printTree(root->next_brother, depth);
    }
}