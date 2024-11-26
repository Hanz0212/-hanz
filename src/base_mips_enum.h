enum mips_type
{
    GLOBAL_DEF_OP,
    GLOBAL_STR_OP,
    LABEL_OP,
    ANNOTATION_OP,

    LOAD_OP,
    STORE_OP,

    ADDU_OP,
    ADDIU_OP,
    SUBU_OP,
    MULT_OP,
    DIV_OP,
    SLT_OP,  // 小于置1
    SLTU_OP, // 无符号小于置1
    SLL_OP,  // 逻辑左移
    SRA_OP,  // 算数右移
    AND_OP,
    OR_OP,
    XOR_OP,
    NOR_OP,

    JAL_OP,
    J_OP,
    JR_OP,
    BGTZ_OP, // 大于零转移
    NOP_OP,
    SYSCALL_OP,

    ANDI_OP,
    LI_OP,    // 加载立即数到寄存器
    LA_OP,    // 加载地址到寄存器
    XORI_OP,  // 亦或立即数
    SLTIU_OP, // 无符号小于立即数置1

    MFHI_OP,
    MFLO_OP,
};

const map<mips_type, string> mips_type_2_str = {
    {GLOBAL_DEF_OP, "GLOBAL_DEF_OP"},
    {GLOBAL_STR_OP, "GLOBAL_STR_OP"},
    {LOAD_OP, "LOAD_OP"},
    {STORE_OP, "STORE_OP"},
    {ANNOTATION_OP, "ANNOTATION_OP"},

    {ADDU_OP, "ADDU_OP"},
    {ADDIU_OP, "ADDIU_OP"},
    {SUBU_OP, "SUBU_OP"},
    {MULT_OP, "MULT_OP"},
    {DIV_OP, "DIV_OP"},
    {SLT_OP, "SLT_OP"},
    {SLTU_OP, "SLTU_OP"},
    {SLL_OP, "SLL_OP"},
    {SRA_OP, "SRA_OP"},
    {AND_OP, "AND_OP"},
    {OR_OP, "OR_OP"},
    {XOR_OP, "XOR_OP"},
    {NOR_OP, "NOR_OP"},

    {JAL_OP, "JAL_OP"},
    {J_OP, "J_OP"},
    {JR_OP, "JR_OP"},
    {BGTZ_OP, "BGTZ_OP"},
    {NOP_OP, "NOP_OP"},
    {SYSCALL_OP, "SYSCALL_OP"},

    {ANDI_OP, "ANDI_OP"},
    {LI_OP, "LI_OP"},
    {LA_OP, "LA_OP"},
    {XORI_OP, "XORI_OP"},
    {SLTIU_OP, "SLTIU_OP"}};

const map<mips_type, string> mips_type_2_opstr = {
    {ADDU_OP, "addu"},
    {ADDIU_OP, "addiu"},
    {SUBU_OP, "subu"},
    {MULT_OP, "mult"},
    {DIV_OP, "div"},
    {SLT_OP, "slt"},
    {SLTU_OP, "sltu"},
    {SLL_OP, "sll"},
    {SRA_OP, "sra"},
    {AND_OP, "ans"},
    {OR_OP, "or"},
    {XOR_OP, "xor"},
    {NOR_OP, "nor"},

    {JAL_OP, "jal"},
    {J_OP, "j"},
    {JR_OP, "jr"},
    {BGTZ_OP, "bgtz"},
    {NOP_OP, "nop"},
    {SYSCALL_OP, "syscall"},

    {ANDI_OP, "andi"},
    {LI_OP, "li"},
    {LA_OP, "la"},
    {XORI_OP, "xori"},
    {SLTIU_OP, "sltiu"},

    {MFHI_OP, "mfhi"},
    {MFLO_OP, "mflo"},
};

const map<mid_type, mips_type> mid_type_2_mips_type = {
    {ADD_IR, ADDU_OP},
    {SUB_IR, SUBU_OP},
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
    INTERMEDIATE,
    LABEL,
    OFFSET,
};

bool in32Reg(reg_type reg)
{
    return reg < 32;
}

const vector<reg_type> save_reg_types = {
    S0, S1, S2, S3, S4, S5, S6, S7};

string reg_type_2_str(reg_type reg)
{
    switch (reg)
    {
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
    case OFFSET:
        return "$offset";
    case INTERMEDIATE:
        return "$intermediate";
    case LABEL:
        return "$label";
    default:
        DIE("unknown_register:" + to_string(reg));
        return "";
    }
}

// const map<tk_type, >
