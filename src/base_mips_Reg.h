typedef struct RegStruct
{
    reg_type regType;
    int offset = 0;

    RegStruct(reg_type regType)
    {
        this->regType = regType;
    }

    RegStruct(reg_type regType, int offset)
    {
        this->regType = regType;
        this->offset = offset;
    }

    reg_type getType()
    {
        return this->regType;
    }

    string getStr()
    {
        return reg_type_2_str(this->regType);
    }

} *RegPtr;

struct TempReg : RegStruct
{
    TempReg(reg_type regType) : RegStruct(regType) {}
};

struct OffsetReg : RegStruct
{
    OffsetReg(int offset) : RegStruct(OFFSET, offset) {}
};
