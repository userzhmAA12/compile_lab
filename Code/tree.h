#ifndef _TREE_H_
#define _TREE_H_
typedef struct TreeNode{
    char type[32];
    int lineno;
    char val[32];
    struct TreeNode* first_child;
    struct TreeNode* next_brother;
    int is_left;
}TreeNode;
TreeNode* creatNode(char* a, int b, char* c);
void printTree(TreeNode* root, int depth);


#endif
