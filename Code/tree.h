typedef struct TreeNode{
    char type[32];
    int lineno;
    char val[32];
    struct TreeNode* first_child;
    struct TreeNode* next_brother;
}TreeNode;
TreeNode* creatNode(char* a, int b, char* c);
void printTree(TreeNode* root, int depth);
