#ifndef _ASSEMBLY_H
#define _ASSEMBLY_H
#include"intercode.h"
#include <stdbool.h>
#define SHIFT 5
#define MASK 0x1F
#define VECNUM 2
typedef struct BitVector_* BitVector;
typedef struct Lable2num_* Lable2num;
typedef struct VarInfo_* VarInfo;
typedef struct RegInfo_* RegInfo;

enum RegName {
    ZERO,
    AT,
    V0,
    V1,
    A0,
    A1,
    A2,
    A3,
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    S0,
    S1,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    T8,
    T9,
    K0,
    K1,
    GP,
    SP,
    FP,
    RA,
};
struct BitVector_{
    int val[VECNUM];
};
struct Lable2num_{
    char lable_name[32];
    int num;
    Lable2num next;
};
struct VarInfo_{
    char var_name[32];
    int num;
    int reg_id;
    int offset; //变量如果在内存中，是在内存中的偏移量，如果在寄存器中是寄存器的编号
    bool in_reg;
    int last_use;
    VarInfo next;
}; // 变量描述符
struct RegInfo_{
    bool is_free;
    char name[32]; // 寄存器名
    VarInfo var; //储存的变量指针
    bool use; //用来保护正在被使用的寄存器不被溢出
}; //寄存器描述符


//位向量相关函数
void bit_set(BitVector vec, int i);
void bit_clr(BitVector vec, int i);
int bit_get(BitVector vec, int i);
void vec_clr(BitVector vec);
bool vec_cmp(BitVector vec1, BitVector vec2);
struct BitVector_ vec_and(BitVector vec1, BitVector vec2);
struct BitVector_ vec_or(BitVector vec1, BitVector vec2);
struct BitVector_ vec_div(BitVector vec1, BitVector vec2);

void active_Analysis(InterCode func);
VarInfo check_var(char* name, int size);
int check_lable(char* name);
void clr_var();
void clr_lable();

void reg_free(int id);
int reg_allocate(VarInfo var);
int get_reg(Operand op);
void init_regs();

void init_final();
void save_regs();
void trans_final(InterCode code, FILE* file);

#endif