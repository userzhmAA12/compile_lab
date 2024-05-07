#include "semantics.h"
#include "intercode.h"
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

// high-level definitions
void Program(TreeNode *root)
{
    dep_now = 0;
    Top = NULL;
    TreeNode* first = root->first_child;
    ExtDefList(first);
}
void ExtDefList(TreeNode *root)
{
    if(strcmp(root->type, "NULL")==0)return ;
    TreeNode* first = root->first_child;
    TreeNode* second = first->next_brother;
    ExtDef(first);
    ExtDefList(second);
}
void ExtDef(TreeNode *root)
{
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
            Compst(third, type);
            stack_pop();
            dep_now--;
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
        }
    }
}
void ExtDecList(TreeNode *root, Type type)
{
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
            insertnode(node);
        }
    }
}
void DefList_struct(TreeNode* root, FieldList head)
{
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
    TreeNode* first = root->first_child;
    TreeNode* second = first->next_brother;
    Type type = Specifier(first);
    FieldList ret = DecList_struct(second, type);
    return ret;
}
FieldList DecList_struct(TreeNode* root, Type dectype)
{
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
        return ret;
    }
}
HashNode FunDec(TreeNode* root, Type ret_type)
{
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
    tmp_op->kind = FUNCTION;
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
    return ret;
}
FieldList VarList(TreeNode* root)
{
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
        insertnode(tmp);

        //IR生成
        InterCode code1 = (InterCode)malloc(sizeof(struct InterCode_));
        Operand tmp_op = (Operand)malloc(sizeof(struct Operand_));
        tmp_op->kind = VAR;
        strcpy(tmp_op->u.name, first->val);
        code1->kind = IR_PARAM;
        code1->u.singleOp.op = tmp_op;
        IR_append(code1);
    }
    return ret;
}
// Statements
void Compst(TreeNode* root, Type ret_type)
{
    TreeNode* lc = root->first_child;
    TreeNode* deflist = lc->next_brother;
    TreeNode* stmtlist = deflist->next_brother;
    DefList_func(deflist);
    StmtList(stmtlist, ret_type);
}
void StmtList(TreeNode* root, Type ret_type)
{
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
    TreeNode* first = root->first_child;
    if(strcmp(first->type, "Exp")==0)
    {
        Exp(first);
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
    }
    else if(strcmp(first->type, "IF")==0)
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
            // printf("Error type 20 at line %d: the expr's type in if is not an integer.\n",root->lineno);
        }
        dep_now++;
        Stack new_top1 = (Stack)malloc(sizeof(struct Stack_));
        new_top1->head = NULL;
        stack_push(new_top1);
        Stmt(stmt, ret_type);
        stack_pop();
        dep_now--;
        if(stmt->next_brother!=NULL)
        {
            TreeNode* else_node = stmt->next_brother;
            TreeNode* stmt_node = else_node->next_brother;
            dep_now++;
            Stack new_top2 = (Stack)malloc(sizeof(struct Stack_));
            new_top2->head = NULL;
            stack_push(new_top2);
            Stmt(stmt_node, ret_type);
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
        Stmt(stmt, ret_type);
    }
}
void DefList_func(TreeNode* root)
{
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
    TreeNode* specifier = root->first_child;
    TreeNode* declist = specifier->next_brother;
    Type tmp = Specifier(specifier);
    DecList_func(declist, tmp);
}
void DecList_func(TreeNode* root, Type dectype)
{
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
    if(vardec->next_brother!=NULL)
    {
        TreeNode* assignop = vardec->next_brother;
        TreeNode* exp = assignop->next_brother;
        Type exp_type = Exp(exp);
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
Type Exp_Assign(TreeNode* root)
{
    TreeNode* exp1 = root->first_child;
    TreeNode* assignop = exp1->next_brother;
    TreeNode* exp2 = assignop->next_brother;
    Type left_type = Exp(exp1);
    Type right_type = Exp(exp2);
    // printf("%d %d\n", left_type->u.basic, right_type->u.basic);
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
            return left_type;
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