enum mips_type
{
    GLOBAL_DEF_OP,
    GLOBAL_STR_OP,
};

const map<mips_type, string> mips_type_2_str = {
    {GLOBAL_DEF_OP, "GLOBAL_DEF_OP"},
    {GLOBAL_STR_OP, "GLOBAL_STR_OP"},
};

struct Mips
{
    mips_type mipsType;
    Mips(mips_type mipsType)
    {
        this->mipsType = mipsType;
    }

    virtual string toString()
    {
        cout << "struct hasnt override toString : " << mips_type_2_str.at(mipsType) << endl;
        exit(1);
    }
};

struct GlobalDefMips : Mips
{
    var_type varType;
    string varName;
    vector<int> varInitVals;
    int varLength;
    // 只声明全局的非const数组变量和const数组，不声明const变量
    GlobalDefMips(var_type varType, string varName, int varLength, vector<int> varInitVals) : Mips(GLOBAL_DEF_OP)
    {
        this->varType = varType;
        this->varName = varName;
        this->varLength = varLength;
        this->varInitVals = varInitVals;
    }

    string toString() override
    {
        string initValTk;
        string lenTk = isIntType(varType) ? "word" : "byte";
        for (int val : varInitVals)
            initValTk += to_string(val) + ", ";
        if (!initValTk.empty())
        {
            initValTk.pop_back();
            initValTk.pop_back();
        }
        initValTk = initValTk.empty() ? "" : varName.size() > 3 ? "\n\t\t." + lenTk + " " + initValTk
                                                                : "\t." + lenTk + " " + initValTk;

        string initZeroTk;
        if (varInitVals.size() < varLength)
            initZeroTk = join_str({initValTk.empty() ? "\t.space" : "\n\t\t.space",
                                   to_string((varLength - varInitVals.size()) * (isIntType(varType) ? 4 : 1))});

        return join_str({"\t" + varName + ":" + initValTk, initZeroTk});
    }
};

struct GlobalStrMips : Mips
{
    int loc;
    string str;

    GlobalStrMips(int loc, string str) : Mips(GLOBAL_STR_OP)
    {
        this->loc = loc;
        string tstr;
        for (int i = 0; i < str.length(); i++)
        {
            if (str.at(i) == '\\' && i + 2 < str.length() && str.at(i + 1) == '0' && str.at(i + 2) == 'A')
            {
                tstr += "\\n";
                i += 2;
            }
            else
            {
                tstr += str.at(i);
            }
        }
        this->str = tstr;
    }    

    string toString() override
    {
        return join_str({"\t.str" + to_string(loc)+":\t"+".asciiz", "\"" + str + "\""});
    }
};

struct RTypeMips : Mips
{
};

struct StoreMips : Mips
{
};

struct LoadMips : Mips
{
};

struct BTypeMips : Mips
{
};

struct JTypeMips : Mips
{
};

vector<Mips *> mipsCodes;