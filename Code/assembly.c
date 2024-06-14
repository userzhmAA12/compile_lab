#include"assembly.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

FILE* F;

struct BitVector_ in[1024];
struct BitVector_ out[1024];
struct BitVector_ def[1024];
struct BitVector_ use[1024];
struct BitVector_ succ[1024];

struct RegInfo_ Regs[32]; //寄存器描述符表

VarInfo VarMap; //变量描述符表头
int var_num; //变量描述符总数
int var_off; //变量偏移量

Lable2num lableMap; // 储存lable和行号之间的对应关系

void bit_set(BitVector vec, int i)
{
    vec->val[i >> SHIFT] |= 1 << ((i & MASK));
}
void bit_clr(BitVector vec, int i)
{
    vec->val[i >> SHIFT] &= ~(1 << (i & MASK));
}
int bit_get(BitVector vec, int i)
{
    return vec->val[i >> SHIFT] & (1 << (i&MASK));
}
void vec_clr(BitVector vec)
{
    for(int i=0; i<VECNUM; i++)
    {
        vec->val[i] = 0;
    }
}
bool vec_cmp(BitVector vec1, BitVector vec2)
{
    bool ret = true;
    for(int i = 0; i < VECNUM; i++)
    {
        if(vec1->val[i]!=vec2->val[i])
            ret = false;
    }
    return ret;
}
struct BitVector_ vec_and(BitVector vec1, BitVector vec2)
{
    struct BitVector_ ret;
    for(int i = 0; i < VECNUM; i++)
    {
        ret.val[i] = vec1->val[i] & vec2->val[i];
    }
    return ret;
}
struct BitVector_ vec_or(BitVector vec1, BitVector vec2)
{
    struct BitVector_ ret;
    for(int i = 0; i < VECNUM; i++)
    {
        ret.val[i] = vec1->val[i] | vec2->val[i];
    }
    return ret;
}
struct BitVector_ vec_div(BitVector vec1, BitVector vec2)
{
    struct BitVector_ ret;
    for(int i = 0; i < VECNUM; i++)
    {
        ret.val[i] = vec1->val[i] & (~vec2->val[i]);
    }
    return ret;
}
void active_Analysis(InterCode func)
{
    int sum = 1; //这个代码段的代码总数
    InterCode code  = func->next;
    vec_clr(&in[0]);
    vec_clr(&out[0]);
    vec_clr(&def[0]);
    vec_clr(&use[0]);
    succ[0].val[0] = sum+1;
    succ[0].val[1] = -1;
    while(code!=NULL&&code->kind!=IR_FUNCTION) //遍历代码段对in、out进行初始化
    {
        vec_clr(&def[sum]);
        vec_clr(&use[sum]);
        if(code->kind==IR_FUNCTION)
        {
            succ[sum].val[0] = sum+1;
            succ[sum].val[1] = -1;
        }
        else if(code->kind==IR_LABEL)
        {
            Lable2num lable = (Lable2num)malloc(sizeof(struct Lable2num_));
            strcpy(lable->lable_name, code->u.singleOp.op->u.name);
            lable->num = sum;
            lable->next = lableMap;
            lableMap = lable;
            succ[sum].val[0] = sum+1;
            succ[sum].val[1] = -1;
        }
        else if(code->kind==IR_ASSIGN||code->kind==IR_GET_ADDR||code->kind==IR_READ_ADDR||code->kind==IR_WRITE_ADDR)
        {
            VarInfo tmp = check_var(code->u.assign.left->u.name, 4);
            int no_left = tmp->num;
            bit_set(&def[sum], no_left);
            if(code->u.assign.right->kind!=CONSTANT)
            {
                tmp = check_var(code->u.assign.right->u.name, 4);
                int no_right = tmp->num;
                bit_set(&use[sum], no_right);
                tmp->last_use = sum;
            }
            succ[sum].val[0] = sum+1;
            succ[sum].val[1] = -1;
        }
        else if(code->kind==IR_ADD||code->kind==IR_SUB||code->kind==IR_MUL||code->kind==IR_DIV)
        {
            VarInfo tmp = check_var(code->u.binOp.result->u.name, 4);
            int no_result = tmp->num;
            bit_set(&def[sum], no_result);
            if(code->u.binOp.op1->kind!=CONSTANT)
            {
                tmp = check_var(code->u.binOp.op1->u.name, 4);
                int no_op1 = tmp->num;
                bit_set(&use[sum], no_op1);
                tmp->last_use = sum;
            }
            if(code->u.binOp.op2->kind!=CONSTANT)
            {
                tmp = check_var(code->u.binOp.op2->u.name, 4);
                int no_op2 = tmp->num;
                bit_set(&use[sum], no_op2);
                tmp->last_use = sum;
            }
            succ[sum].val[0] = sum+1;
            succ[sum].val[1] = -1;
        }
        else if(code->kind==IR_RETURN)
        {
            if(code->u.singleOp.op->kind!=CONSTANT)
            {
                VarInfo tmp = check_var(code->u.singleOp.op->u.name, 4);
                int no = tmp->num;
                bit_set(&use[sum], no);
                tmp->last_use = sum;
            }
            succ[sum].val[0] = -1;
            succ[sum].val[1] = -1;
        }
        else if(code->kind==IR_DEC)
        {
            VarInfo tmp = check_var(code->u.dec.op->u.name, code->u.dec.size);
            int no_op = tmp->num;
            bit_set(&def[sum], no_op);
            succ[sum].val[0] = sum+1;
            succ[sum].val[1] = -1;
        }
        else if(code->kind==IR_ARG||code->kind==IR_WRITE)
        {
            if(code->u.singleOp.op->kind!=CONSTANT)
            {
                VarInfo tmp = check_var(code->u.singleOp.op->u.name, 4);
                int no = tmp->num;
                bit_set(&use[sum], no);
                tmp->last_use = sum;
            }
            succ[sum].val[0] = sum+1;
            succ[sum].val[1] = -1;
        }
        else if(code->kind==IR_CALL)
        {
            VarInfo tmp = check_var(code->u.assign.left->u.name, 4);
            int no = tmp->num;
            bit_set(&def[sum], no);
            succ[sum].val[0] = sum+1;
            succ[sum].val[1] = -1;
        }
        else if(code->kind==IR_PARAM || code->kind==IR_READ)
        {
            VarInfo tmp = check_var(code->u.singleOp.op->u.name, 4);
            int no = tmp->num;
            bit_set(&def[sum], no);
            succ[sum].val[0] = sum+1;
            succ[sum].val[1] = -1;
        }
        vec_clr(&in[sum]);
        vec_clr(&out[sum]);
        sum++;
        code = code->next;
    }
    code = func->next;
    sum = 1;
    while(code!=NULL&&code->kind!=IR_FUNCTION) //遍历代码段对in、out进行初始化
    {
        if(code->kind==IR_GOTO)
        {
            int no = check_lable(code->u.singleOp.op->u.name);
            succ[sum].val[0] = no;
            succ[sum].val[1] = -1;
        }
        else if(code->kind==IR_IF_GOTO)
        {
            if(code->u.ifgoto.x->kind!=CONSTANT)
            {
                VarInfo tmp = check_var(code->u.ifgoto.x->u.name, 4);
                int no_x = tmp->num;
                bit_set(&use[sum], no_x);
                tmp->last_use = sum;
            }
            if(code->u.ifgoto.y->kind!=CONSTANT)
            {
                VarInfo tmp = check_var(code->u.ifgoto.y->u.name, 4);
                int no_y = tmp->num;
                bit_set(&use[sum], no_y);
                tmp->last_use = sum;
            }
            int no_lable = check_lable(code->u.ifgoto.label->u.name);
            succ[sum].val[0] = sum+1;
            succ[sum].val[1] = no_lable;
        }
        sum++;
        code = code->next;
    }
    bool flag = false;
    while(!flag) //迭代使其收敛
    {
        for(int i = 0;i<sum;i++)
        {
            struct BitVector_ temp_in = in[i];
            struct BitVector_ temp_out = out[i];
            struct BitVector_ temp = vec_div(&out[i], &def[i]);
            in[i] = vec_or(&use[i], &temp);
            if(succ[i].val[0]!=-1)
            {
                out[i] = in[succ[i].val[0]];
                if(succ[i].val[1]!=-1)
                {
                    out[i] = vec_or(&out[i], &in[succ[i].val[1]]);
                }
            }
            if(vec_cmp(&temp_in, &in[i])==false || vec_cmp(&temp_out, &out[i])==false)
                flag = false;
            else 
                flag = true;
        }
    }
}
VarInfo check_var(char* name, int size) //查找一个变量对应的序号，如果变量不在表中则新建一个插入，返回对应的序号
{
    VarInfo ret = NULL;
    VarInfo now = VarMap;
    while(now!=NULL)
    {
        if(strcmp(name, now->var_name)==0)
        {
            ret = now;
            break;
        }
        now = now->next;
    }
    if(ret==NULL) //如果未找到，则新建一个插入
    {
        VarInfo new_var = (VarInfo)malloc(sizeof(struct VarInfo_));
        strcpy(new_var->var_name, name);
        var_off-=size;
        new_var->num = var_num;
        new_var->in_reg = false;
        new_var->next = VarMap;
        new_var->offset = var_off;
        VarMap = new_var;
        var_num++;
        ret = new_var;
    }
    return ret;
}
int check_lable(char* name)
{
    int ret = -1;
    Lable2num now = lableMap;
    while(now!=NULL)
    {
        if(strcmp(name, now->lable_name)==0)
        {
            ret = now->num;
            break;
        }
        now = now->next;
    }
    assert(ret!=-1); //一定能找到
    return ret;
}
void clr_var() //清空变量描述符
{
    
    while(VarMap!=NULL)
    {
        VarInfo now = VarMap;
        VarMap = VarMap->next;
        free(now);
    }
}
void clr_lable() //清空lablemap
{
    while(lableMap!=NULL)
    {
        Lable2num now = lableMap;
        lableMap = lableMap->next;
        free(now);
    }
}
void reg_free(int id)
{
    Regs[id].is_free = true;
    if(Regs[id].var!=NULL)
    {
        fprintf(F, "sw %s, %d($fp)\n", Regs[id].name, Regs[id].var->offset);
        Regs[id].var->in_reg = false;
        Regs[id].var = NULL;
    }
}
int reg_allocate(VarInfo var)
{
    int time = -1;
    int ret = -1;
    for(int i=8;i<26;i++) //$s0-$s7、$t0-$t9
    {
        if(Regs[i].is_free==true)
        {
            Regs[i].is_free = false;
            Regs[i].var = var;
            return i;
        }
        else if(Regs[i].var!=NULL)//记录一下寄存器中变量的最晚use时间，用来在溢出时使用
        {
            if(Regs[i].var->last_use>time && Regs[i].use==false)
            {
                time = Regs[i].var->last_use;
                ret = i;
            }
        }
    }
    // 如果没有空闲寄存器,进行溢出操作
    VarInfo tmp = Regs[ret].var;
    //把原寄存器中内容存到内存中
    fprintf(F, "sw %s, %d($fp)\n", Regs[ret].name, var->offset);
    Regs[ret].var = var;
    return ret;
}
int get_reg(Operand op) //根据变量名返回寄存器编号
{
    if(op->kind==CONSTANT) //常量
    {
        if(op->u.value==0) //是0则直接返回$0
            return 0;
        int reg_no = reg_allocate(NULL);
        //将内容读入寄存器
        fprintf(F, "li %s, %d\n", Regs[reg_no].name, op->u.value);
        return reg_no;
    }
    else if(op->kind==VAR) //变量
    {
        VarInfo var = check_var(op->u.name, 4);
        if(var->in_reg==false) //不在寄存器中，申请寄存器并放回
        {
            int reg_no = reg_allocate(var);
            // 将内容读入寄存器
            fprintf(F,"lw %s, %d($fp)\n",Regs[reg_no].name, var->offset);
            var->in_reg = true;
            var->reg_id = reg_no;
            return reg_no;
        }
        else //在寄存器中，则返回寄存器编号
        {
            return var->reg_id;
        }
    }
    else if(op->kind==ADDR) //地址
    {
        VarInfo var = check_var(op->u.name, 4);
        int reg_no = reg_allocate(NULL);
        fprintf(F,"addi %s, $fp, %d\n",Regs[reg_no].name, var->offset);
        return reg_no;
    }
    else if(op->kind==MYSTAR)
    {
        VarInfo var = check_var(op->u.name, 4);
        int reg_no;
        if(var->in_reg==false) //不在寄存器中，申请寄存器并放回
        {
            reg_no = reg_allocate(var);
            fprintf(F,"lw %s, %d($fp)\n",Regs[reg_no].name, var->offset);
            var->in_reg = true;
            var->reg_id = reg_no;
        }
        else //在寄存器中，则返回寄存器编号
        {
            reg_no = var->reg_id;
        }
        int ret = reg_allocate(NULL);
        fprintf(F,"lw %s, 0(%s)\n", Regs[ret].name, Regs[reg_no].name);
        return ret;
    }
}
void init_regs()
{
    strcpy(Regs[0].name,"$zero");
    strcpy(Regs[1].name,"$at");
    strcpy(Regs[2].name,"$v0");
    strcpy(Regs[3].name,"$v1");
    strcpy(Regs[4].name,"$a0");
    strcpy(Regs[5].name,"$a1");
    strcpy(Regs[6].name,"$a2");
    strcpy(Regs[7].name,"$a3");
    strcpy(Regs[8].name,"$t0");
    strcpy(Regs[9].name,"$t1");
    strcpy(Regs[10].name,"$t2");
    strcpy(Regs[11].name,"$t3");
    strcpy(Regs[12].name,"$t4");
    strcpy(Regs[13].name,"$t5");
    strcpy(Regs[14].name,"$t6");
    strcpy(Regs[15].name,"$t7");
    strcpy(Regs[16].name,"$s0");
    strcpy(Regs[17].name,"$s1");
    strcpy(Regs[18].name,"$s2");
    strcpy(Regs[19].name,"$s3");
    strcpy(Regs[20].name,"$s4");
    strcpy(Regs[21].name,"$s5");
    strcpy(Regs[22].name,"$s6");
    strcpy(Regs[23].name,"$s7");
    strcpy(Regs[24].name,"$t8");
    strcpy(Regs[25].name,"$t9");
    strcpy(Regs[26].name,"$k0");
    strcpy(Regs[27].name,"$k1");
    strcpy(Regs[28].name,"$gp");
    strcpy(Regs[29].name,"$sp");
    strcpy(Regs[30].name,"$fp");
    strcpy(Regs[31].name,"$ra");
    for(int i=1;i<32;i++)
    {
        Regs[i].is_free=true;
        Regs[i].var=NULL;
    }
    Regs[0].is_free = false;
    Regs[0].var = NULL;
}
void init_final()
{
    fprintf(F,".data\n");
    fprintf(F,"_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(F,"_ret: .asciiz \"\\n\"\n");
    fprintf(F,".globl main\n");
    fprintf(F,".text\n");
    fprintf(F,"read:\n");
    fprintf(F,"li $v0, 4\n");
    fprintf(F,"la $a0, _prompt\n");
    fprintf(F,"syscall\n");
    fprintf(F,"li $v0, 5\n");
    fprintf(F,"syscall\n");
    fprintf(F,"jr $ra\n");

    fprintf(F,"\n");
    fprintf(F,"write:\n");
    fprintf(F,"li $v0, 1\n");
    fprintf(F,"syscall\n");
    fprintf(F,"li $v0, 4\n");
    fprintf(F,"la $a0, _ret\n");
    fprintf(F,"syscall\n");
    fprintf(F,"move $v0, $0\n");
    fprintf(F,"jr $ra\n\n");
}
void save_regs()
{
    for(int i = 8;i<26;i++)
    {
        if(Regs[i].is_free==false)
        {
            // assert(Regs[i].var!=NULL);
            if(Regs[i].var!=NULL)
            {
                fprintf(F, "sw %s, %d($fp)\n", Regs[i].name, Regs[i].var->offset);
                Regs[i].var->in_reg = false;
            }
            
            Regs[i].is_free = true;
        }
    }
}
void trans_final(InterCode code, FILE* file)
{
    InterCode now = code;
    int index;
    F=file;
    printIR(code, NULL);
    init_regs();
    init_final();
    while(now!=NULL)
    {
        
        if(now->kind==IR_LABEL)
        {
             //基本块入口，将寄存器中所有须保存内容写入内存
            fprintf(F, "%s:\n", now->u.singleOp.op->u.name);
        }
        else if(now->kind==IR_FUNCTION) //函数开头，进行栈的一系列操作
        {
            fprintf(F,"\n%s:\n", now->u.singleOp.op->u.name);
            fprintf(F,"sw $fp, -8($sp)\n");
            fprintf(F,"sw $ra, -4($sp)\n");
            fprintf(F,"move $fp, $sp\n");
            fprintf(F,"addi $sp, $sp, -8\n");
            index = 0;
            var_off = -8;
            clr_lable();
            clr_var();
            printf("active variable analysis start\n");
            active_Analysis(now); //活跃变量分析
            printf("active variable analysis finished\n");
            fprintf(F,"addi $sp, $sp, %d\n", var_off+8);
            for(int i = 8;i<26;i++)
            {
                Regs[i].is_free = true;
                Regs[i].var = NULL;
                Regs[i].use = false;
            }
        }
        else if(now->kind==IR_ASSIGN)
        {
            Operand op1 = now->u.assign.left;
            VarInfo var1 = check_var(op1->u.name, 4);
            int op1_reg;
            // Regs[op1_reg].use = true;
            Operand op2 = now->u.assign.right;
            if(op2->kind==CONSTANT) //常量无需分配寄存器
            {
                op1_reg = get_reg(op1);
                fprintf(F, "li %s, %d\n", Regs[op1_reg].name, op2->u.value);
            }
            else 
            {
                VarInfo var2 = check_var(op2->u.name, 4);
                int op2_reg = get_reg(op2);
                op1_reg = get_reg(op1);
                fprintf(F, "move %s, %s\n", Regs[op1_reg].name, Regs[op2_reg].name);
                if(bit_get(&out[index], var2->num)==0 || op2->kind!=VAR)
                {
                    reg_free(op2_reg);
                }
            }
            if(op1->kind==MYSTAR) //如果op1是指针解引用，则需要写回并直接free寄存器
            {
                int reg_no;
                if(var1->in_reg==false) //不在寄存器中，申请寄存器并放回
                {
                    reg_no = reg_allocate(var1);
                    fprintf(F,"lw %s, %d($fp)\n",Regs[reg_no].name, var1->offset);
                    var1->in_reg = true;
                    var1->reg_id = reg_no;
                }
                else //在寄存器中，则返回寄存器编号
                {
                    reg_no = var1->reg_id;
                }
                fprintf(F,"sw %s, 0(%s)\n", Regs[op1_reg].name, Regs[reg_no].name);
                reg_free(op1_reg);
            }
        }
        else if(now->kind==IR_ADD)
        {
            Operand result = now->u.binOp.result;
            Operand op1 = now->u.binOp.op1;
            Operand op2 = now->u.binOp.op2;
            int op1_reg = get_reg(op1);
            int result_reg;
            if(op2->kind==CONSTANT)
            {
                result_reg = get_reg(result);
                fprintf(F, "addi %s, %s, %d\n", Regs[result_reg].name, Regs[op1_reg].name, op2->u.value);
            }
            else
            {
                Regs[op1_reg].use = true;
                VarInfo var2 = check_var(op2->u.name, 4);
                int op2_reg = get_reg(op2);
                Regs[op1_reg].use = false;
                result_reg = get_reg(result);
                fprintf(F, "add %s, %s, %s\n", Regs[result_reg].name, Regs[op1_reg].name, Regs[op2_reg].name);
                if(op2->kind!=VAR || bit_get(&out[index], var2->num)==0)
                {
                    reg_free(op2_reg);
                }
            }
            if(op1->kind==CONSTANT)
            {
                reg_free(op1_reg);
            }
            else
            {
                VarInfo var1 = check_var(op1->u.name, 4);
                if(op1->kind!=VAR || bit_get(&out[index], var1->num)==0)
                {
                    reg_free(op1_reg);
                }
            }
        }
        else if(now->kind==IR_SUB)
        {
            Operand result = now->u.binOp.result;
            Operand op1 = now->u.binOp.op1;
            Operand op2 = now->u.binOp.op2;
            int op1_reg = get_reg(op1);
            int result_reg;
            if(op2->kind==CONSTANT)
            {
                result_reg = get_reg(result);
                fprintf(F, "addi %s, %s, %d\n", Regs[result_reg].name, Regs[op1_reg].name, -op2->u.value);
            }
            else
            {
                Regs[op1_reg].use = true;
                VarInfo var2 = check_var(op2->u.name, 4);
                int op2_reg = get_reg(op2);
                Regs[op1_reg].use = false;
                result_reg = get_reg(result);
                fprintf(F, "sub %s, %s, %s\n", Regs[result_reg].name, Regs[op1_reg].name, Regs[op2_reg].name);
                if(op2->kind!=VAR || bit_get(&out[index], var2->num)==0)
                {
                    reg_free(op2_reg);
                }
            }
            if(op1->kind==CONSTANT)
            {
                reg_free(op1_reg);
            }
            else
            {
                VarInfo var1 = check_var(op1->u.name, 4);
                if(op1->kind!=VAR || bit_get(&out[index], var1->num)==0)
                {
                    reg_free(op1_reg);
                }
            }
        }
        else if(now->kind==IR_MUL)
        {
            Operand result = now->u.binOp.result;
            Operand op1 = now->u.binOp.op1;
            Operand op2 = now->u.binOp.op2;
            int op1_reg = get_reg(op1);
            Regs[op1_reg].use = true;
            int op2_reg = get_reg(op2);
            Regs[op1_reg].use = false;
            int result_reg = get_reg(result);
            fprintf(F, "mul %s, %s, %s\n", Regs[result_reg].name, Regs[op1_reg].name, Regs[op2_reg].name);
            if(op1->kind==CONSTANT)
            {
                reg_free(op1_reg);
            }
            else
            {
                VarInfo var1 = check_var(op1->u.name, 4);
                if(op1->kind!=VAR || bit_get(&out[index], var1->num)==0)
                {
                    reg_free(op1_reg);
                }
            }
            if(op2->kind==CONSTANT)
            {
                reg_free(op2_reg);
            }
            else
            {
                VarInfo var2 = check_var(op2->u.name, 4);
                if(op2->kind!=VAR || bit_get(&out[index], var2->num)==0)
                {
                    reg_free(op2_reg);
                }
            }
        }
        else if(now->kind==IR_DIV)
        {
            Operand result = now->u.binOp.result;
            Operand op1 = now->u.binOp.op1;
            Operand op2 = now->u.binOp.op2;
            int op1_reg = get_reg(op1);
            Regs[op1_reg].use = true;
            int op2_reg = get_reg(op2);
            Regs[op1_reg].use = false;
            int result_reg = get_reg(result);
            fprintf(F, "div %s, %s\n", Regs[op1_reg].name, Regs[op2_reg].name);
            fprintf(F, "mflo %s\n", Regs[result_reg].name);
            if(op1->kind==CONSTANT)
            {
                reg_free(op1_reg);
            }
            else
            {
                VarInfo var1 = check_var(op1->u.name, 4);
                if(op1->kind!=VAR || bit_get(&out[index], var1->num)==0)
                {
                    reg_free(op1_reg);
                }
            }
            if(op2->kind==CONSTANT)
            {
                reg_free(op2_reg);
            }
            else
            {
                VarInfo var2 = check_var(op2->u.name, 4);
                if(op2->kind!=VAR || bit_get(&out[index], var2->num)==0)
                {
                    reg_free(op2_reg);
                }
            }
        }
        else if(now->kind==IR_GOTO)
        {
            save_regs();
            fprintf(F, "j %s\n", now->u.singleOp.op->u.name);
        }
        else if(now->kind==IR_IF_GOTO)
        {
            Operand op1=now->u.ifgoto.x;
            Operand op2=now->u.ifgoto.y;
            Operand label=now->u.ifgoto.label;
            char relop[32];
            strcpy(relop, now->u.ifgoto.relop);
            int op1_reg = get_reg(op1);
            Regs[op1_reg].use = true;
            int op2_reg = get_reg(op2);
            Regs[op1_reg].use = false;
            save_regs();
            if(strcmp(relop,"==")==0)
            {
                fprintf(F,"beq %s, %s, %s\n",Regs[op1_reg].name, Regs[op2_reg].name,label->u.name);
            }
            else if(strcmp(relop,"!=")==0)
            {
                fprintf(F,"bne %s, %s, %s\n",Regs[op1_reg].name, Regs[op2_reg].name,label->u.name);
            }
            else if(strcmp(relop,">")==0)
            {
                fprintf(F,"bgt %s, %s, %s\n",Regs[op1_reg].name, Regs[op2_reg].name,label->u.name);
            }
            else if(strcmp(relop,"<")==0)
            {
                fprintf(F,"blt %s, %s, %s\n",Regs[op1_reg].name, Regs[op2_reg].name,label->u.name);
            }
            else if(strcmp(relop,">=")==0)
            {
                fprintf(F,"bge %s, %s, %s\n",Regs[op1_reg].name, Regs[op2_reg].name,label->u.name);
            }
            else if(strcmp(relop,"<=")==0)
            {
                fprintf(F,"ble %s, %s, %s\n",Regs[op1_reg].name, Regs[op2_reg].name,label->u.name);
            }
            /*if(op1->kind==CONSTANT)
            {
                reg_free(op1_reg);
            }
            else
            {
                VarInfo var1 = check_var(op1->u.name, 4);
                if(op1->kind!=VAR || bit_get(&out[index], var1->num)==0)
                {
                    reg_free(op1_reg);
                }
            }
            if(op2->kind==CONSTANT)
            {
                reg_free(op2_reg);
            }
            else
            {
                VarInfo var2 = check_var(op2->u.name, 4);
                if(op2->kind!=VAR || bit_get(&out[index], var2->num)==0)
                {
                    reg_free(op2_reg);
                }
            }*/
        }
        else if(now->kind==IR_RETURN)
        {
            Operand op1 = now->u.assign.left;
            Operand op2 = now->u.assign.right;
            int reg_no = get_reg(op1);
            fprintf(F,"move $v0, %s\n", Regs[reg_no].name);
            fprintf(F,"jr $ra\n");
            fprintf(F,"lw $ra, -4($fp)\n");
            fprintf(F,"move $sp, $fp\n");
            fprintf(F, "lw $fp, -8($fp)\n");
        }
        else if(now->kind==IR_DEC) //无需做任何处理
        {
            ;
        }
        else if(now->kind==IR_PARAM)
        {
            int count = 0;
            VarInfo var;
            do
            {
                var = check_var(now->u.singleOp.op->u.name, 4);
                if(count<4)
                {
                    fprintf(F, "sw %s, %d($fp)\n", Regs[4+count].name, var->offset);
                }
                else 
                {
                    int reg_no = get_reg(now->u.singleOp.op);
                    fprintf(F, "lw %s, %d($fp)\n", Regs[reg_no].name, 4*(count-4));
                }
                count++;
                now = now->next;
                index++;
            }while(now->kind==IR_PARAM);
            now = now->pre;
        }
        else if(now->kind==IR_CALL)
        {
            save_regs();
            Operand op1 = now->u.assign.left;
            Operand op2 = now->u.assign.right;
            fprintf(F,"jal %s\n",op2->u.name);
            int reg_no = get_reg(op1);
            fprintf(F,"move %s, $v0\n", Regs[reg_no].name);
        }
        else if(now->kind==IR_ARG)
        {
            int count = 0;
            VarInfo var;
            InterCode tmp = now;
            while(tmp->kind==IR_ARG) //计算变量总数
            {
                count++;
                tmp = tmp->next;
            }
            count = count-1;
            do
            {
                Operand op = now->u.singleOp.op;
                int reg_no = get_reg(op);
                if(count<4)
                {
                    fprintf(F, "move %s, %s\n", Regs[4+count].name, Regs[reg_no].name);
                }
                else 
                {
                    fprintf(F, "addi $sp, $sp, -4\n"); //改变$sp的值
                    fprintf(F, "sw %s, 0($sp)\n", Regs[reg_no].name);
                }
                if(op->kind==CONSTANT)
                {
                    reg_free(reg_no);
                }
                else
                {
                    VarInfo var = check_var(op->u.name, 4);
                    if(op->kind!=VAR || bit_get(&out[index], var->num)==0)
                    {
                        reg_free(reg_no);
                    }
                }
                count--;
                now = now->next;
                index++;
            }while(now->kind==IR_ARG);
            now = now->pre;
        }
        else if(now->kind==IR_READ)
        {
            fprintf(F,"addi $sp, $sp, -4\n");
            fprintf(F,"sw $ra, 0($sp)\n");
            fprintf(F,"jal read\n");
            fprintf(F,"lw $ra, 0($sp)\n");
            fprintf(F,"addi $sp, $sp, 4\n");
            int reg_no =get_reg(now->u.singleOp.op);
            fprintf(F,"move %s, $v0\n",Regs[reg_no].name);
        }
        else if(now->kind==IR_WRITE)
        {
            Operand op = now->u.singleOp.op;
            int reg_no =get_reg(op);
            fprintf(F,"move $a0, %s\n",Regs[reg_no].name);
            fprintf(F,"addi $sp, $sp, -4\n");
            fprintf(F,"sw $ra, 0($sp)\n");
            fprintf(F,"jal write\n");
            fprintf(F,"lw $ra, 0($sp)\n");
            fprintf(F,"addi $sp, $sp, 4\n");
            if(op->kind==CONSTANT)
            {
                reg_free(reg_no);
            }
            else
            {
                VarInfo var = check_var(op->u.name, 4);
                if(op->kind!=VAR || bit_get(&out[index], var->num)==0)
                {
                    reg_free(reg_no);
                }
            }
        }
        if(now->next!=NULL&&now->next->kind==IR_LABEL)
        {
            save_regs();
        }
        now = now->next;
        index++;
    }
}
