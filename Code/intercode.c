#include"intercode.h"
#include"tree.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

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
    // print_test(code);
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
    strcpy(ret->u.name, "label");
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
void printOperand(Operand op, FILE* file)
{
    if (file == NULL)
    {
        if (op->kind == CONSTANT)
        {
            printf("#%d", op->u.value);
        }
        else if (op->kind == ADDR)
        {
            printf("&%s", op->u.name);
        }
        else if (op->kind == MYSTAR)
        {
            printf("*%s", op->u.name);
        }
        else
            printf("%s", op->u.name);
    }
    else
    {
        if (op->kind == CONSTANT)
        {
            fprintf(file, "#%d", op->u.value);
        }
        else if (op->kind == ADDR)
        {
            fprintf(file, "&%s", op->u.name);
        }
        else if (op->kind == MYSTAR)
        {
            fprintf(file, "*%s", op->u.name);
        }
        else
            fprintf(file, "%s", op->u.name);
    }
}
void printIR(InterCode head, FILE* file)
{
    if(file == NULL)
    {
        while (head != NULL)
        {
            if (head->kind == IR_LABEL)
            {
                printf("LABEL ");
                printOperand(head->u.singleOp.op, file);
                printf(" :\n");
            }
            else if (head->kind == IR_FUNCTION)
            {
                printf("FUNCTION ");
                printOperand(head->u.singleOp.op, file);
                printf(" :\n");
            }
            else if (head->kind == IR_ASSIGN)
            {
                printOperand(head->u.assign.left, file);
                printf(" := ");
                printOperand(head->u.assign.right, file);
                printf("\n");
            }
            else if (head->kind == IR_ADD)
            {
                printOperand(head->u.binOp.result, file);
                printf(" := ");
                printOperand(head->u.binOp.op1, file);
                printf(" + ");
                printOperand(head->u.binOp.op2, file);
                printf("\n");
            }
            else if (head->kind == IR_SUB)
            {
                printOperand(head->u.binOp.result, file);
                printf(" := ");
                printOperand(head->u.binOp.op1, file);
                printf(" - ");
                printOperand(head->u.binOp.op2, file);
                printf("\n");
            }
            else if (head->kind == IR_MUL)
            {
                printOperand(head->u.binOp.result, file);
                printf(" := ");
                printOperand(head->u.binOp.op1, file);
                printf(" * ");
                printOperand(head->u.binOp.op2, file);
                printf("\n");
            }
            else if (head->kind == IR_DIV)
            {
                printOperand(head->u.binOp.result, file);
                printf(" := ");
                printOperand(head->u.binOp.op1, file);
                printf(" / ");
                printOperand(head->u.binOp.op2, file);
                printf("\n");
            }
            else if (head->kind == IR_GET_ADDR)
            {
                printOperand(head->u.assign.left, file);
                printf(" := &");
                printOperand(head->u.assign.right, file);
                printf("\n");
            }
            else if (head->kind == IR_READ_ADDR)
            {
                printOperand(head->u.assign.left, file);
                printf(" := *");
                printOperand(head->u.assign.right, file);
                printf("\n");
            }
            else if (head->kind == IR_WRITE_ADDR)
            {
                printf("*");
                printOperand(head->u.assign.left, file);
                printf(" := ");
                printOperand(head->u.assign.right, file);
                printf("\n");
            }
            else if (head->kind == IR_GOTO)
            {
                printf("GOTO ");
                printOperand(head->u.singleOp.op, file);
                printf("\n");
            }
            else if (head->kind == IR_IF_GOTO)
            {
                printf("IF ");
                printOperand(head->u.ifgoto.x, file);
                printf(" %s ", head->u.ifgoto.relop);
                printOperand(head->u.ifgoto.y, file);
                printf(" GOTO ");
                printOperand(head->u.ifgoto.label, file);
                printf("\n");
            }
            else if (head->kind == IR_RETURN)
            {
                printf("RETURN ");
                printOperand(head->u.singleOp.op, file);
                printf("\n");
            }
            else if (head->kind == IR_DEC)
            {
                printf("DEC ");
                printOperand(head->u.dec.op, file);
                printf(" %d\n", head->u.dec.size);
            }
            else if (head->kind == IR_ARG)
            {
                printf("ARG ");
                printOperand(head->u.singleOp.op, file);
                printf("\n");
            }
            else if (head->kind == IR_CALL)
            {
                printOperand(head->u.assign.left, file);
                printf(" := CALL ");
                printOperand(head->u.assign.right, file);
                printf("\n");
            }
            else if (head->kind == IR_PARAM)
            {
                printf("PARAM ");
                printOperand(head->u.singleOp.op, file);
                printf("\n");
            }
            else if (head->kind == IR_READ)
            {
                printf("READ ");
                printOperand(head->u.singleOp.op, file);
                printf("\n");
            }
            else if (head->kind == IR_WRITE)
            {
                printf("WRITE ");
                printOperand(head->u.singleOp.op, file);
                printf("\n");
            }
            head = head->next;
        }
    }
    else 
    {
        while (head != NULL)
        {
            if (head->kind == IR_LABEL)
            {
                fprintf(file, "LABEL ");
                printOperand(head->u.singleOp.op, file);
                fprintf(file, " :\n");
            }
            else if (head->kind == IR_FUNCTION)
            {
                fprintf(file, "FUNCTION ");
                printOperand(head->u.singleOp.op, file);
                fprintf(file, " :\n");
            }
            else if (head->kind == IR_ASSIGN)
            {
                printOperand(head->u.assign.left, file);
                fprintf(file, " := ");
                printOperand(head->u.assign.right, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_ADD)
            {
                printOperand(head->u.binOp.result, file);
                fprintf(file, " := ");
                printOperand(head->u.binOp.op1, file);
                fprintf(file, " + ");
                printOperand(head->u.binOp.op2, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_SUB)
            {
                printOperand(head->u.binOp.result, file);
                fprintf(file, " := ");
                printOperand(head->u.binOp.op1, file);
                fprintf(file, " - ");
                printOperand(head->u.binOp.op2, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_MUL)
            {
                printOperand(head->u.binOp.result, file);
                fprintf(file, " := ");
                printOperand(head->u.binOp.op1, file);
                fprintf(file, " * ");
                printOperand(head->u.binOp.op2, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_DIV)
            {
                printOperand(head->u.binOp.result, file);
                fprintf(file, " := ");
                printOperand(head->u.binOp.op1, file);
                fprintf(file, " / ");
                printOperand(head->u.binOp.op2, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_GET_ADDR)
            {
                printOperand(head->u.assign.left, file);
                fprintf(file, " := &");
                printOperand(head->u.assign.right, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_READ_ADDR)
            {
                printOperand(head->u.assign.left, file);
                fprintf(file, " := *");
                printOperand(head->u.assign.right, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_WRITE_ADDR)
            {
                fprintf(file, "*");
                printOperand(head->u.assign.left, file);
                fprintf(file, " := ");
                printOperand(head->u.assign.right, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_GOTO)
            {
                fprintf(file, "GOTO ");
                printOperand(head->u.singleOp.op, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_IF_GOTO)
            {
                fprintf(file, "IF ");
                printOperand(head->u.ifgoto.x, file);
                fprintf(file, " %s ", head->u.ifgoto.relop);
                printOperand(head->u.ifgoto.y, file);
                fprintf(file, " GOTO ");
                printOperand(head->u.ifgoto.label, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_RETURN)
            {
                fprintf(file, "RETURN ");
                printOperand(head->u.singleOp.op, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_DEC)
            {
                fprintf(file, "DEC ");
                printOperand(head->u.dec.op, file);
                fprintf(file, " %d\n", head->u.dec.size);
            }
            else if (head->kind == IR_ARG)
            {
                fprintf(file, "ARG ");
                printOperand(head->u.singleOp.op, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_CALL)
            {
                printOperand(head->u.assign.left, file);
                fprintf(file, " := CALL ");
                printOperand(head->u.assign.right, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_PARAM)
            {
                fprintf(file, "PARAM ");
                printOperand(head->u.singleOp.op, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_READ)
            {
                fprintf(file, "READ ");
                printOperand(head->u.singleOp.op, file);
                fprintf(file, "\n");
            }
            else if (head->kind == IR_WRITE)
            {
                fprintf(file, "WRITE ");
                printOperand(head->u.singleOp.op, file);
                fprintf(file, "\n");
            }
            head = head->next;
        }
    }
}
void print_test(InterCode head, FILE* file)
{
    if (head->kind == IR_LABEL)
    {
        printf("LABLE ");
        printOperand(head->u.singleOp.op, file);
        printf(" :\n");
    }
    else if (head->kind == IR_FUNCTION)
    {
        printf("FUNCTION ");
        printOperand(head->u.singleOp.op, file);
        printf(" :\n");
    }
    else if (head->kind == IR_ASSIGN)
    {
        printOperand(head->u.assign.left, file);
        printf(" := ");
        printOperand(head->u.assign.right, file);
        printf("\n");
    }
    else if (head->kind == IR_ADD)
    {
        printOperand(head->u.binOp.result, file);
        printf(" := ");
        printOperand(head->u.binOp.op1, file);
        printf(" + ");
        printOperand(head->u.binOp.op2, file);
        printf("\n");
    }
    else if (head->kind == IR_SUB)
    {
        printOperand(head->u.binOp.result, file);
        printf(" := ");
        printOperand(head->u.binOp.op1, file);
        printf(" - ");
        printOperand(head->u.binOp.op2, file);
        printf("\n");
    }
    else if (head->kind == IR_MUL)
    {
        printOperand(head->u.binOp.result, file);
        printf(" := ");
        printOperand(head->u.binOp.op1, file);
        printf(" * ");
        printOperand(head->u.binOp.op2, file);
        printf("\n");
    }
    else if (head->kind == IR_DIV)
    {
        printOperand(head->u.binOp.result, file);
        printf(" := ");
        printOperand(head->u.binOp.op1, file);
        printf(" / ");
        printOperand(head->u.binOp.op2, file);
        printf("\n");
    }
    else if (head->kind == IR_GET_ADDR)
    {
        printOperand(head->u.assign.left, file);
        printf(" := &");
        printOperand(head->u.assign.right, file);
        printf("\n");
    }
    else if (head->kind == IR_READ_ADDR)
    {
        printOperand(head->u.assign.left, file);
        printf(" := *");
        printOperand(head->u.assign.right, file);
        printf("\n");
    }
    else if (head->kind == IR_WRITE_ADDR)
    {
        printf("*");
        printOperand(head->u.assign.left, file);
        printf(" := ");
        printOperand(head->u.assign.right, file);
        printf("\n");
    }
    else if (head->kind == IR_GOTO)
    {
        printf("GOTO ");
        printOperand(head->u.singleOp.op, file);
        printf("\n");
    }
    else if (head->kind == IR_IF_GOTO)
    {
        printf("IF ");
        printOperand(head->u.ifgoto.x, file);
        printf(" %s ", head->u.ifgoto.relop);
        printOperand(head->u.ifgoto.y, file);
        printf(" GOTO ");
        printOperand(head->u.ifgoto.label, file);
        printf("\n");
    }
    else if (head->kind == IR_RETURN)
    {
        printf("RETURN ");
        printOperand(head->u.singleOp.op, file);
        printf("\n");
    }
    else if (head->kind == IR_DEC)
    {
        printf("DEC ");
        printOperand(head->u.dec.op, file);
        printf(" %d\n", head->u.dec.size);
    }
    else if (head->kind == IR_ARG)
    {
        printf("ARG ");
        printOperand(head->u.singleOp.op, file);
        printf("\n");
    }
    else if (head->kind == IR_CALL)
    {
        printOperand(head->u.assign.left, file);
        printf(" := CALL ");
        printOperand(head->u.assign.right, file);
        printf("\n");
    }
    else if (head->kind == IR_PARAM)
    {
        printf("PARAM ");
        printOperand(head->u.singleOp.op, file);
        printf("\n");
    }
    else if (head->kind == IR_READ)
    {
        printf("READ ");
        printOperand(head->u.singleOp.op, file);
        printf("\n");
    }
    else if (head->kind == IR_WRITE)
    {
        printf("WRITE ");
        printOperand(head->u.singleOp.op, file);
        printf("\n");
    }
}