struct DataMips : Mips
{
    // 全局变量
    var_type varType;
    string varName;
    vector<int> varInitVals;
    int varLength;
    // 字符串
    int loc;
    string str;

    DataMips(var_type varType, string varName, vector<int> varInitVals, int varLength,
               int loc, string str, mips_type mipsType) : Mips(mipsType, "")
    {
        this->varType = varType;
        this->varName = varName;
        this->varInitVals = varInitVals;
        this->varLength = varLength;
        this->loc = loc;
        this->str = this->transferStr(str);
    }

private:
    string transferStr(string str)
    {
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
        return tstr;
    }
};

struct GlobalDefMips : DataMips
{
    // 只声明全局的非const数组变量和const数组，不声明const变量
    GlobalDefMips(var_type varType, string varName, int varLength, vector<int> varInitVals)
        : DataMips(varType, varName, varInitVals, varLength, -1, "", GLOBAL_DEF_OP) {}

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

        return "\t" + join_str({varName + ":" + initValTk, initZeroTk});
    }
};

struct GlobalStrMips : DataMips
{
    GlobalStrMips(int loc, string str)
        : DataMips(var_null, "", vector<int>(), -1, loc, str, GLOBAL_STR_OP) {}

    string toString() override
    {
        return "\t" + join_str({".str" + to_string(loc) + ":\t" + ".asciiz", "\"" + str + "\""});
    }
};