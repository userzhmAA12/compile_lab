#include"intercode.h"
#include"tree.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

InterCode IR_head;
InterCode IR_tail;
int temp_num;
void IR_append(InterCode code)
{
    if(IR_head==NULL)
    {
        IR_head = code;
        IR_tail = code;
        code->next = NULL;
        code->pre = NULL;
    }
    else
    {
        IR_tail->next = code;
        code->pre = IR_tail;
        code->next = NULL;
        IR_tail = code;
    }
}
void new_temp()
{
    temp_num++;
    Operand ret = (Operand)malloc(sizeof(struct Operand_));
    ret->kind = VAR;
    int tmp = temp_num;
    int i = 0;
    char str[32];
    while(tmp>0)
    {
        str[i] = tmp%10 + '0';
        tmp = tmp/10;
        i++;
    }
    str[i] = '\0';
    i = i - 1;
    int j = 1;
    ret->u.name[0] = 't';
    while(i>=0)
    {
        ret->u.name[j] = str[i];
        j++;
        i--;
    }
    ret->u.name[j] = '\0';
}
void translate_Program(TreeNode* root)
{
    translate_ExtDefList(root->first_child);
}
void translate_ExtDefList(TreeNode* root)
{
    if(strcmp(root->type, "NULL")==0)return ;
    TreeNode* first = root->first_child;
    TreeNode* second = first->next_brother;
    translate_ExtDef(first);
    translate_ExtDefList(second);
}
void translate_ExtDef(TreeNode* root)
{
    TreeNode *first = root->first_child;
    TreeNode *second = first->next_brother;
    if(strcmp(second->type, "FunDec")==0)
    {
        TreeNode *third = second->next_brother;
        if(strcmp(third->type, "Compst")==0) //函数定义
        {
            translate_FunDec(second);
            translate_Compst(third);
        }
    }
}
void translate_FunDec(TreeNode* root)
{
    TreeNode* id = root->first_child;
    TreeNode* lp = id->next_brother;
    TreeNode* varlist = lp->next_brother;
    InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
    Operand tmp_op = (Operand)malloc(sizeof(struct Operand_));
    tmp_op->kind = FUNCTION;
    strcpy(tmp_op->u.name, id->val);
    code1->u.singleOp.op = tmp_op;
    code1->kind = IR_FUNCTION;
    IR_append(code1);
    if(strcmp(varlist->type, "VarList")==0)
    {
        translate_VarList(varlist);
    }
}
void translate_VarList(TreeNode* root)
{
    TreeNode* paramdec = root->first_child;
    TreeNode* second = paramdec->next_brother;
    translate_ParamDec(paramdec);
    if(second!=NULL)
    {
        translate_VarList(second->next_brother);
    }
}
void translate_ParamDec(TreeNode* root) //函数形参的具体处理
{
    TreeNode* specifier = root->first_child;
    TreeNode* vardec = specifier->next_brother;
    TreeNode* id = vardec->first_child; //函数形参的vardec一定解析成id
    InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
    Operand tmp_op = (Operand)malloc(sizeof(struct Operand_));
    tmp_op->kind = VAR;
    strcpy(tmp_op->u.name, id->val);
    code1->kind = IR_PARAM;
    code1->u.singleOp.op = tmp_op;
    IR_append(code1);
}
void translate_Compst(TreeNode* root)
{

}