enum mips_type
{
    GLOBAL_DEF_OP,
    GLOBAL_STR_OP,
    LABEL_OP,

    LOAD_OP,
    STORE_OP,

    ADD_OP,
    ADDIU_OP,
    SUB_OP,
    MULT_OP,
    DIV_OP,
    SLT_OP, // 小于置1
    SLL_OP, // 逻辑左移
    SRA_OP, // 算数右移
    AND_OP,
    OR_OP,
    XOR_OP,
    NOR_OP,

    LI_OP, // 加载立即数到寄存器

    MFHI_OP,
    MFLO_OP,
};

const map<mips_type, string> mips_type_2_str = {
    {GLOBAL_DEF_OP, "GLOBAL_DEF_OP"},
    {GLOBAL_STR_OP, "GLOBAL_STR_OP"},
    {LOAD_OP, "LOAD_OP"},
    {STORE_OP, "STORE_OP"},

    {ADD_OP, "ADD_OP"},
    {ADDIU_OP, "ADDIU_OP"},
    {SUB_OP, "SUB_OP"},
    {MULT_OP, "MULT_OP"},
    {DIV_OP, "DIV_OP"},
    {SLT_OP, "SLT_OP"},
    {SLL_OP, "SLL_OP"},
    {SRA_OP, "SRA_OP"},
    {AND_OP, "AND_OP"},
    {OR_OP, "OR_OP"},
    {XOR_OP, "XOR_OP"},
    {NOR_OP, "NOR_OP"},
    {LI_OP, "LI_OP"},};

const map<mips_type, string> mips_type_2_opstr = {
    {ADD_OP, "add"},
    {ADDIU_OP, "addiu"},
    {SUB_OP, "sub"},
    {MULT_OP, "mult"},
    {DIV_OP, "div"},
    {SLT_OP, "slt"},
    {SLL_OP, "sll"},
    {SRA_OP, "sra"},
    {AND_OP, "ans"},
    {OR_OP, "or"},
    {XOR_OP, "xor"},
    {NOR_OP, "nor"},
    {LI_OP, "li"},

    {MFHI_OP, "mfhi"},
    {MFLO_OP, "mflo"},
};

const map<mid_type, mips_type> mid_type_2_mips_type = {
    {ADD_IR, ADD_OP},
    {SUB_IR, SUB_OP},
    {MUL_IR, MULT_OP},
    {SDIV_IR, DIV_OP},
    {SREM_IR, DIV_OP},
    {AND_IR, AND_OP},
    {OR_IR, OR_OP},
    // ICMP_IR, // 比较指令
};

enum reg_type
{
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
    OFFSET,
};

const vector<reg_type> temp_reg_types = {
    T0, T1, T2, T3, T4, T5, T6, T7, T8, T9};

const vector<reg_type> save_reg_types = {
    S0, S1, S2, S3, S4, S5, S6, S7};

string reg_type_2_str(reg_type reg) {  
    switch (reg) {  
        case ZERO:  
            return "$zero";  
        case AT:  
            return "$at";  
        case V0:  
            return "$v0";  
        case V1:  
            return "$v1";  
        case A0:  
            return "$a0";  
        case A1:  
            return "$a1";  
        case A2:  
            return "$a2";  
        case A3:  
            return "$a3";  
        case T0:  
            return "$t0";  
        case T1:  
            return "$t1";  
        case T2:  
            return "$t2";  
        case T3:  
            return "$t3";  
        case T4:  
            return "$t4";  
        case T5:  
            return "$t5";  
        case T6:  
            return "$t6";  
        case T7:  
            return "$t7";  
        case S0:  
            return "$s0";  
        case S1:  
            return "$s1";  
        case S2:  
            return "$s2";  
        case S3:  
            return "$s3";  
        case S4:  
            return "$s4";  
        case S5:  
            return "$s5";  
        case S6:  
            return "$s6";  
        case S7:  
            return "$s7";  
        case T8:  
            return "$t8";  
        case T9:  
            return "$t9";  
        case K0:  
            return "$k0";  
        case K1:  
            return "$k1";  
        case GP:  
            return "$gp";  
        case SP:  
            return "$sp";  
        case FP:  
            return "$fp";  
        case RA:  
            return "$ra";  
        default:  
            cout << "unknown_register:" + to_string(reg) << endl;
            exit(1);
    }  
}

// const map<tk_type, >
