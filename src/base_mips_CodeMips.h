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
    RTypeMips(mips_type mipsType, RegPtr rd, RegPtr rs, RegPtr rt, string annotation)
        : CodeMips(mipsType, rs, rt, rd, NULL, NULL, annotation) {}

    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType), rd->getStr() + ",", rs->getStr() + ",", rt->getStr()}) + Mips::toString();
    }
};

// / MFHI MHLO
struct MFTypeMips : RTypeMips
{
    MFTypeMips(bool isMfhi, RegPtr rd, string annotation)
        : RTypeMips(isMfhi ? MFHI_OP : MFLO_OP, rd, NULL, NULL, annotation) {}

    string toString() override
    {
        return "\t" + join_str({mips_type_2_opstr.at(mipsType), rd->getStr()});
    }
};

// 乘法 除法指令
struct MDTypeMips : RTypeMips
{
    MDTypeMips(bool isDiv, RegPtr rs, RegPtr rt, string annotation)
        : RTypeMips(isDiv ? DIV_OP : MULT_OP, NULL, rs, rt, annotation) {}

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
    MEMTypeMips(mips_type mipsType, RegPtr rt, RegPtr desReg, string annotation)
        : ITypeMips(mipsType, rt, NULL, desReg, annotation) {}

    string toString() override
    {
        RegPtr desReg = intermediate;
        string opTk = mipsType == STORE_OP ? "sw" : "lw";
        string result;
        if (in32Reg(desReg->getType())) // 0($desReg)
            result = join_str({opTk, rt->getStr() + ",", "0(" + desReg->getStr() + ")"});
        else if (desReg->getType() == OFFSET) // val($sp)
            result = join_str({opTk, rt->getStr() + ",", desReg->getStr() + "($sp)"});
        else if (desReg->getType() == LABEL) // label
            result = join_str({opTk, rt->getStr() + ",", desReg->getStr()});
        else
            DIE("wrong fomat in LoadMips");
        result = "\t" + result + Mips::toString();
        return result;
    }
};

// rt -> desReg(label/offset($sp)/0($reg))
struct StoreMips : MEMTypeMips
{
    // rt -> desReg(label/offset($sp)/0($reg))
    StoreMips(RegPtr rt, RegPtr desReg, string annotation)
        : MEMTypeMips(STORE_OP, rt, desReg, annotation) {}
};

// rt <- desReg(label/offset($sp)/0($reg))
struct LoadMips : MEMTypeMips
{
    // rt <- desReg(label/offset($sp)/0($reg))
    LoadMips(RegPtr rt, RegPtr desReg, string annotation)
        : MEMTypeMips(LOAD_OP, rt, desReg, annotation) {}
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

struct LaMips : CodeMips
{
    // offset需为reg
    LaMips(RegPtr rt, RegPtr base, RegPtr offset, string annotation)
        : CodeMips(LA_OP, base, rt, NULL, offset, NULL, annotation)
    {
        if (base->getType() == OFFSET && !in32Reg(offset->getType()))
            DIE("in LaMips: Invalid <base,offset> type: <" +
                reg_type_2_str(base->getType()) + "," + reg_type_2_str(offset->getType()) + ">");
    }

    string toString() override
    {
        string result = "\t";
        RegPtr base = rs, offset = intermediate;
        if (base->getType() == LABEL && offset->getType() == INTERMEDIATE)
            result += join_str({"la", rt->getStr() + ",", base->getStr(), "+", offset->getStr()});
        else if (base->getType() == LABEL && in32Reg(offset->getType()))
            result += join_str({"la", rt->getStr() + ",", base->getStr() + "(" + offset->getStr() + ")"});
        else if (base->getType() == OFFSET)
            result += join_str({"la", rt->getStr() + ",", base->getStr() + "(" + offset->getStr() + ")"});
        else
            DIE("in LaMips toString invalid type");
        return result + Mips::toString();
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

struct AnnotationMips : CodeMips
{
    AnnotationMips(string annotation)
        : CodeMips(ANNOTATION_OP, NULL, NULL, NULL, NULL, NULL, annotation) {}
};
