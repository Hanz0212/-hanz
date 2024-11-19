struct CodeMips : Mips
{
    RegPtr rs, rt, rd;
    int intermediate;
    string labelName;

    CodeMips(mips_type mipsType, RegPtr rs, RegPtr rt, RegPtr rd, int intermediate,
             string labelName) : Mips(mipsType)
    {
        this->rs = rs;
        this->rt = rt;
        this->rd = rd;
        this->intermediate = intermediate;
        this->labelName = labelName;
    }
};

struct RTypeMips : CodeMips // 三寄存器运算
{
    RTypeMips(mips_type mipsType, RegPtr rs, RegPtr rt, RegPtr rd)
        : CodeMips(mipsType, rs, rt, rd, -1, "") {}

    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType),
                                rd->getStr() + ",",
                                rs->getStr() + ",",
                                rt->getStr()});
    }
};

// / MFHI MHLO
struct MFTypeMips : RTypeMips
{
    MFTypeMips(bool isMfhi, RegPtr reg)
        : RTypeMips(isMfhi ? MFHI_OP : MFLO_OP, NULL, NULL, rd) {}

    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType), rd->getStr()});
    }
};

// 乘法 除法指令
struct MDTypeMips : RTypeMips
{
    MDTypeMips(bool isDiv, RegPtr rs, RegPtr rt)
        : RTypeMips(isDiv ? DIV_OP : MULT_OP, rs, rt, NULL) {}

    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType),
                                rs->getStr() + ",",
                                rt->getStr()});
    }
};

// rt <- rs + intermediate
struct ITypeMips : CodeMips
{
    // // rt <- rs + intermediate
    ITypeMips(mips_type mipsType, RegPtr rt, RegPtr rs, int intermediate)
        : CodeMips(mipsType, rs, rt, NULL, intermediate, "") {}
    
    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType),
                                rt->getStr() + ",",
                                rs->getStr() + ",",
                                to_string(intermediate)});
    }

};

// rt -> intermediate(rs)
struct StoreMips : ITypeMips
{
    bool isInt;
    // rt -> intermediate(rs)
    StoreMips(RegPtr rt, int intermediate, bool isInt)
        : ITypeMips(STORE_OP, rt, manager->sp, intermediate) {}

    string toString() override
    {
        return "\t" + join_str({isInt ? "sw" : "sb", rt->getStr()+",", to_string(intermediate) + "(" + rs->getStr() + ")"});
    }
};

// rt <- intermediate(rs)
struct LoadMips : ITypeMips
{
    bool isInt;
    // rt <- intermediate(rs)
    LoadMips(RegPtr rt, int intermediate, bool isInt)
        : ITypeMips(LOAD_OP, rt, manager->sp, intermediate), isInt(isInt) {}

    string toString() override
    {
        return "\t" + join_str({isInt ? "lw" : "lb", rt->getStr()+",", to_string(intermediate) + "(" + rs->getStr() + ")"});
    }
};


struct JTypeMips : CodeMips
{
};

struct LiMips : CodeMips
{
    LiMips(RegPtr rt, int intermediate) 
    : CodeMips(LI_OP, NULL, rt, NULL, intermediate, "") {}

    string toString() override
    {
        return "\t" + join_str({"li", rt->getStr()+",", to_string(intermediate)});
    }
};

struct LabelMips : CodeMips
{
    LabelMips(string labelName)
        : CodeMips(LABEL_OP, NULL, NULL, NULL, -1, labelName) {}

    string toString() override
    {
        return labelName + ":";
    }
};
