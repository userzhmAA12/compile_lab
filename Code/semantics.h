#ifndef _SEMANTICS_H
#define _SEMANTICS_H
#include"tree.h"
#include<string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct HashNode_* HashNode;
typedef struct FuncList_* FuncList;
typedef struct Stack_* Stack;
typedef struct FuncNode_* FuncNode;
#define TABLESIZE 16384
#define BASIC_INT 0
#define BASIC_FLOAT 1

struct Type_
{
    enum
    {
        BASIC,
        ARRAY,
        STRUCTURE,
        FUNCTION
    } kind;
    union
    {
        // 基本类型
        int basic;
        // 数组类型信息包括元素类型与数组大小构成
        struct
        {
            Type elem;
            int size;
        } array;
        // 结构体类型信息是一个链表
        FieldList structure;
        // 函数信息也可以用链表来表示
        FieldList function;
    } u;
};

struct FieldList_
{
    char name[32];     // 域的名字, 函数名/参数名
    Type type;      // 域的类型, 返回值类型/参数类型
    FieldList tail; // 下一个域, 下一个参数名
};

struct HashNode_
{
    int depth; //作用域深度
    char name[32]; //名称
    Type val; //类型
    HashNode next; //哈希值相同的下一个节点
    HashNode after; //同作用域内下一个节点
    bool flag; //区分函数是否被定义
};

struct FuncNode_ //记录只声明未定义的函数
{
    HashNode func;
    int lineno;
    FuncNode next;
    FuncNode before;
};
void func_insert(HashNode tmp1, int tmp2);
void find_delete(HashNode tmp1);

unsigned int hash_pjw(char *name); //哈希函数 
const HashNode check(char* name);
void insertnode(HashNode tmp);
void deletenode(HashNode tmp);

struct Stack_{
    HashNode head; //作用域链表表头
    Stack next; //下一个作用域
};
void stack_push(Stack tmp);
void stack_pop();
bool type_cmp(Type t1, Type t2);
bool field_cmp(FieldList f1, FieldList f2);

void Program(TreeNode *root);
void ExtDefList(TreeNode *root);
void ExtDef(TreeNode *root);
void ExtDecList(TreeNode *root, Type type);
Type Specifier(TreeNode *root);
Type StructSpecifier(TreeNode *root);
void DefList_struct(TreeNode* root, FieldList head);
FieldList Def_struct(TreeNode* root);
FieldList DecList_struct(TreeNode* root, Type dectype);
FieldList Dec_struct(TreeNode* root, Type dectype);
HashNode VarDec(TreeNode* root, Type dectype);
HashNode FunDec(TreeNode* root, Type ret_type);
FieldList VarList(TreeNode* root);
FieldList ParamDec(TreeNode* root);
void Compst(TreeNode* root, Type ret_type);
void StmtList(TreeNode* root, Type ret_type);
void Stmt(TreeNode* root, Type ret_type);
void DefList_func(TreeNode* root);
void Def_func(TreeNode* root);
void DecList_func(TreeNode* root, Type dectype);
void Dec_func(TreeNode* root, Type dectype);
Type Exp(TreeNode* root);
Type Exp_Assign(TreeNode* root);
Type Exp_And(TreeNode* root);
Type Exp_Math(TreeNode* root);
Type Exp_ID(TreeNode* root);
Type Exp_Func(TreeNode* root);
int Args(TreeNode* root, FieldList func);
Type Exp_Array(TreeNode* root);
Type Exp_Dot(TreeNode* root);
#endif