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
        : CodeMips(mipsType, rs, rt, rd, NULL, NULL, annotation)
    {
    }

    string toString() override
    {
        if (mipsType == MULT_OP || mipsType == DIV_OP)
            return "\t" + join_str({mips_type_2_opstr.at(mipsType), rs->getStr() + ",", rt->getStr()});
        else
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
// struct MDTypeMips : RTypeMips
// {
//     MDTypeMips(bool isDiv, RegPtr rs, RegPtr rt, string annotation)
//         : RTypeMips(isDiv ? DIV_OP : MULT_OP, NULL, rs, rt, annotation) {}

//     string toString() override
//     {
//         return "\t" + join_str({mips_type_2_opstr.at(mipsType), rs->getStr() + ",", rt->getStr()});
//     }
// };

// rt <- rs ? intermediate
struct ITypeMips : CodeMips
{
    // // rt <- rs + intermediate
    ITypeMips(mips_type mipsType, RegPtr rt, RegPtr rs, RegPtr intermediate, string annotation)
        : CodeMips(mipsType, rs, rt, NULL, intermediate, NULL, annotation) {}

    string toString() override
    {
        if (!rt || !rs || !intermediate)
            DIE("in ITypeMips : nullptr :" + join_str({to_string(rt == 0), to_string(rs == 0), to_string(intermediate == 0)}));
        if (!in32Reg(rt->getType()) || !in32Reg(rs->getType()) || intermediate->getType() != INTERMEDIATE)
            DIE("wrong fomat in ITypeMips: <" + reg_type_2_str(rt->getType()) + "," +
                reg_type_2_str(rs->getType()) + "," +
                reg_type_2_str(intermediate->getType()) + ">");

        return "\t" + join_str({mips_type_2_opstr.at(mipsType), rt->getStr() + ",", rs->getStr() + ",", intermediate->getStr()}) + Mips::toString();
    }
};

// lw sw
struct MEMTypeMips : ITypeMips
{
    MEMTypeMips(mips_type mipsType, RegPtr rt, RegPtr desReg, string annotation)
        : ITypeMips(mipsType, rt, NULL, desReg, annotation)
    {
        toString();
    }

    string toString() override
    {
        RegPtr desReg = intermediate;
        string opTk = mipsType == STORE_OP ? "sw" : "lw";
        string result;

        if (!rt || !desReg)
            DIE("in StoreMips : nullptr :" + join_str({to_string(rt == 0), to_string(desReg == 0)}));

        if (in32Reg(rt->getType()) && in32Reg(desReg->getType())) // 0($desReg)
            result = join_str({opTk, rt->getStr() + ",", "0(" + desReg->getStr() + ")"});
        else if (in32Reg(rt->getType()) && desReg->getType() == OFFSET) // val($sp)
            result = join_str({opTk, rt->getStr() + ",", desReg->getStr() + "($sp)"});
        else if (in32Reg(rt->getType()) && desReg->getType() == LABEL) // label
            result = join_str({opTk, rt->getStr() + ",", desReg->getStr()});
        else
            DIE("wrong fomat in (Load/store)Mips: <" +
                reg_type_2_str(rt->getType()) + "," + reg_type_2_str(desReg->getType()) + ">");
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
    JTypeMips(mips_type mipsType, RegPtr labelName, string annotation)
        : CodeMips(mipsType, NULL, NULL, NULL, NULL, labelName, annotation) { }

    string toString() override
    {
        if (!labelName)
            DIE("in JTypeMips: labelName nullptr");
        if (labelName->getType() != LABEL)
            DIE("in JTypeMips: label error type: <" + reg_type_2_str(labelName->getType()) + ">");
        return "\t" + join_str({mips_type_2_opstr.at(mipsType), labelName->getStr()}) + Mips::toString();
    }
};

struct BTypeMips : CodeMips
{
    BTypeMips(mips_type mipsType, RegPtr op1, RegPtr op2, RegPtr labelName, string annotation)
        : CodeMips(mipsType, op1, op2, NULL, NULL, labelName, annotation) {}

    string toString() override
    {
        RegPtr op1 = rs, op2 = rt;
        if (op2 == NULL)
            return "\t" + join_str({mips_type_2_opstr.at(mipsType), op1->getStr() + ",", labelName->getStr()}) + Mips::toString();
        else
            return "\t" + join_str({mips_type_2_opstr.at(mipsType), op1->getStr() + ",", op2->getStr() + ",", labelName->getStr()}) + Mips::toString();
    }
};

// jr $ra
struct JRMips : JTypeMips
{
    JRMips() : JTypeMips(JR_OP, NULL, "ret") {}

    string toString() override
    {
        return "\t" + join_str({"jr", "$ra"}) + Mips::toString();
    }
};

struct LiMips : CodeMips
{
    LiMips(RegPtr rt, RegPtr intermediate, string annotation)
        : CodeMips(LI_OP, NULL, rt, NULL, intermediate, NULL, annotation)
    {
        if (!in32Reg(rt->getType()) || intermediate->getType() != INTERMEDIATE)
        {
            DIE("in LiMips: wrong type: <" +
                reg_type_2_str(rt->getType()) + ", " + reg_type_2_str(intermediate->getType()) + ">");
        }
    }

    string toString() override
    {
        return "\t" + join_str({"li", rt->getStr() + ",", intermediate->getStr()}) + Mips::toString();
    }
};

struct LaMips : CodeMips
{
    // label + val / label($t) /  val($t)
    LaMips(RegPtr rt, RegPtr base, RegPtr offset, string annotation)
        : CodeMips(LA_OP, base, rt, NULL, offset, NULL, annotation)
    {
        if (toString() == "unknownType")
            DIE("in LaMips: Invalid <base,offset> type: <" +
                reg_type_2_str(base->getType()) + "," + reg_type_2_str(offset->getType()) + ">");
    }

    string toString() override
    {
        string result = "\t";
        RegPtr base = rs, offset = intermediate;
        if (base->getType() == LABEL && offset->getType() == INTERMEDIATE) // label + val
            result += join_str({"la", rt->getStr() + ",", base->getStr(), "+", offset->getStr()});
        else if (base->getType() == LABEL && in32Reg(offset->getType())) // label($t)
            result += join_str({"la", rt->getStr() + ",", base->getStr() + "(" + offset->getStr() + ")"});
        else if (base->getType() == OFFSET && in32Reg(offset->getType())) // val($t)
            result += join_str({"la", rt->getStr() + ",", base->getStr() + "(" + offset->getStr() + ")"});
        else
            return "unknownType";
        return result + Mips::toString();
    }
};

struct NopMips : CodeMips
{
    NopMips() : CodeMips(NOP_OP, NULL, NULL, NULL, NULL, NULL, "") {}

    string toString() override
    {
        return "\t" + join_str({"nop"});
    }
};

struct SyscallMips : CodeMips
{
    SyscallMips() : CodeMips(SYSCALL_OP, NULL, NULL, NULL, NULL, NULL, "") {}

    string toString() override
    {
        return "\t" + join_str({"syscall"});
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
