class MipsManager;

typedef struct RegStruct
{
    reg_type regType;
    int val = 0;
    string labelName = "undefined labelName"; // 全局变量

    RegStruct(reg_type regType)
    {
        this->regType = regType;
    }

    RegStruct(reg_type regType, int val, string labelName)
    {
        this->regType = regType;
        this->val = val;
        this->labelName = labelName;
    }

    reg_type getType()
    {
        return this->regType;
    }

    virtual string getStr()
    {
        DIE("hasnt override getStr");
        return "";
    }

} *RegPtr;

struct TempReg : RegStruct
{
    TempReg(reg_type regType) : RegStruct(regType) {}

    string getStr() override
    {
        return reg_type_2_str(this->regType);
    }
};

struct IntermediateReg : RegStruct
{
    IntermediateReg(int intermediate) : RegStruct(INTERMEDIATE, intermediate, "") {}

    string getStr() override
    {
        return to_string(val);
    }

private:
    friend class MipsManager;

    void changeValTo(int newVal) { this->val = newVal; }
};

struct OffsetReg : RegStruct
{
    OffsetReg(int offset) : RegStruct(OFFSET, offset, "") {}

    string getStr() override
    {
        return to_string(val);
    }
};

struct LabelReg : RegStruct
{
    LabelReg(string labelName) : RegStruct(LABEL, 0, labelName) {}

    string getStr() override
    {
        return labelName;
    }
};

typedef struct _RegPool
{
private:
    map<reg_type, RegPtr> regPool;
    const vector<reg_type> regTypes = {
        T0, T1, T2, T3, T4, T5, T6, T7, T8, T9};
    int index = 0; // 临时寄存器分配号，每次申请都+1 mod n

public:
    bool inRegTypes(reg_type type)
    {
        for (reg_type regType : regTypes)
        {
            if (regType == type)
            {
                return true;
            }
        }
        return false;
    }

    int POOLSIZE = regTypes.size();

    _RegPool()
    {
        for (reg_type regType : regTypes)
            regPool.insert({regType, new TempReg(regType)});
    }

    // 如果regType不在regTypes内 无事发生 返回NULL
    RegPtr tryOccupy(reg_type regType)
    {
        if (inRegTypes(regType))
        {
            if (!isFree(regType))
                DIE("already occupied !:" + reg_type_2_str(regType));
            RegPtr reg = regPool.at(regType);
            regPool.erase(regType);
            return reg;
        }
        return NULL;
    }

    // 如果regType不在regTypes内 无事发生
    void tryRelease(RegPtr reg)
    {
        if (inRegTypes(reg->getType()))
        {
            if (isFree(reg->getType()))
                DIE("in regPool tryRelease :already released !:" + reg_type_2_str(reg->getType()));
            regPool.insert({reg->getType(), reg});
        }
    }

    reg_type getRegTypeAt(int i = 0) { return regTypes.at((index + i) % POOLSIZE); }

    RegPtr getReg(reg_type regType)
    {
        if (!isFree(regType))
            DIE("not free regType:" + reg_type_2_str(regType));
        return regPool.at(regType);
    }

    // 返回update后的regType
    reg_type updateRegType(int i = 1)
    {
        index = (index + i) % POOLSIZE;
        return regTypes.at(index);
    }

    bool isFree(reg_type regType) { return regPool.find(regType) != regPool.end(); }

} *RegPool;
