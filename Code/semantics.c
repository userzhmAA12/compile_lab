#include "semantics.h"
#include <stdio.h>
HashNode SymbolTable[TABLESIZE];
Stack Top;
FuncNode func_head;
int error_num;
int dep_now;

// 辅助函数
void func_insert(HashNode tmp1, int tmp2)
{
    FuncNode ret = (FuncNode)malloc(sizeof(struct FuncNode_));
    ret->func = tmp1;
    ret->lineno = tmp2;
    ret->next = func_head;
    if(func_head!=NULL)func_head->before = ret;
    ret->before = NULL;
    func_head = ret;
}
void find_delete(HashNode tmp1)
{
    FuncNode ret = func_head;
    while(ret!=NULL)
    {
        if(ret->func==tmp1)
        {
            if(ret->before!=NULL)
            {
                ret->before->next = ret->next;
            }
            if(ret->next!=NULL)
            {
                ret->next->before = ret->before;
            }
            if(func_head==ret)
            {
                func_head = ret->next;
            }
            free(ret);
            break;
        }
    }
}

unsigned int hash_pjw(char *name) //哈希函数 
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~0x3fff)
            val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}
const HashNode check(char* name)
{
    unsigned int hash_val = hash_pjw(name);
    HashNode tmp=SymbolTable[hash_val];
    while(tmp!=NULL)
    {
        if(strcmp(tmp->name,name)==0)
        {
            return tmp;
        }
        tmp=tmp->next;
    }
    return tmp;
}
void insertnode(HashNode tmp)
{
    unsigned int hash_val = hash_pjw(tmp->name);
    tmp->next = SymbolTable[hash_val];
    SymbolTable[hash_val] = tmp;
}
void deletenode(HashNode tmp)
{
    unsigned int hash_val = hash_pjw(tmp->name);
    assert(strcmp(SymbolTable[hash_val]->name, tmp->name)==0);
    SymbolTable[hash_val] = tmp->next;
    free(tmp);
}
void stack_push(Stack tmp)
{
    tmp->next = Top;
    Top = tmp;
}
void stack_pop()
{
    Stack tmp1 = Top;
    Top = tmp1->next;
    HashNode tmp2 = tmp1->head;
    while(tmp2!=NULL)
    {
        HashNode tmp3 = tmp2->after;
        deletenode(tmp2);
        tmp2 = tmp3;
    }
    free(tmp1);
}
bool type_cmp(Type t1, Type t2)
{
    if(t1->kind!=t2->kind)
    {
        return false;
    }
    else
    {
        switch (t1->kind)
        {
        case BASIC:
            if(t1->u.basic == t2->u.basic)
                return true;
            else
                return false;
            break;
        case ARRAY:
            if(t1->u.array.size!=t2->u.array.size)
            {
                return false;
            }
            else
            {
                return type_cmp(t1->u.array.elem, t2->u.array.elem);
            }
            break;
        case STRUCTURE:
            return field_cmp(t1->u.structure, t2->u.structure);
            break;
        case FUNCTION:
            return field_cmp(t1->u.function, t2->u.function); 
            break;
        default:
            break;
        }
    }
}
bool field_cmp(FieldList f1, FieldList f2)
{
    
    if(f1==NULL&&f2==NULL)
    {
        return true;
    }
    else if(f1!=NULL&&f2!=NULL)
    {
        if(type_cmp(f1->type, f2->type)==false)
            return false;
        else
            field_cmp(f1->tail, f2->tail);
    }
    else
        return false;
}
int calculate_size(Type t)
{
    if(t->kind==BASIC)
    {
        return 4;
    }
    else if(t->kind==ARRAY)
    {
        return t->u.array.size*calculate_size(t->u.array.elem);
    }
    else if(t->kind==STRUCTURE)
    {
        int size=0;
        FieldList list=t->u.structure;
        while(list!=NULL)
        {
            size+=calculate_size(list->type);
            list=list->tail;
        }
        return size;
    }
}
void init_table()
{
    HashNode func_read = (HashNode)malloc(sizeof(struct HashNode_));
    strcpy(func_read->name, "read");
    FieldList func = (FieldList)malloc(sizeof(struct FieldList_));
    strcpy(func->name, "read");
    Type temp_type1 = (Type)malloc(sizeof(struct Type_));
    Type ret_type = (Type)malloc(sizeof(struct Type_));
    ret_type->kind = BASIC;
    ret_type->u.basic = BASIC_INT;
    func->type = ret_type;
    func->tail = NULL;
    temp_type1->kind = FUNCTION;
    temp_type1->u.function = func;
    func_read->val = temp_type1;
    func_read->depth = dep_now;
    func_read->flag = true;
    insertnode(func_read);

    HashNode func_write = (HashNode)malloc(sizeof(struct HashNode_));
    strcpy(func_write->name, "write");
    FieldList func2 = (FieldList)malloc(sizeof(struct FieldList_));
    strcpy(func2->name, "write");
    Type temp_type2 = (Type)malloc(sizeof(struct Type_));
    Type ret_type2 = (Type)malloc(sizeof(struct Type_));
    ret_type2->kind = BASIC;
    ret_type2->u.basic = BASIC_INT;
    func2->type = ret_type2;
    FieldList param = (FieldList)malloc(sizeof(struct FieldList_));
    strcpy(param->name, "x");
    Type param_type = (Type)malloc(sizeof(struct Type_));
    param_type->kind = BASIC;
    param_type->u.basic = BASIC_INT;
    param->type = param_type;
    param->tail = NULL;
    func2->tail = param;
    temp_type2->kind = FUNCTION;
    temp_type2->u.function = func2;
    func_write->val = temp_type2;
    func_write->depth = dep_now;
    func_write->flag = true;
    insertnode(func_write);
}

// high-level definitions
void Program(TreeNode *root)
{
    // printf("Program\n");
    dep_now = 0;
    error_num = 0;
    Top = NULL;
    init_table();
    TreeNode* first = root->first_child;
    ExtDefList(first);
}
void ExtDefList(TreeNode *root)
{
    // printf("ExtDefList\n");
    if(strcmp(root->type, "NULL")==0)return ;
    TreeNode* first = root->first_child;
    TreeNode* second = first->next_brother;
    ExtDef(first);
    ExtDefList(second);
}
void ExtDef(TreeNode *root)
{
    // printf("ExtDef\n");
    TreeNode *first = root->first_child;
    TreeNode *second = first->next_brother;
    Type type = Specifier(first);
    if(strcmp(second->type, "ExtDecList")==0) //变量定义
    {
        ExtDecList(second, type);
    }
    else if(strcmp(second->type, "SEMI")==0)
    {
        return ;
    }
    else if(strcmp(second->type, "FunDec")==0) //函数定义或声明
    {
        TreeNode *third = second->next_brother;
        dep_now++;
        Stack new_top = (Stack)malloc(sizeof(struct Stack_));
        new_top->head = NULL;
        stack_push(new_top);
        if(strcmp(third->type, "SEMI")==0) //函数声明
        {
            HashNode tmp = FunDec(second, type);
            stack_pop();
            dep_now--;
            HashNode find_node = check(tmp->name);
            if(find_node==NULL) //之前没有过声明或定义
            {
                tmp->flag = false;
                insertnode(tmp);
                func_insert(tmp, root->lineno);
            }
            else //之前有过声明或定义
            {
                if(type_cmp(tmp->val, find_node->val)==false) //与之前定义或声明不一致
                {
                    printf("Error type 19 at Line %d: Declaration conflict \"%s\".\n",root->lineno, tmp->name);
                    free(tmp);
                }
            }
        }
        else //函数定义
        {
            HashNode tmp = FunDec(second, type);
            HashNode find_node = check(tmp->name);
            if(find_node==NULL) //之前没有过声明或定义
            {
                tmp->flag = true;
                insertnode(tmp);
            }
            else //之前有过声明或定义
            {
                if(find_node->flag == true) //之前有过定义
                {
                    printf("Error type 4 at Line %d: Redefined function \"%s\".\n",root->lineno, tmp->name);
                    free(tmp);
                }
                else //之前有过声明
                {
                    if(type_cmp(tmp->val, find_node->val)==false) //与之前声明不一致
                    {
                        printf("Error type 19 at Line %d: Declaration conflict \"%s\".\n",root->lineno, tmp->name);
                        free(tmp);
                    }
                    else //与之前声明一致
                    {
                        find_node->flag = true;
                        free(tmp);
                        find_delete(find_node);
                    }
                }
            }
            Compst(third, type);
            stack_pop();
            dep_now--;
        }
    }
}
void ExtDecList(TreeNode *root, Type type)
{
    // printf("ExtDecList\n");
    TreeNode *first = root->first_child;
    HashNode tmp = VarDec(first, type);
    HashNode find_node = check(tmp->name);
    if(find_node!=NULL) //全局变量重定义，不用考虑作用域层数
    {
        error_num++;
        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",first->lineno, tmp->name);
        free(tmp);
    }
    else //定义成功，加入符号表
    {
        insertnode(tmp);
    }
    TreeNode *second = first->next_brother;
    if(second!=NULL)
    {
        TreeNode *third = second->next_brother;
        ExtDecList(third, type);
    }
}
// Specifiers
Type Specifier(TreeNode *root)
{
    // printf("Specifier\n");
    TreeNode *first = root->first_child;
    if(strcmp(first->type, "TYPE")==0)
    {
        Type type=(Type)malloc(sizeof(struct Type_));
        type->kind=BASIC;
        if(strcmp(first->val,"int")==0){
            type->u.basic = BASIC_INT;
        }
        else if(strcmp(first->val,"float")==0)
            type->u.basic = BASIC_FLOAT;
        return type;
    }
    else
    {
        return StructSpecifier(first);
    }
}
Type StructSpecifier(TreeNode *root)
{
    // printf("StructSpecifier\n");
    TreeNode *first = root->first_child;
    TreeNode *second = first->next_brother;
    Type ret = NULL;
    if(strcmp(second->type, "Tag")==0) // 结构体变量定义
    {
        const HashNode find_node = check(second->first_child->val);
        if(find_node==NULL || find_node->val->kind != STRUCTURE) // 结构体未定义
        {
            error_num++;
            printf("Error Type 17 at Line %d: Undefined structure \"%s\".\n",second->lineno, second->first_child->val);
        }
        else 
        {
            ret = find_node->val;
        }
    }
    else // 结构体定义
    {
        if(strcmp(second->type, "NULL") != 0) //非匿名结构体
        {
            const HashNode find_node = check(second->first_child->val);
            if(find_node!=NULL)
            {
                error_num++;
                printf("Error Type 16 at Line %d: Refined structure \"%s\".\n", second->lineno, second->first_child->val);
            }
            else
            {
                TreeNode *lc = second->next_brother;
                TreeNode *deflist = lc->next_brother;
                TreeNode *rc = deflist->next_brother;
                HashNode node=(HashNode)malloc(sizeof(struct HashNode_));
                strcpy(node->name, second->first_child->val);
                Type type=(Type)malloc(sizeof(struct Type_));
                type->kind=STRUCTURE;
                FieldList temp=(FieldList)malloc(sizeof(struct FieldList_));
                temp->tail=NULL;
                DefList_struct(deflist,temp);
                type->u.structure=temp->tail;
                free(temp);
                node->val=type;
                node->next=NULL;
                node->flag = false;
                insertnode(node);
            }
        }
        else //匿名结构体，仅检查域重名
        {
            TreeNode *lc = second->next_brother;
            TreeNode *deflist = lc->next_brother;
            TreeNode *rc = deflist->next_brother;
            HashNode node = (HashNode)malloc(sizeof(struct HashNode_));
            strcpy(node->name, second->first_child->val);
            Type type = (Type)malloc(sizeof(struct Type_));
            type->kind = STRUCTURE;
            FieldList temp = (FieldList)malloc(sizeof(struct FieldList_));
            temp->tail = NULL;
            DefList_struct(deflist, temp);
            type->u.structure = temp->tail;
            free(temp);
            node->val = type;
            node->next = NULL;
            node->flag = false;
            insertnode(node);
        }
    }
}
void DefList_struct(TreeNode* root, FieldList head)
{
    // printf("DefList_struct\n");
    if(strcmp(root->type, "NULL")!=0)
    {
        TreeNode* first = root->first_child;
        TreeNode* second = first->next_brother;
        FieldList new_def = Def_struct(first);
        FieldList tmp = new_def;
        while(tmp!=NULL)
        {
            FieldList cur = head->tail;
            while(cur!=NULL)
            {
                if(strcmp(cur->name, tmp->name)==0)
                {
                    error_num++;
                    printf("Error Type 15 at Line %d: Refined field \"%s\".\n",first->lineno,tmp->name);
                    break;
                }
                cur = cur->tail;
            }
            tmp = tmp->tail;
        }
        FieldList end = head;
        while(end->tail!=NULL)
        {
            end = end->tail;
        }
        end->tail = new_def;
        DefList_struct(second, head);
        return ;
    }
    return ;
}
FieldList Def_struct(TreeNode* root)
{
    // printf("Def_struct\n");
    TreeNode* first = root->first_child;
    TreeNode* second = first->next_brother;
    Type type = Specifier(first);
    FieldList ret = DecList_struct(second, type);
    return ret;
}
FieldList DecList_struct(TreeNode* root, Type dectype)
{
    // printf("DecList_struct\n");
    TreeNode* first = root->first_child;
    TreeNode* second = first->next_brother;
    if(second==NULL)
    {
        return Dec_struct(first, dectype);
    }
    else
    {
        TreeNode* third = second->next_brother;
        FieldList ret = Dec_struct(first, dectype);
        ret->tail = DecList_struct(third, dectype);
        return ret;
    }
}
FieldList Dec_struct(TreeNode* root, Type dectype)
{
    // printf("Dec_struct\n");
    FieldList ret = (FieldList)malloc(sizeof(struct FieldList_));
    TreeNode* vardec = root->first_child; 
    TreeNode* assignop = vardec->next_brother;
    HashNode tmp = VarDec(vardec, dectype);
    ret->type = tmp->val;
    strcpy(ret->name, tmp->name);
    free(tmp);
    if(assignop!=NULL) //结构体变量初始化
    {
        error_num++;
        printf("Error Type 15 at Line %d: Refined field \"%s\".\n",vardec->lineno, ret->name);
    }
    return ret;
}
// Declarators
HashNode VarDec(TreeNode* root, Type dectype)
{
    // printf("VarDec\n");
    TreeNode* first = root->first_child;
    if(strcmp(first->type, "VarDec")==0) //数组
    {
        TreeNode* lb = first->next_brother;
        TreeNode* degree = lb->next_brother;
        TreeNode* rb = degree->next_brother;
        Type tmp = (Type)malloc(sizeof(struct Type_));
        tmp->kind = ARRAY;
        tmp->u.array.size = strtol(degree->val, NULL, 0);
        HashNode ret = VarDec(first, dectype);
        tmp->u.array.elem = ret->val;
        ret->val = tmp;
        return ret;
    }
    else //基本变量
    {
        HashNode ret = (HashNode)malloc(sizeof(struct HashNode_));
        strcpy(ret->name, first->val);
        ret->val = dectype;
        ret->next = NULL;
        ret->after = NULL;
        ret->depth = dep_now;
        ret->flag = false;
        return ret;
    }
}
HashNode FunDec(TreeNode* root, Type ret_type)
{
    // printf("FunDec\n");
    TreeNode* id = root->first_child;
    TreeNode* lp = id->next_brother;
    TreeNode* third = lp->next_brother;
    HashNode ret = (HashNode)malloc(sizeof(struct HashNode_));
    strcpy(ret->name, id->val);
    FieldList func = (FieldList)malloc(sizeof(struct FieldList_));
    strcpy(func->name, id->val);
    func->type = ret_type;
    // IR生成
    InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
    Operand tmp_op = (Operand)malloc(sizeof(struct Operand_));
    tmp_op->kind = FUNC;
    strcpy(tmp_op->u.name, id->val);
    code1->u.singleOp.op = tmp_op;
    code1->kind = IR_FUNCTION;
    IR_append(code1);

    if(strcmp(third->type, "VarList")==0)
    {
        func->tail = VarList(third);
    }
    else
    {
        func->tail = NULL;
    }
    Type tmp = (Type)malloc(sizeof(struct Type_));
    tmp->kind = FUNCTION;
    tmp->u.function = func;
    ret->val = tmp;
    ret->next = NULL;
    ret->flag = false;
    return ret;
}
FieldList VarList(TreeNode* root)
{
    // printf("VarList\n");
    TreeNode* first = root->first_child;
    TreeNode* second = first->next_brother;
    if(second==NULL)
    {
        return ParamDec(first);
    }
    else
    {
        TreeNode* third = second->next_brother;
        FieldList ret = ParamDec(first);
        ret->tail = VarList(third);
        return ret;
    }
}
FieldList ParamDec(TreeNode* root) //既要判断是否重名，又要负责加入符号表和返回FieldList
{
    // printf("ParamDec\n");
    TreeNode* first = root->first_child;
    TreeNode* second = first->next_brother;
    Type type = Specifier(first);
    HashNode tmp = VarDec(second, type);
    FieldList ret = (FieldList)malloc(sizeof(struct FieldList_));
    strcpy(ret->name, tmp->name);
    ret->type = tmp->val;
    ret->tail = NULL;
    HashNode find_node = check(tmp->name);
    if(find_node!=NULL && (find_node->val->kind == STRUCTURE || find_node->depth == tmp->depth)) //变量重名，不加入符号表
    {
        error_num++;
        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",first->lineno, tmp->name);
        free(tmp);
    }
    else //加入符号表
    {
        tmp->after = Top->head;
        Top->head = tmp;
        tmp->flag = true;
        insertnode(tmp);

        //IR生成
        InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
        Operand tmp_op = (Operand)malloc(sizeof(struct Operand_));
        tmp_op->kind = VAR;
        strcpy(tmp_op->u.name, tmp->name);
        code1->kind = IR_PARAM;
        code1->u.singleOp.op = tmp_op;
        IR_append(code1);
    }
    return ret;
}
// Statements
void Compst(TreeNode* root, Type ret_type)
{
    // printf("Compst\n");
    TreeNode* lc = root->first_child;
    TreeNode* deflist = lc->next_brother;
    TreeNode* stmtlist = deflist->next_brother;
    DefList_func(deflist);
    StmtList(stmtlist, ret_type);
}
void StmtList(TreeNode* root, Type ret_type)
{
    // printf("StmtList\n");
    if(strcmp(root->type, "NULL")!=0)
    {
        TreeNode* stmt = root->first_child;
        TreeNode* stmtlist = stmt->next_brother;
        Stmt(stmt, ret_type);
        StmtList(stmtlist, ret_type);
    }
}
void Stmt(TreeNode* root, Type ret_type)
{
    // printf("Stmt\n");
    TreeNode* first = root->first_child;
    if(strcmp(first->type, "Exp")==0)
    {
        Exp(first);
        // IR
        Operand t1 = new_temp();
        translate_Exp(first, t1);
    }
    else if(strcmp(first->type, "CompSt")==0)
    {
        dep_now++;
        Stack new_top = (Stack)malloc(sizeof(struct Stack_));
        new_top->head = NULL;
        stack_push(new_top);
        Compst(first, ret_type);
        stack_pop();
        dep_now--;
    }
    else if(strcmp(first->type, "RETURN")==0)
    {
        TreeNode* exp = first->next_brother;
        Type exp_type = Exp(exp);
        if(exp_type==NULL)
            return ;
        if(type_cmp(exp_type, ret_type)==false)
        {
            error_num++;
            printf("Error type 8 at line %d: Type mismatched for return.\n", first->lineno);
        }
        // IR
        Operand t1 =new_temp();
        translate_Exp(exp, t1);
        InterCode code2 = (InterCode)malloc(sizeof(struct InterCode_));
        code2->kind = IR_RETURN;
        code2->u.singleOp.op = t1;
        IR_append(code2);
    }
    else if(strcmp(first->type, "IF")==0)
    {
        TreeNode* lp = first->next_brother;
        TreeNode* exp = lp->next_brother;
        TreeNode* rp = exp->next_brother;
        TreeNode* stmt = rp->next_brother;
        if(stmt->next_brother!=NULL)
        {
            Type exp_type = Exp(exp);
            if (exp_type == NULL)
                return;
            if (!(exp_type->kind == BASIC && exp_type->u.basic == BASIC_INT))
            {
                // error_num++;
                // printf("Error type 20 at line %d: the expr's type in if is not an integer.\n",root->lineno);
            }
            //IR
            Operand label1 = new_lable();
            Operand label2 = new_lable();
            Operand label3 = new_lable();
            translate_Cond(exp, label1, label2);
            InterCode ret0 = (InterCode)malloc(sizeof(struct InterCode_));
            ret0->kind = IR_LABEL;
            ret0->u.singleOp.op = label1;
            IR_append(ret0);

            dep_now++;
            Stack new_top1 = (Stack)malloc(sizeof(struct Stack_));
            new_top1->head = NULL;
            stack_push(new_top1);
            Stmt(stmt, ret_type);
            //IR
            InterCode ret1 = (InterCode)malloc(sizeof(struct InterCode_));
            ret1->kind = IR_GOTO;
            ret1->u.singleOp.op = label3;
            IR_append(ret1);

            stack_pop();
            dep_now--;
            TreeNode* else_node = stmt->next_brother;
            TreeNode* stmt_node = else_node->next_brother;
            dep_now++;
            Stack new_top2 = (Stack)malloc(sizeof(struct Stack_));
            new_top2->head = NULL;
            stack_push(new_top2);
            //IR
            InterCode ret2 = (InterCode)malloc(sizeof(struct InterCode_));
            ret2->kind = IR_LABEL;
            ret2->u.singleOp.op = label2;
            IR_append(ret2);

            Stmt(stmt_node, ret_type);
            //IR
            InterCode ret3 = (InterCode)malloc(sizeof(struct InterCode_));
            ret3->kind = IR_LABEL;
            ret3->u.singleOp.op = label3;
            IR_append(ret3);

            stack_pop();
            dep_now--;
        }
        else
        {
            Type exp_type = Exp(exp);
            if (exp_type == NULL)
                return;
            if (!(exp_type->kind == BASIC && exp_type->u.basic == BASIC_INT))
            {
                // error_num++;
                // printf("Error type 20 at line %d: the expr's type in if is not an integer.\n",root->lineno);
            }

            Operand label1 = new_lable();
            Operand label2 = new_lable();
            translate_Cond(exp, label1, label2);
            // printf("Cond_finish\n");
            InterCode ret0 = (InterCode)malloc(sizeof(struct InterCode_));
            ret0->kind = IR_LABEL;
            ret0->u.singleOp.op = label1;
            IR_append(ret0);

            dep_now++;
            Stack new_top1 = (Stack)malloc(sizeof(struct Stack_));
            new_top1->head = NULL;
            stack_push(new_top1);
            Stmt(stmt, ret_type);
            // IR
            InterCode ret1 = (InterCode)malloc(sizeof(struct InterCode_));
            ret1->kind = IR_LABEL;
            ret1->u.singleOp.op = label2;
            IR_append(ret1);

            stack_pop();
            dep_now--;
        }
    }
    else if(strcmp(first->type, "WHILE")==0)
    {
        TreeNode* lp = first->next_brother;
        TreeNode* exp = lp->next_brother;
        TreeNode* rp = exp->next_brother;
        TreeNode* stmt = rp->next_brother;
        Type exp_type = Exp(exp);
        if(exp_type==NULL)
            return ;
        if(!(exp_type->kind==BASIC&&exp_type->u.basic==BASIC_INT))
        {
            // error_num++;
            // printf("Error type 20 at line %d: the expr's type in while is not an integer.\n",root->lineno);
        }

        //IR
        Operand begin = new_lable();
        InterCode ret0 = (InterCode)malloc(sizeof(struct InterCode_));
        ret0->kind = IR_LABEL;
        ret0->u.singleOp.op = begin;
        IR_append(ret0);
        Operand labelin = new_lable();
        Operand labelout = new_lable();

        translate_Cond(exp, labelin, labelout);

        InterCode ret1 = (InterCode)malloc(sizeof(struct InterCode_));
        ret1->kind = IR_LABEL;
        ret1->u.singleOp.op = labelin;
        IR_append(ret1);

        Stmt(stmt, ret_type);

        //IR
        InterCode ret2 = (InterCode)malloc(sizeof(struct InterCode_));
        ret2->kind = IR_GOTO;
        ret2->u.singleOp.op = begin;
        IR_append(ret2);

        InterCode ret3 = (InterCode)malloc(sizeof(struct InterCode_));
        ret3->kind = IR_LABEL;
        ret3->u.singleOp.op = labelout;
        IR_append(ret3);
    }
}
void DefList_func(TreeNode* root)
{
    // printf("DefList_func\n");
    if(strcmp(root->type, "NULL")!=0)
    {
        TreeNode* def = root->first_child;
        TreeNode* deflist = def->next_brother;
        Def_func(def);
        DefList_func(deflist);
    }
}
void Def_func(TreeNode* root)
{
    // printf("Def_func\n");
    TreeNode* specifier = root->first_child;
    TreeNode* declist = specifier->next_brother;
    Type tmp = Specifier(specifier);
    DecList_func(declist, tmp);
}
void DecList_func(TreeNode* root, Type dectype)
{
    // printf("DecList_func\n");
    TreeNode* dec = root->first_child;
    if(dec->next_brother==NULL)
    {
        Dec_func(dec, dectype);
    }
    else
    {
        Dec_func(dec, dectype);
        TreeNode* comma = dec->next_brother;
        TreeNode* declist = comma->next_brother;
        DecList_func(declist, dectype);
    }
}
void Dec_func(TreeNode* root, Type dectype)
{
    // printf("Dec_func\n");
    TreeNode *vardec = root->first_child;
    HashNode var = VarDec(vardec, dectype);
    HashNode find_node = check(var->name);
    if (find_node == NULL) // 未找到重名
    {
        var->after = Top->head;
        Top->head = var;
        insertnode(var);
    }
    else if (find_node->val->kind != STRUCTURE && find_node->depth != var->depth) // 找到重名但不是同一个作用域
    {
        var->after = Top->head;
        Top->head = var;
        insertnode(var);
    }
    else // 找到重名
    {
        error_num++;
        printf("Error Type 3 at Line %d: Refined variable \"%s\".\n", vardec->lineno, var->name);
    }
    if(var->val->kind==STRUCTURE || var->val->kind==ARRAY) // 如果是结构体或数组需要DEC
    {
        int size = calculate_size(var->val);
        InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
        Operand temp_op = (Operand)malloc(sizeof(struct Operand_));
        temp_op->kind = VAR;
        strcpy(temp_op->u.name, var->name);
        code1->kind = IR_DEC;
        code1->u.dec.op = temp_op;
        code1->u.dec.size = size;
        IR_append(code1);
    }
    if(vardec->next_brother!=NULL)
    {
        TreeNode* assignop = vardec->next_brother;
        TreeNode* exp = assignop->next_brother;
        Type exp_type = Exp(exp);
        Operand t1 = new_temp();
        translate_Exp(exp, t1);
        InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
        Operand temp_op = (Operand)malloc(sizeof(struct Operand_));
        temp_op->kind = VAR;
        strcpy(temp_op->u.name, var->name);
        code1->kind = IR_ASSIGN;
        code1->u.assign.left = temp_op;
        code1->u.assign.right = t1;
        IR_append(code1);
        if(type_cmp(var->val, exp_type)==false)
        {
            error_num++;
            printf("Error Type 5 at Line %d: Type mismatched for assignment.\n", vardec->lineno);
        }
    }
}
// Expressions
Type Exp(TreeNode* root)
{
    // printf("Exp\n");
    TreeNode* first = root->first_child;
    if(strcmp(first->type, "Exp")==0)
    {
        TreeNode* second = first->next_brother;
        if(strcmp(second->type, "ASSIGNOP")==0)
        {
            return Exp_Assign(root);
        }
        else if(strcmp(second->type, "AND")==0)
        {
            return Exp_And(root);
        }
        else if(strcmp(second->type, "OR")==0)
        {
            return Exp_And(root);
        }
        else if(strcmp(second->type, "RELOP")==0)
        {
            return Exp_Math(root);
        }
        else if(strcmp(second->type, "PLUS")==0)
        {
            return Exp_Math(root);
        }
        else if(strcmp(second->type, "MINUS")==0)
        {
            return Exp_Math(root);
        }
        else if(strcmp(second->type, "STAR")==0)
        {
            return Exp_Math(root);
        }
        else if(strcmp(second->type, "DIV")==0)
        {
            return Exp_Math(root);
        }
        else if(strcmp(second->type, "LB")==0)
        {
            return Exp_Array(root);
        }
        else if(strcmp(second->type, "DOT")==0)
        {
            return Exp_Dot(root);
        }
    }
    else if(strcmp(first->type, "LP")==0)
    {
        TreeNode* second = first->next_brother;
        Type tmp = Exp(second);
        if(second->is_left==1)
            root->is_left = 1;
        return tmp;
    }
    else if(strcmp(first->type, "MINUS")==0)
    {
        TreeNode* second = first->next_brother;
        Type tmp = Exp(second);
        // 语义检查
        if(tmp==NULL)
        {
            return NULL;
        }
        if(tmp->kind==BASIC)
        {
            return tmp;
        }
        else
        {
            error_num++;
            printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
            return NULL;
        }
    }
    else if(strcmp(first->type, "NOT")==0)
    {
        TreeNode* second = first->next_brother;
        Type tmp = Exp(second);
        if(tmp==NULL)
        {
            return NULL;
        }
        if(tmp->kind==BASIC&&tmp->u.basic==BASIC_INT)
        {
            return tmp;
        }
        else
        {
            error_num++;
            printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
            return NULL;
        }
    }
    else if(strcmp(first->type, "ID")==0)
    {
        TreeNode* second = first->next_brother;
        if(second==NULL)
        {
            return Exp_ID(root);
        }
        else 
        {
            return Exp_Func(root);
        }
    }
    else if(strcmp(first->type, "INT")==0)
    {
        Type type=(Type)malloc(sizeof(struct Type_));
        type->kind=BASIC;
        type->u.basic=BASIC_INT;
        return type;
    }
    else if(strcmp(first->type, "FLOAT")==0)
    {
        Type type=(Type)malloc(sizeof(struct Type_));
        type->kind=BASIC;
        type->u.basic=BASIC_FLOAT;
        return type;
    }
}
Type Exp_Assign(TreeNode* root) // check
{
    TreeNode* exp1 = root->first_child;
    TreeNode* assignop = exp1->next_brother;
    TreeNode* exp2 = assignop->next_brother;
    Type left_type = Exp(exp1);
    Type right_type = Exp(exp2);
    // printf("%d %d\n", left_type->u.basic, right_type->u.basic);
    // 语义检查
    if(left_type==NULL||right_type==NULL)
    {
        return NULL;
    }
    else
    {
        if(type_cmp(left_type, right_type)==false)
        {
            error_num++;
            printf("Error type 5 at Line %d: Type mismatched for assignment.\n",root->lineno);
            return NULL;
        }
        else if(exp1->is_left==0)
        {
            error_num++;
            printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",root->lineno);
            return NULL;
        }
        else
        {
            return left_type;
        }
        
    }
}
Type Exp_And(TreeNode* root)
{
    TreeNode* exp1 = root->first_child;
    TreeNode* and = exp1->next_brother;
    TreeNode* exp2 = and->next_brother;
    Type left_type = Exp(exp1);
    Type right_type = Exp(exp2);
    if(left_type==NULL||right_type==NULL)
    {
        return NULL;
    }
    if(type_cmp(left_type, right_type)==true&&left_type->kind==BASIC&&left_type->u.basic==BASIC_INT)
    {
        return left_type;
    }
    else 
    {
        error_num++;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
        return NULL;
    }
}
Type Exp_Math(TreeNode* root)
{
    TreeNode* exp1 = root->first_child;
    TreeNode* math = exp1->next_brother;
    TreeNode* exp2 = math->next_brother;
    Type left_type = Exp(exp1);
    Type right_type = Exp(exp2);
    // 语义检查
    if(left_type==NULL||right_type==NULL)
    {
        return NULL;
    }
    if(type_cmp(left_type, right_type)==true&&left_type->kind==BASIC)
    {
        return left_type;
    }
    else 
    {
        error_num++;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
        return NULL;
    }
}
Type Exp_ID(TreeNode* root)
{
    TreeNode* id = root->first_child;
    HashNode find_node = check(id->val);
    if(find_node==NULL || find_node->val->kind==FUNCTION)
    {
        error_num++;
        printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",root->lineno, id->val);
        return NULL;
    }
    else
    {
        root->is_left = 1;
        return find_node->val;
    }
}
Type Exp_Func(TreeNode* root)
{
    TreeNode* id = root->first_child;
    HashNode find_node = check(id->val);
    if(find_node==NULL)
    {
        error_num++;
        printf("Error type 2 at Line %d: Undefined function \"%s\".\n",root->lineno, id->val);
        return NULL;
    }
    else if(find_node->val->kind!=FUNCTION)
    {
        error_num++;
        printf("Error type 11 at Line %d: \"%s\" is not a function.\n",root->lineno, id->val);
        return NULL;
    }
    else
    {
        FieldList func = find_node->val->u.function;
        Type ret_type = func->type;
        func = func->tail;
        TreeNode* lp = id->next_brother;
        TreeNode* args = lp->next_brother;
        if(strcmp(args->type, "Args") == 0)
        {
            if(Args(args, func)==0)
            {
                printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",root->lineno, id->val);
                return NULL;
            }
            else
            {
                return ret_type;
            }
        }
        else
        {
            if(func==NULL)
            {
                return ret_type;
            }
            else
            {
                printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",root->lineno, id->val);
                return NULL;
            }
        }
    }
}
int Args(TreeNode* root, FieldList func)
{
    if(func==NULL)
    {
        return 0;
    }
    else
    {
        TreeNode* exp = root->first_child;
        Type exp_type = Exp(exp);
        if(exp_type == NULL)
            return 1;
        if(type_cmp(func->type, exp_type)==true)
        {
            TreeNode* second = exp->next_brother;
            if(second==NULL)
            {
                return 1;
            }
            else
            {
                return Args(second->next_brother, func->tail);
            }
        }
        else
        {
            return 0;
        }
    }
}
Type Exp_Array(TreeNode* root)
{
    TreeNode* exp1 = root->first_child;
    TreeNode* lb = exp1->next_brother;
    TreeNode* exp2 = lb->next_brother;
    Type left_type = Exp(exp1);
    Type right_type = Exp(exp2);
    if(left_type==NULL||right_type==NULL)
        return NULL;
    if(left_type->kind!=ARRAY)
    {
        error_num++;
        printf("Error Type 10 at Line %d: Not an array.\n", root->lineno);
        return NULL;
    }
    else if(right_type->kind!=BASIC || right_type->u.basic!=BASIC_INT)
    {
        error_num++;
        printf("Error Type 12 at Line %d: Not an integer between [].\n", root->lineno);
        return NULL;
    }
    else
    {
        root->is_left = 1;
        return left_type->u.array.elem;
    }
}
Type Exp_Dot(TreeNode* root)
{
    TreeNode* exp = root->first_child;
    TreeNode* dot = exp->next_brother;
    TreeNode* id = dot->next_brother;
    Type type = Exp(exp);
    if(type==NULL)
    {
        return NULL;
    }
    if(type->kind!=STRUCTURE)
    {
        error_num++;
        printf("Error Type 13 at Line %d: Not a structure.\n", root->lineno);
        return NULL;
    }
    else
    {
        FieldList member = type->u.structure;
        while(member!=NULL)
        {
            if(strcmp(member->name, id->val)==0)
            {
                break;
            }
            member = member->tail;
        }
        if(member==NULL)
        {
            error_num++;
            printf("Error type 14 at Line %d: Undefined field \"%s\".\n",root->lineno, id->val);
            return NULL;
        }
        else
        {
            root->is_left = 1;
            return member->type;
        }
    }
}
void translate_Exp(TreeNode* root, Operand place)
{
    // printf("translate_Exp\n");
    TreeNode* first = root->first_child;
    if(strcmp(first->type, "Exp")==0)
    {
        TreeNode* second = first->next_brother;
        if(strcmp(second->type, "ASSIGNOP")==0)
        {
            translate_Exp_Assign(root, place);
        }
        else if(strcmp(second->type, "AND")==0)
        {
            translate_Exp_Logic(root, place);
        }
        else if(strcmp(second->type, "OR")==0)
        {
            translate_Exp_Logic(root, place);
        }
        else if(strcmp(second->type, "RELOP")==0)
        {
            translate_Exp_Logic(root, place);
        }
        else if(strcmp(second->type, "PLUS")==0)
        {
            translate_Exp_Math(root, place);
        }
        else if(strcmp(second->type, "MINUS")==0)
        {
            translate_Exp_Math(root, place);
        }
        else if(strcmp(second->type, "STAR")==0)
        {
            translate_Exp_Math(root, place);
        }
        else if(strcmp(second->type, "DIV")==0)
        {
            translate_Exp_Math(root, place);
        }
        else if(strcmp(second->type, "LB")==0) //
        {
            translate_Exp_Array(root, place);
        }
        else if(strcmp(second->type, "DOT")==0)
        {
            translate_Exp_Struct(root, place);
        }
    }
    else if(strcmp(first->type, "LP")==0)
    {
        TreeNode* second = first->next_brother;
        translate_Exp(second, place);
    }
    else if(strcmp(first->type, "MINUS")==0)
    {
        TreeNode* second = first->next_brother;
        Operand t1 = new_temp();
        translate_Exp(second, t1);
        Operand temp_op = (Operand)malloc(sizeof(struct Operand_));
        temp_op->kind = CONSTANT;
        temp_op->u.value = 0;
        InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
        code1->kind = IR_SUB;
        code1->u.binOp.op1 = temp_op;
        code1->u.binOp.op2 = t1;
        code1->u.binOp.result = place;
        IR_append(code1);
    }
    else if(strcmp(first->type, "NOT")==0)
    {
        translate_Exp_Logic(root, place);
    }
    else if(strcmp(first->type, "ID")==0)
    {
        TreeNode* second = first->next_brother;
        if(second==NULL)
        {
            place->kind = VAR;
            strcpy(place->u.name, first->val);
        }
        else
        {
            translate_Exp_Func(root, place);
        }
    }
    else if(strcmp(first->type, "INT")==0)
    {
        place->kind = CONSTANT;
        place->u.value = strtol(first->val, NULL, 0);
    }
}
void translate_Exp_Assign(TreeNode* root, Operand place)
{
    TreeNode* exp1 = root->first_child;
    TreeNode* assign = exp1->next_brother;
    TreeNode* exp2 = assign->next_brother;
    translate_Exp(exp1, place);
    Operand t1 = new_temp();
    translate_Exp(exp2, t1);
    InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
    code1->kind = IR_ASSIGN;
    code1->u.assign.left = place;
    code1->u.assign.right = t1;
    IR_append(code1);
}
void translate_Exp_Math(TreeNode* root, Operand place)
{
    TreeNode* exp1 = root->first_child;
    TreeNode* math = exp1->next_brother;
    TreeNode* exp2 = math->next_brother;
    Operand t1 = new_temp();
    Operand t2 = new_temp();
    translate_Exp(exp1, t1);
    translate_Exp(exp2, t2);
    InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
    code1->u.binOp.op1 = t1;
    code1->u.binOp.op2 = t2;
    code1->u.binOp.result = place;
    if(strcmp(math->type, "PLUS")==0)
    {
        code1->kind = IR_ADD;
    }
    else if(strcmp(math->type, "MINUS")==0)
    {
        code1->kind = IR_SUB;
    }
    else if(strcmp(math->type, "STAR")==0)
    {
        code1->kind = IR_MUL;
    }
    else if(strcmp(math->type, "DIV")==0)
    {
        code1->kind = IR_DIV;
    }
    IR_append(code1);
}
void translate_Exp_Logic(TreeNode* root, Operand place)
{
    Operand lable1 = new_lable();
    Operand lable2 = new_lable();
    InterCode code0 = (InterCode)malloc(sizeof(struct InterCode_));
    Operand temp_op1 = (Operand)malloc(sizeof(struct Operand_));
    temp_op1->kind = CONSTANT;
    temp_op1->u.value = 0;
    code0->kind = IR_ASSIGN;
    code0->u.assign.left = place;
    code0->u.assign.right = temp_op1;
    IR_append(code0);
    translate_Cond(root, lable1, lable2);
    InterCode code2 = (InterCode)malloc(sizeof(struct InterCode_));
    code2->kind = IR_LABEL;
    code2->u.singleOp.op = lable1;
    IR_append(code2);
    InterCode code3 = (InterCode)malloc(sizeof(struct InterCode_));
    Operand temp_op2 = (Operand)malloc(sizeof(struct Operand_));
    temp_op2->kind = CONSTANT;
    temp_op1->u.value = 1;
    code3->kind = IR_ASSIGN;
    code3->u.assign.left = place;
    code3->u.assign.right = temp_op2;
    IR_append(code3);
    InterCode code4 = (InterCode)malloc(sizeof(struct InterCode_));
    code4->kind = IR_LABEL;
    code4->u.singleOp.op = lable2;
    IR_append(code4);
}
void translate_Cond(TreeNode* root, Operand lable_true, Operand lable_false)
{
    // printf("translate_Cond\n");
    TreeNode* first = root->first_child;
    if(strcmp(first->type, "Exp")==0)
    {
        TreeNode* second = first->next_brother;
        if(strcmp(second->type, "RELOP")==0)
        {
            TreeNode* exp1 = first;
            TreeNode* exp2 = second->next_brother;
            Operand t1 = new_temp();
            Operand t2 = new_temp();
            translate_Exp(exp1, t1);
            translate_Exp(exp2, t2);
            InterCode code3 = (InterCode)malloc(sizeof(struct InterCode_));
            code3->kind = IR_IF_GOTO;
            code3->u.ifgoto.label = lable_true;
            strcpy(code3->u.ifgoto.relop, second->val);
            code3->u.ifgoto.x = t1;
            code3->u.ifgoto.y = t2;
            IR_append(code3);
            InterCode code4 = (InterCode)malloc(sizeof(struct InterCode_));
            code4->kind = IR_GOTO;
            code4->u.singleOp.op = lable_false;
            IR_append(code4);
        }
        else if(strcmp(second->type, "AND")==0)
        {
            TreeNode* exp1 = first;
            TreeNode* exp2 = second->next_brother;
            Operand lable1 = new_lable();
            translate_Cond(exp1, lable1, lable_false);
            InterCode code2 = (InterCode)malloc(sizeof(struct InterCode_));
            code2->kind = IR_LABEL;
            code2->u.singleOp.op = lable1;
            IR_append(code2);
            translate_Cond(exp2, lable_true, lable_false);
        }
        else if(strcmp(second->type, "OR")==0)
        {
            TreeNode* exp1 = first;
            TreeNode* exp2 = second->next_brother;
            Operand lable1 = new_lable();
            translate_Cond(exp1, lable_true, lable1);
            InterCode code2 = (InterCode)malloc(sizeof(struct InterCode_));
            code2->kind = IR_LABEL;
            code2->u.singleOp.op = lable1;
            IR_append(code2);
            translate_Cond(exp2, lable_true, lable_false);
        }
        else
        {
            Operand t1 = new_temp();
            translate_Exp(root, t1);
            InterCode code2 = (InterCode)malloc(sizeof(struct InterCode_));
            code2->kind = IR_IF_GOTO;
            Operand temp_op1 = (Operand)sizeof(struct Operand_);
            temp_op1->kind = CONSTANT;
            temp_op1->u.value = 0;
            code2->u.ifgoto.label = lable_true;
            strcpy(code2->u.ifgoto.relop, "!=");
            code2->u.ifgoto.x = t1;
            code2->u.ifgoto.y = temp_op1;
            IR_append(code2);
            InterCode code4 = (InterCode)malloc(sizeof(struct InterCode_));
            code4->kind = IR_GOTO;
            code4->u.singleOp.op = lable_false;
            IR_append(code4);
        }
    }
    else if(strcmp(first->type, "NOT")==0)
    {
        TreeNode* second = first->next_brother;
        translate_Cond(second, lable_false, lable_true);
    }
    else
    {
        Operand t1 = new_temp();
        translate_Exp(root, t1);
        InterCode code2 = (InterCode)malloc(sizeof(struct InterCode_));
        code2->kind = IR_IF_GOTO;
        Operand temp_op1 = (Operand)sizeof(struct Operand_);
        temp_op1->kind = CONSTANT;
        temp_op1->u.value = 0;
        code2->u.ifgoto.label = lable_true;
        strcpy(code2->u.ifgoto.relop, "!=");
        code2->u.ifgoto.x = t1;
        code2->u.ifgoto.y = temp_op1;
        IR_append(code2);
        InterCode code4 = (InterCode)malloc(sizeof(struct InterCode_));
        code4->kind = IR_GOTO;
        code4->u.singleOp.op = lable_false;
        IR_append(code4);
    }
    // printf("Finish\n");
}
void translate_Exp_Func(TreeNode* root, Operand place)
{
    TreeNode* id = root->first_child;
    TreeNode* lp = id->next_brother;
    TreeNode* third = lp->next_brother;
    if(strcmp(third->type, "RP")==0)
    {
        HashNode find_node = check(id->val);
        if(strcmp(id->val, "read")==0)
        {
            InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
            code1->kind = IR_READ;
            code1->u.singleOp.op = place;
            IR_append(code1);
        }
        else
        {
            Operand temp_op = (Operand)malloc(sizeof(struct Operand_));
            InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
            temp_op->kind = FUNC;
            strcpy(temp_op->u.name, find_node->name);
            code1->kind = IR_CALL;
            code1->u.assign.left = place;
            code1->u.assign.right = temp_op;
            IR_append(code1);
        }
    }
    else
    {
        HashNode find_node = check(id->val);
        ArgList arg_list = translate_Args(third);
        if(strcmp(id->val, "write")==0)
        {
            InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
            code1->kind = IR_WRITE;
            code1->u.singleOp.op = arg_list->arg;
            IR_append(code1);
        }
        else
        {
            while(arg_list!=NULL)
            {
                InterCode code = (InterCode)malloc(sizeof(struct InterCode_));
                code->kind = IR_ARG;
                code->u.singleOp.op = arg_list->arg;
                IR_append(code);
                arg_list = arg_list->next;
            }
            Operand temp_op = (Operand)malloc(sizeof(struct Operand_));
            temp_op->kind = FUNC;
            strcpy(temp_op->u.name, id->val);
            InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
            code1->kind = IR_CALL;
            code1->u.assign.left = place;
            code1->u.assign.right = temp_op;
            IR_append(code1);
        }
    }
}
ArgList translate_Args(TreeNode* root)
{
    ArgList ret = NULL;
    TreeNode* exp1 = root->first_child;
    while(exp1->next_brother!=NULL)
    {
        Operand t1 = new_temp();
        translate_Exp(exp1, t1);
        Type type = Exp(exp1);
        if(type->kind==STRUCTURE || type->kind==ARRAY)
        {
            t1->kind = ADDR;
        }
        ArgList temp_arg = (ArgList)malloc(sizeof(struct ArgList_));
        temp_arg->arg = t1;
        temp_arg->next = ret;
        ret = temp_arg;
        TreeNode* args = exp1->next_brother->next_brother;
        exp1 = args->first_child;
    }
    Operand t1 = new_temp();
    translate_Exp(exp1, t1);
    Type type = Exp(exp1);
    if (type->kind == STRUCTURE || type->kind == ARRAY)
    {
        t1->kind = ADDR;
    }
    ArgList temp_arg = (ArgList)malloc(sizeof(struct ArgList_));
    temp_arg->arg = t1;
    temp_arg->next = ret;
    ret = temp_arg;
    return ret;
}
void translate_Exp_Struct(TreeNode* root, Operand place) // 结构体成员访问
{ 
    TreeNode* exp = root->first_child;
    TreeNode* id = exp->next_brother->next_brother;
    Operand t1 = new_temp();
    translate_Exp(exp, t1);
    Type type = Exp(exp);
    FieldList field = type->u.structure;
    int size = 0;
    while(field != NULL)
    {
        if(strcmp(field->name, id->val)==0) break;
        else
        {
            size += calculate_size(field->type);
            field = field->tail;
        }
    }
    Operand offset = (Operand)malloc(sizeof(struct Operand_));
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    strcpy(tmp->u.name, place->u.name);
    tmp->kind = VAR;
    offset->kind = CONSTANT;
    offset->u.value = size;
    HashNode find_node = check(t1->u.name);
    if (find_node==NULL) 
        t1->kind = VAR;
    else 
    {
        if(find_node->flag==true)
            t1->kind = VAR;
        else
            t1->kind = ADDR;
    }

    InterCode ret = (InterCode)malloc(sizeof(struct InterCode_));
    ret->kind = IR_ADD;
    ret->u.binOp.op1 = t1;
    ret->u.binOp.op2 = offset;
    ret->u.binOp.result = tmp;
    IR_append(ret);
    place->kind = MYSTAR;
}
void translate_Exp_Array(TreeNode* root, Operand place)
{
    TreeNode *exp1 = root->first_child;
    TreeNode *exp2 = exp1->next_brother->next_brother;
    Operand array = new_temp();
    translate_Exp(exp1, array);
    Operand value = new_temp();
    translate_Exp(exp2, value);
    Operand tmp = new_temp();
    tmp->kind = place->kind;
    strcpy(tmp->u.name, place->u.name);
    InterCode ret0 = (InterCode)malloc(sizeof(struct InterCode_));
    ret0->kind = IR_ADD;
    bool tmpflg = false;
    if (exp1->val != NULL)
    {
        HashNode tmphash = check(array->u.name);
        if (tmphash != NULL)
            tmpflg = tmphash->flag;
    }
    if (tmpflg || array->kind == MYSTAR)
        array->kind = VAR;
    else
        array->kind = ADDR;
    ret0->u.binOp.op1 = array;

    Operand tmp1 = new_temp();
    Operand cons = (Operand)malloc(sizeof(struct Operand_));
    cons->kind = CONSTANT;
    Type type = Exp(exp1);
    cons->u.value = calculate_size(type->u.array.elem);
    InterCode ret1 = (InterCode)malloc(sizeof(struct InterCode_));
    ret1->kind = IR_MUL;
    ret1->u.binOp.op1 = cons;
    ret1->u.binOp.op2 = value;
    ret1->u.binOp.result = tmp1;
    IR_append(ret1);
    ret0->u.binOp.op2 = tmp1;
    ret0->u.binOp.result = tmp;
    IR_append(ret0);
    place->kind = MYSTAR;
}