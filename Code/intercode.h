#ifndef _INTERCODE_H
#define _INTERCODE_H

typedef struct Operand_* Operand; 
typedef struct InterCode_* InterCode;
typedef struct ArgList_* ArgList;
struct Operand_{
    enum{VAR, CONSTANT, LABEL, FUNC, ADDR, MYSTAR, NONE}kind;
    union{
		int value;
		char name[32];
	}u;
};
struct InterCode_
{
    enum{
        IR_LABEL, 
        IR_FUNCTION, 
        IR_ASSIGN, 
        IR_ADD, 
        IR_SUB, 
        IR_MUL, 
        IR_DIV, 
        IR_GET_ADDR, 
        IR_READ_ADDR, 
        IR_WRITE_ADDR, 
        IR_GOTO, 
        IR_IF_GOTO, 
        IR_RETURN, 
        IR_DEC, 
        IR_ARG, 
        IR_CALL, 
        IR_PARAM, 
        IR_READ, 
        IR_WRITE
    }kind;
    union{
		struct{	Operand op;	}singleOp;
		struct{ Operand op1,op2,result; }binOp;
		struct{ Operand x; Operand y; Operand label; char relop[32]; }ifgoto;
        struct{ Operand left,right; }assign;
		struct{ Operand op; int size; }dec;
	}u;
	InterCode pre;
	InterCode next;
};
struct ArgList_{
    Operand arg;
    ArgList next;
};
void IR_append(InterCode code);
Operand new_temp();
Operand new_lable();
void printOperand(Operand op);
void printIR(InterCode head);

#endif