#include"intercode.h"
#include"tree.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

InterCode IR_head;
InterCode IR_tail;
int temp_num;
int lable_num;
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
Operand new_temp()
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
    int j = 0;
    strcpy(ret->u.name, "tfh");
    while(j<i)
    {
        char c = str[j];
        str[j] = str[i];
        str[i] = c;
        j++;
        i--;
    }
    strcat(ret->u.name, str);
    return ret;
}
Operand new_lable()
{
    lable_num++;
    Operand ret = (Operand)malloc(sizeof(struct Operand_));
    ret->kind = LABEL;
    int tmp = lable_num;
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
    int j = 0;
    strcpy(ret->u.name, "lable");
    while(j<i)
    {
        char c = str[j];
        str[j] = str[i];
        str[i] = c;
        j++;
        i--;
    }
    strcat(ret->u.name, str);
    return ret;
}
void printOperand(Operand op)
{
    if(op->kind==CONSTANT)
    {
        printf("#%d", op->u.value);
    }
    else if(op->kind==ADDR)
    {
        printf("&%s", op->u.name);
    }
    else if(op->kind==MYSTAR)
    {
        printf("*%s", op->u.name);
    }
    else
        printf("%s", op->u.name);
}
void printIR(InterCode head)
{
    while(head!=NULL)
    {
        if(head->kind==IR_LABEL)
        {
            printf("LABLE ");
            printOperand(head->u.singleOp.op);
            printf(" :\n");
        }
        else if(head->kind==IR_FUNCTION)
        {
            printf("FUNCTION ");
            printOperand(head->u.singleOp.op);
            printf(" :\n");
        }
        else if(head->kind==IR_ASSIGN)
        {
            printOperand(head->u.assign.left);
            printf(" := ");
            printOperand(head->u.assign.right);
            printf("\n");
        }
        else if(head->kind==IR_ADD)
        {
            printOperand(head->u.binOp.result);
            printf(" := ");
            printOperand(head->u.binOp.op1);
            printf(" + ");
            printOperand(head->u.binOp.op2);
            printf("\n");
        }
        else if(head->kind==IR_SUB)
        {
            printOperand(head->u.binOp.result);
            printf(" := ");
            printOperand(head->u.binOp.op1);
            printf(" - ");
            printOperand(head->u.binOp.op2);
            printf("\n");
        }
        else if(head->kind==IR_MUL)
        {
            printOperand(head->u.binOp.result);
            printf(" := ");
            printOperand(head->u.binOp.op1);
            printf(" * ");
            printOperand(head->u.binOp.op2);
            printf("\n");
        }
        else if(head->kind==IR_DIV)
        {
            printOperand(head->u.binOp.result);
            printf(" := ");
            printOperand(head->u.binOp.op1);
            printf(" / ");
            printOperand(head->u.binOp.op2);
            printf("\n");
        }
        else if(head->kind==IR_GET_ADDR)
        {
            printOperand(head->u.assign.left);
            printf(" := &");
            printOperand(head->u.assign.right);
            printf("\n");
        }
        else if(head->kind==IR_READ_ADDR)
        {
            printOperand(head->u.assign.left);
            printf(" := *");
            printOperand(head->u.assign.right);
            printf("\n");
        }
        else if(head->kind==IR_WRITE_ADDR)
        {
            printf("*");
            printOperand(head->u.assign.left);
            printf(" := ");
            printOperand(head->u.assign.right);
            printf("\n");
        }
        else if(head->kind==IR_GOTO)
        {
            printf("GOTO ");
            printOperand(head->u.singleOp.op);
            printf("\n");
        }
        else if(head->kind==IR_IF_GOTO)
        {
            printf("IF ");
            printOperand(head->u.ifgoto.x);
            printf(" %s ", head->u.ifgoto.relop);
            printOperand(head->u.ifgoto.y);
            printf(" GOTO ");
            printOperand(head->u.ifgoto.label);
            printf("\n");
        }
        else if(head->kind==IR_RETURN)
        {
            printf("RETURN ");
            printOperand(head->u.singleOp.op);
            printf("\n");
        }
        else if(head->kind==IR_DEC)
        {
            printf("DEC ");
            printOperand(head->u.dec.op);
            printf(" [%d]\n", head->u.dec.size);
        }
        else if(head->kind==IR_ARG)
        {
            printf("ARG ");
            printOperand(head->u.singleOp.op);
            printf("\n");
        }
        else if(head->kind==IR_CALL)
        {
            printOperand(head->u.assign.left);
            printf(" := CALL ");
            printOperand(head->u.assign.right);
            printf("\n");
        }
        else if(head->kind==IR_PARAM)
        {
            printf("PARAM ");
            printOperand(head->u.singleOp.op);
            printf("\n");
        }
        else if(head->kind==IR_READ)
        {
            printf("READ ");
            printOperand(head->u.singleOp.op);
            printf("\n");
        }
        else if(head->kind==IR_WRITE)
        {
            printf("WRITE ");
            printOperand(head->u.singleOp.op);
            printf("\n");
        }
    }
}