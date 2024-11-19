struct CodeMips : Mips
{
    RegPtr rs, rt, rd, intermediate, labelName;

    CodeMips(mips_type mipsType, RegPtr rs, RegPtr rt, RegPtr rd, RegPtr intermediate,
             RegPtr labelName, string annotation) : Mips(mipsType, annotation)
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
    RTypeMips(mips_type mipsType, RegPtr rs, RegPtr rt, RegPtr rd, string annotation)
        : CodeMips(mipsType, rs, rt, rd, NULL, NULL, annotation) {}

    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType),
                                rd->getStr() + ",",
                                rs->getStr() + ",",
                                rt->getStr()}) + Mips::toString();
    }
};

// / MFHI MHLO
struct MFTypeMips : RTypeMips
{
    MFTypeMips(bool isMfhi, RegPtr rd, string annotation)
        : RTypeMips(isMfhi ? MFHI_OP : MFLO_OP, NULL, NULL, rd, annotation) {}

    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType), rd->getStr()});
    }
};

// 乘法 除法指令
struct MDTypeMips : RTypeMips
{
    MDTypeMips(bool isDiv, RegPtr rs, RegPtr rt, string annotation)
        : RTypeMips(isDiv ? DIV_OP : MULT_OP, rs, rt, NULL, annotation) {}

    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType),
                                rs->getStr() + ",",
                                rt->getStr()});
    }
};

// rt <- rs ? intermediate
struct ITypeMips : CodeMips
{
    // // rt <- rs + intermediate
    ITypeMips(mips_type mipsType, RegPtr rt, RegPtr rs, RegPtr intermediate, string annotation)
        : CodeMips(mipsType, rs, rt, NULL, intermediate, NULL, annotation) {}

    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType),
                                rt->getStr() + ",",
                                rs->getStr() + ",",
                                intermediate->getStr()});
    }
};

// lw sw
struct MEMTypeMips : ITypeMips
{
    MEMTypeMips(mips_type mipsType, RegPtr rt, RegPtr rs, RegPtr intermediate, string annotation)
        : ITypeMips(mipsType, rt, rs, intermediate, annotation) {}

    string toString() override
    {
        string opTk = mipsType == STORE_OP ? "sw" : "lw";
        string result;
        if (rs->getType() == LABEL && intermediate->getType() == OFFSET) // label + val
            result = join_str({opTk, rt->getStr() + ",", rs->getStr(), "+", intermediate->getStr()});
        else if (rs->getType() == LABEL && in32Reg(intermediate->getType())) // label($t)
            result = join_str({opTk, rt->getStr() + ",", rs->getStr() + "(" + intermediate->getStr() + ")"});
        else if (in32Reg(rs->getType()) && intermediate->getType() == OFFSET) // val($sp)
            result = join_str({opTk, rt->getStr() + ",", intermediate->getStr() + "(" + rs->getStr() + ")"});
        else if (intermediate->getType() == LABEL)
            result = join_str({opTk, rt->getStr() + ",", intermediate->getStr()});
        else
            DIE("wrong fomat in LoadMips");
        result = "\t" + result + Mips::toString();
        return result;
    }
};

// rt -> intermediate(rs)
struct StoreMips : MEMTypeMips
{
    // rt -> intermediate(rs)
    StoreMips(RegPtr rt, RegPtr rs, RegPtr intermediate, string annotation)
        : MEMTypeMips(STORE_OP, rt, rs, intermediate, annotation) {}
};

// rt <- intermediate(rs)
struct LoadMips : MEMTypeMips
{
    // rt <- intermediate(rs)
    LoadMips(RegPtr rt, RegPtr rs, RegPtr intermediate, string annotation)
        : MEMTypeMips(LOAD_OP, rt, rs, intermediate, annotation) {}
};

struct JTypeMips : CodeMips
{
};

struct LiMips : CodeMips
{
    LiMips(RegPtr rt, RegPtr intermediate, string annotation)
        : CodeMips(LI_OP, NULL, rt, NULL, intermediate, NULL, annotation) {}

    string toString() override
    {
        return "\t" + join_str({"li", rt->getStr() + ",", intermediate->getStr()});
    }
};

struct LabelMips : CodeMips
{
    LabelMips(RegPtr labelName, string annotation)
        : CodeMips(LABEL_OP, NULL, NULL, NULL, NULL, labelName, annotation) {}

    string toString() override
    {
        return labelName->getStr() + ":";
    }
};
