int curStrLoc;

enum mid_type
{
    ADD_IR,
    SUB_IR,
    MUL_IR,
    SDIV_IR, // 有符号除法
    SREM_IR, // 有符号取余
    ICMP_IR, // 比较指令
    AND_IR,
    OR_IR,
    CALL_IR,   // 函数调用
    ALLOCA_IR, // 分配内存
    LOAD_IR,
    STORE_IR,
    GETELEMENTPTR_IR, // 计算目标元素的位置
    PHI_IR,           // 选择控制流
    ZEXT_IR,          // 无符号扩展 语法规定了char>0
    TRUNC_IR,         // 截断数据
    BR_IR,            // 改变控制流
    RET_IR,           // 退出当前函数，并返回值

    LABEL_IR, // label

    G_VAR_DEF_IR,  // 全局变量声明
    G_FUNC_DEF_IR, // 函数声明

    G_STR, // 用于putstr

    FUNC_PARAM_IR, // 函数形参标志
    FUNC_END_IR,   // 函数结束标志

    CONST_IR, // 单独一个常数 不可单独作为指令 用于传参
    mid_null
};

set<mid_type> retMidTypes = {
    ADD_IR,
    SUB_IR,
    MUL_IR,
    SDIV_IR, // 有符号除法
    SREM_IR, // 有符号取余
    ICMP_IR, // 比较指令
    AND_IR,
    OR_IR,
    // CALL_IR,   // 函数调用
    ALLOCA_IR, // 分配内存
    LOAD_IR,
    // STORE_IR,
    GETELEMENTPTR_IR, // 计算目标元素的位置
    // PHI_IR,           // 选择控制流
    ZEXT_IR,  // 无符号扩展 语法规定了char>0
    TRUNC_IR, // 截断数据
    // BR_IR,            // 改变控制流
    // RET_IR,           // 退出当前函数，并返回值

    LABEL_IR, // label

    // G_VAR_DEF_IR,  // 全局变量声明
    // G_FUNC_DEF_IR, // 函数声明

    // G_STR, // 用于putstr

    // FUNC_PARAM_IR, // 函数形参标志
    // FUNC_END_IR,   // 函数结束标志

    // CONST_IR, // 单独一个常数 不可单独作为指令 用于传参
    // mid_null
};

const map<tk_type, mid_type> tk_2_mid = {
    {PLUS, ADD_IR},
    {MINU, SUB_IR},
    {MULT, MUL_IR},
    {DIV, SDIV_IR},
    {MOD, SREM_IR},
    {AND, AND_IR},
    {OR, OR_IR}};

const map<tk_type, string> tk_2_cmp = {
    {dop_2_tk_type.at('>'), "sge"},
    {dop_2_tk_type.at('<'), "sle"},
    {dop_2_tk_type.at('='), "eq"},
    {dop_2_tk_type.at('!'), "ne"},
    {sop_2_tk_type.at('>'), "sgt"},
    {sop_2_tk_type.at('<'), "slt"},
};

bool isRType(mid_type type)
{
    return type == ADD_IR || type == SUB_IR || type == MUL_IR ||
           type == SDIV_IR || type == SREM_IR || type == AND_IR || type == OR_IR;
}

string get_i_tk(var_type type)
{
    return isIntType(type) ? "i32" : "i8";
}

string get_ip_tk(var_type type)
{
    return isIntType(type) ? "i32*" : "i8*";
}

string get_ipp_tk(var_type type)
{
    return isIntType(type) ? "i32**" : "i8**";
}

string join_str(vector<string> strList)
{
    string ans;
    for (string str : strList)
    {
        ans += str + " ";
    }
    if (!ans.empty())
    {
        ans.pop_back();
    }
    return ans;
}

string get_op_tk(mid_type type)
{
    switch (type)
    {
    case ADD_IR:
        return "add";
    case SUB_IR:
        return "sub";
    case MUL_IR:
        return "mul";
    case SDIV_IR:
        return "sdiv";
    case SREM_IR:
        return "srem";
    case ICMP_IR:
        return "icmp";
    case AND_IR:
        return "and";
    case OR_IR:
        return "or";
    default:
        cout << "in get_op_tk: unknown midType" << endl;
        exit(1);
    }
}

struct LLVM
{
    mid_type midType;
    string returnTk; // 虚拟寄存器编号 @a 23 %5
    bool hasReturn;

    virtual string toString()
    {
        cout << "LLVM tostring: no override!" << endl;
        cout << midType << endl;
        return "";
    }

    virtual void refillNum()
    {
        cout << "LLVM refillNum: no override!" << endl;
        cout << midType << endl;
    }

    // virtual var_type

    LLVM()
    {
        this->midType = mid_null;
        this->returnTk = -1;
        this->hasReturn = false;
    }

    LLVM(mid_type midType)
    {
        this->midType = midType;
        this->returnTk = -1;
        this->hasReturn = retMidTypes.count(midType) != 0;
    }
};

// 非全局变量而言，def时不用区分是否为常量，llvm ir 相同
// 全局变量有区分 非常量为  @a = i32 97
// 常量为                  @d = constant i32 2
// @f = constant i32 [10] {1,2}
struct GDefLLVM : LLVM
{
    var_type GDefType;
    string varName;
    vector<int> initValList;
    vector<Symbol *> funcFParams;
    int length;

    string initValStr, initValStr0;

    GDefLLVM(var_type GDefType, string varName, vector<Symbol *> funcFParams) : LLVM(G_FUNC_DEF_IR)
    {
        this->GDefType = GDefType;
        this->varName = varName;
        this->funcFParams = funcFParams;
        this->length = 0;
        this->returnTk = "@" + this->varName;
        this->get_initValStr();
    }

    GDefLLVM(var_type GDefType, string varName, vector<int> initValList, int length) : LLVM(G_VAR_DEF_IR)
    {
        this->GDefType = GDefType;
        this->varName = varName;
        this->initValList = initValList;
        this->length = length;
        this->returnTk = "@" + this->varName;
        this->get_initValStr();
    }

    void get_initValStr()
    {
        string iTk = get_i_tk(GDefType);
        vector<string> initValStrList;
        vector<string> initValStrList0;
        for (int val : this->initValList)
        {
            initValStrList0.push_back(iTk + ",");
            initValStrList.push_back(join_str({iTk, to_string(val) + ","}));
        }
        string initValStr = join_str(initValStrList);
        string initValStr0 = join_str(initValStrList0);

        if (!initValStr.empty())
        {
            initValStr0.pop_back();
            initValStr.pop_back();
        }

        int left = this->length - this->initValList.size();
        string comma = left == this->length ? "[" : ", [";
        initValStr += join_str({comma + to_string(left), "x", iTk + "]", "zeroinitializer"});
        initValStr0 += join_str({comma + to_string(left), "x", iTk + "]"});

        initValStr = join_str({"<{", initValStr, "}>"});
        initValStr0 = join_str({"<{", initValStr0, "}>"});

        this->initValStr = initValStr;
        this->initValStr0 = initValStr0;
    }

    void refillNum() override {};

    string toString() override
    {
        if (isArrayType(GDefType))
        {
            string initStr = join_str({this->initValStr0, this->initValStr});
            if (isConstType(this->GDefType))
                return join_str({this->returnTk, "=", "dso_local", "constant", initStr});
            else
                return join_str({this->returnTk, "=", "dso_local", "global", initStr});
        }
        else if (isVarType(this->GDefType))
        // @b = dso_local global i32 1,
        {
            return join_str({this->returnTk, "=", "dso_local", "global", get_i_tk(GDefType), to_string(initValList.at(0))});
        }
        else
        { // define void @func1() {}
            vector<string> funcFParamsList;
            int len = this->funcFParams.size();
            for (int i = 0; i < len; i++)
            {
                var_type type = this->funcFParams.at(i)->type;
                string t = join_str({isArrayType(type) ? get_ip_tk(type) : get_i_tk(type), "%" + to_string(i) + ","});
                funcFParamsList.push_back(t);
            }
            string funcFParamsStr = join_str(funcFParamsList);
            if (!funcFParamsStr.empty())
                funcFParamsStr.pop_back();
            string retTk = this->GDefType == VoidFunc ? "void" : get_i_tk(this->GDefType);
            return join_str({"define", "dso_local", retTk, "@" + this->varName + "(" + funcFParamsStr + ")"});
        }
        return "ERROR";
    }
};

struct FuncFParamLLVM : LLVM
{
    var_type paramType;
    int loc; // 第loc个形参

    FuncFParamLLVM(var_type paramType, int loc) : LLVM(FUNC_PARAM_IR)
    {
        this->paramType = paramType;
        this->loc = loc;
        this->returnTk = "%" + to_string(this->loc);
    }
};

// add sub mul sdiv srem icmp and or
struct RTypeLLVM : LLVM
{
    LLVM *op1, *op2;
    string op1Tk, op2Tk;
    // 有返回值，两个op
    RTypeLLVM(mid_type midType, LLVM *op1, LLVM *op2) : LLVM(midType)
    {
        this->op1 = op1;
        this->op2 = op2;
        this->op1Tk = "undefined tk";
        this->op2Tk = "undefined tk";
    }

    void refillNum() override
    {
        this->op1Tk = op1->returnTk;
        this->op2Tk = op2->returnTk;
    }

    string toString() override
    {
        return join_str({this->returnTk, "=", get_op_tk(this->midType), "i32",
                         this->op1Tk + ",", this->op2Tk});
    }
};

// alloca
//   %2 = alloca [10 x i32], align 4
struct AllocaLLVM : LLVM
{
    var_type allocaType;
    int length;
    bool isfuncArrayParam = false;
    LLVM *funcFParam; // 辅助标识allocatype
    // 有返回值 0个op
    AllocaLLVM(LLVM *funcFParam) : LLVM(ALLOCA_IR) // 函数形参
    {
        this->allocaType = dynamic_cast<FuncFParamLLVM *>(funcFParam)->paramType;
        this->funcFParam = funcFParam;
        this->length = 1;
        this->isfuncArrayParam = true;
    }

    AllocaLLVM(var_type allocaType) : LLVM(ALLOCA_IR)
    { // 变量
        this->allocaType = allocaType;
        this->length = 1;
        this->funcFParam = NULL;
    }

    AllocaLLVM(var_type allocaType, int length) : LLVM(ALLOCA_IR)
    { // 数组
        this->allocaType = allocaType;
        this->length = length;
        this->funcFParam = NULL;
    }

    bool isFuncArrayParam()
    {
        return this->isfuncArrayParam;
    }

    void refillNum() override {}

    string toString() override
    {
        if (isVarType(allocaType))
            return join_str({this->returnTk, "=", "alloca", get_i_tk(allocaType)});
        else if (this->isFuncArrayParam())
            return join_str({this->returnTk, "=", "alloca", get_ip_tk(allocaType)});
        else
            return join_str({this->returnTk, "=", "alloca", "[" + to_string(this->length),
                             "x", get_i_tk(this->allocaType) + "]"});
    }
};

// load
// %7 = load i32, ptr %2, align 4
struct LoadLLVM : LLVM
{
    var_type loadType;
    LLVM *des;
    string desTk;
    // 有返回值，一个op
    LoadLLVM(var_type loadType, LLVM *des) : LLVM(LOAD_IR)
    {
        this->loadType = loadType;
        this->des = des;
    }

    void refillNum() override
    {
        this->desTk = des->returnTk;
    }

    string toString() override
    {

        if (des->midType == ALLOCA_IR &&
            dynamic_cast<AllocaLLVM *>(des)->length == -1 &&
            isArrayType(dynamic_cast<AllocaLLVM *>(des)->allocaType))
            return join_str({this->returnTk, "=", "load", get_i_tk(loadType) + "*" + ",", get_ip_tk(loadType) + "*", this->desTk});
        else
            return join_str({this->returnTk, "=", "load", get_i_tk(loadType) + ",", get_ip_tk(loadType), this->desTk});
    }
};

// 常数 用于当作其他结构体的属性
struct ConstLLVM : LLVM
{
    int val;

    ConstLLVM(int val) : LLVM(CONST_IR)
    {
        this->val = val;
        this->returnTk = to_string(val);
    }
};

// 标志函数结束 每个block结尾都会塞一个到midCodes
struct FuncEndLLVM : LLVM
{
    FuncEndLLVM() : LLVM(FUNC_END_IR) {}

    void refillNum() override {}

    string toString() override
    {
        cout << "cnt tostring in funcEndLLVM" << endl;
        exit(1);
    }
};

// store
// store i32 0, ptr %1
struct StoreLLVM : LLVM
{
    var_type storeType;
    LLVM *val, *des;
    string valTk, desTk;
    // 无返回值 2个op
    StoreLLVM(var_type storeType, LLVM *val, LLVM *des) : LLVM(STORE_IR)
    {
        this->storeType = storeType;
        this->val = val;
        this->des = des;
    }

    void refillNum() override
    {
        this->valTk = val->returnTk;
        this->desTk = des->returnTk;
    }

    string toString() override
    {
        if (val->midType == FUNC_PARAM_IR && isArrayType(dynamic_cast<FuncFParamLLVM *>(val)->paramType))
            return join_str({"store", get_i_tk(storeType) + "*", this->valTk + ",", get_ip_tk(storeType) + "*", this->desTk});
        else
            return join_str({"store", get_i_tk(storeType), this->valTk + ",", get_ip_tk(storeType), this->desTk});
    }
};

// getelementptr
// getelementptr  [10 x i32], ptr %5, i32 0, i32 1
struct GetelementptrLLVM : LLVM
{
    //  %17 = getelementptr inbounds [10 x i32], [10 x i32]* %9, i64 0, i64 0
    var_type arrayType;
    LLVM *src, *offset;
    int size; // 数组大小
    string srcTk, offsetTk;
    // 函数形参数组 size为-1

    GetelementptrLLVM(var_type arrayType, int size, LLVM *src, LLVM *offset) : LLVM(GETELEMENTPTR_IR)
    {
        this->arrayType = arrayType;
        this->size = size;
        this->src = src;
        this->offset = offset;
    }

    void refillNum() override
    {
        if (src != NULL)
            this->srcTk = src->returnTk;
        this->offsetTk = offset->returnTk;
    }

    bool isFuncArrayParam()
    {
        return this->size == -1;
    }

    string toString() override
    {
        if (this->isFuncArrayParam())
        {
            return join_str({this->returnTk, "=", "getelementptr", "inbounds", get_i_tk(arrayType) + ",",
                             get_ip_tk(arrayType), this->srcTk + ",", "i32", this->offsetTk});
        }
        else if (this->src->midType == G_VAR_DEF_IR)
        {
            string typeTk0 = dynamic_cast<GDefLLVM *>(this->src)->initValStr0;
            string typeTk = join_str({"[" + to_string(this->size), "x", get_i_tk(arrayType) + "]"});
            string bitcast = join_str({"bitcast", "(" + typeTk0 + "*", this->srcTk, "to", typeTk + "*)"});
            return join_str({this->returnTk, "=", "getelementptr", "inbounds", typeTk + ",", typeTk + "*",
                             bitcast + ",", "i32", "0,", "i32", this->offsetTk});
        }
        else
        {
            string typeTk = join_str({"[" + to_string(size), "x", get_i_tk(this->arrayType) + "]"});
            return join_str({this->returnTk, "=", "getelementptr", "inbounds", typeTk + ",", typeTk + "*",
                             this->srcTk + ",", "i32", "0,", "i32", this->offsetTk});
        }
    }
};
// ret i32 0
// ret void
struct RetLLVM : LLVM
{
    var_type retType;
    LLVM *src; // ret var
    string srcTk;

    RetLLVM() : LLVM(RET_IR)
    {
        this->retType = VoidFunc;
        this->src = NULL;
    }

    RetLLVM(var_type retType, LLVM *src) : LLVM(RET_IR)
    {
        this->retType = retType;
        this->src = src;
    }

    void refillNum() override
    {
        if (src != NULL)
            this->srcTk = src->returnTk;
    }

    string toString() override
    {
        if (this->retType == VoidFunc)
            return join_str({"ret", "void"});
        else
            return join_str({"ret", get_i_tk(this->retType), srcTk});
    }
};

// %12 = icmp sgt i32 %10, %11
struct IcmpLLVM : LLVM
{
    LLVM *op1, *op2;
    tk_type cmpType; // sgt slt
    string op1Tk, op2Tk;

    IcmpLLVM(tk_type cmpType, LLVM *op1, LLVM *op2) : LLVM(ICMP_IR)
    {
        this->op1 = op1;
        this->op2 = op2;
        this->cmpType = cmpType;
    }

    void refillNum() override
    {
        this->op1Tk = op1->returnTk;
        this->op2Tk = op2->returnTk;
    }

    string toString() override
    {
        return join_str({this->returnTk, "=", "icmp", tk_2_cmp.at(cmpType), "i32", op1Tk + ",", op2Tk});
    }
};

struct ZextLLVM : LLVM
{
    LLVM *src;
    string srcTk;

    ZextLLVM(LLVM *src) : LLVM(ZEXT_IR)
    {
        this->src = src;
    }

    void refillNum() override
    {
        this->srcTk = src->returnTk;
    }

    string toString() override
    {
        if (dynamic_cast<IcmpLLVM *>(this->src))
            return join_str({this->returnTk, "=", "zext", "i1", this->srcTk, "to", "i32"});
        else
            return join_str({this->returnTk, "=", "zext", "i8", this->srcTk, "to", "i32"});
    }
};

struct TruncLLVM : LLVM
{
    LLVM *src;
    string srcTk, desTk;

    TruncLLVM(LLVM *src) : LLVM(TRUNC_IR)
    {
        this->src = src;
        this->desTk = "i8";
    }

    TruncLLVM(LLVM *src, string desTk) : LLVM(TRUNC_IR)
    {
        this->src = src;
        this->desTk = desTk;
    }

    void refillNum() override
    {
        this->srcTk = src->returnTk;
    }

    string toString() override
    {
        return join_str({this->returnTk, "=", "trunc", "i32", this->srcTk, "to", desTk});
    }
};

struct GStrLLVM : LLVM
{
    string str;
    int length;
    int loc; // 用于编号
    string typeTk;

    GStrLLVM(string str, int length) : LLVM(G_STR)
    {
        this->str = str;
        this->length = length;
        this->loc = curStrLoc++;
        this->typeTk = join_str({"[" + to_string(this->length), "x", "i8]"});
        this->returnTk = "@.str." + to_string(this->loc);
    }

    void refillNum() override {}

    string toString() override
    {
        return join_str({this->returnTk, "=", "private", "unnamed_addr", "constant",
                         this->typeTk, "c\"" + this->str + "\\00\""});
    }
};

// %8 = call i32 @func1(i32 %6, i8* %7)
// call void @func1(i32 %10, i8* %11)
struct CallLLVM : LLVM
{
    var_type retType;
    string funcName;
    vector<LLVM *> params;
    vector<Symbol *> paramSymbols;
    vector<string> paramTks;

    LLVM *gStr; // 用于putstr函数

    LLVM *src;

    CallLLVM(LLVM *gStr) : LLVM(CALL_IR)
    {
        this->retType = VoidFunc;
        this->funcName = "putstr";
        this->gStr = gStr;
    }

    CallLLVM(string funcName, LLVM *src) : LLVM(CALL_IR)
    // getint getchar
    {
        this->retType = VoidFunc;
        this->funcName = funcName;
        this->src = src;
    }

    CallLLVM(var_type retType, string funcName, vector<Symbol *> paramSymbols, vector<LLVM *> params) : LLVM(CALL_IR)
    {
        this->retType = retType;
        this->funcName = funcName;
        this->paramSymbols = paramSymbols;
        this->params = params;

        this->hasReturn = retType != VoidFunc;
    }

    void refillNum() override
    {
        for (LLVM *param : this->params)
        {
            paramTks.push_back(param->returnTk);
        }
    }

    string toString() override
    {
        vector<string> paramsStrList;
        for (int i = 0; i < params.size(); i++)
        {
            string itk = isArrayType(paramSymbols.at(i)->type)
                             ? get_ip_tk(paramSymbols.at(i)->type)
                             : get_i_tk(paramSymbols.at(i)->type);
            paramsStrList.push_back(join_str({itk, paramTks.at(i) + ","}));
        }
        string paramsStr = join_str(paramsStrList);
        if (!paramsStr.empty())
            paramsStr.pop_back();

        if (this->funcName == "putstr")
        {
            string gStrTypeTk = dynamic_cast<GStrLLVM *>(this->gStr)->typeTk;
            string getPtrTk = join_str({"getelementptr", "inbounds",
                                        "(" + gStrTypeTk + ",", gStrTypeTk + "*",
                                        gStr->returnTk + ",", "i32 0,", "i32 0)"});
            return join_str({"call", "void", "@" + this->funcName + "(" + "i8*", getPtrTk + ")"});
        }
        else if (this->funcName == "putint" || this->funcName == "putch")
        {
            return join_str({"call", "void", "@" + this->funcName + "(" + "i32", src->returnTk + ")"});
        }
        else if (this->retType == VoidFunc)
            return join_str({"call", "void", "@" + this->funcName + "(" + paramsStr + ")"});
        else
            return join_str({this->returnTk, "=", "call", get_i_tk(this->retType), "@" + this->funcName + "(" + paramsStr + ")"});
    }
};

// 13:
struct LabelLLVM : LLVM
{
    vector<LLVM *> preds; // 前置结点

    LabelLLVM() : LLVM(LABEL_IR) {}

    void refillNum() override {}

    string toString() override
    {
        return this->returnTk.substr(1) + ":";
    }
};

// br i1 %12, label %13, label %14
struct BrLLVM : LLVM
{
    LLVM *cmp, *label1, *label2;
    string cmpTk, label1Tk, label2Tk;

    BrLLVM(LLVM *label) : LLVM(BR_IR)
    {
        this->label1 = label;
        this->label2 = NULL;
        this->cmp = NULL;
        dynamic_cast<LabelLLVM *>(label)->preds.push_back(this);
    }

    BrLLVM(LLVM *cmp, LLVM *label1, LLVM *label2) : LLVM(BR_IR)
    {
        this->cmp = cmp;
        this->label1 = label1;
        this->label2 = label2;
        dynamic_cast<LabelLLVM *>(label1)->preds.push_back(this);
        dynamic_cast<LabelLLVM *>(label2)->preds.push_back(this);
    }

    void changeLabel(LLVM *oldLabel, LLVM *newLabel)
    {
        if (label1 == oldLabel)
            label1 = newLabel;
        if (label2 == oldLabel)
            label2 = newLabel;
    }

    bool isNoCond()
    {
        return this->cmp == NULL;
    }

    void refillNum() override
    {
        this->label1Tk = label1->returnTk;
        if (this->cmp != NULL)
        {
            this->cmpTk = cmp->returnTk;
            this->label2Tk = label2->returnTk;
        }
    }

    string toString() override
    {
        if (cmp == NULL)
            return join_str({"br", "label", label1Tk});
        else
            return join_str({"br", "i1", cmpTk + ",", "label", label1Tk + ",", "label", label2Tk});
    }
};

vector<LLVM *> midCodes;
vector<LLVM *> strCodes;
map<Symbol *, LLVM *> syb_2_llvm;
var_type curFuncType;
LLVM *curFuncReturnLabel, *curFuncReturnVar;
vector<LLVM *> ctnLabels;
vector<LLVM *> brkLabels;

void syb_2_llvm_insert(Symbol *symbol, LLVM *llvm)
{
    if (syb_2_llvm.find(symbol) != syb_2_llvm.end())
    {
        cout << "insert syb_2_llvm failed!!" << endl;
        exit(1);
    }
    syb_2_llvm.insert({symbol, llvm});
}

LLVM *generate_midCode(LLVM *llvm);
void generate_CompUnit(Node *root);
void generate_Decl(Node *root, bool isGlobal);
void generate_ConstDecl(Node *root, bool isGlobal);
void generate_ConstDef(Node *root, bool isGlobal);
void generate_VarDecl(Node *root, bool isGlobal);
void generate_VarDef(Node *root, bool isGlobal);
void generate_Block(Node *root);
LLVM *generate_stmt_LVal(Node *LVal);
bool generate_Stmt(Node *root);
void generate_ForStmt(Node *root);
void generate_FuncDef(Node *root);
void generate_Cond(Node *root, LLVM *trueLabel, LLVM *falseLabel);
LLVM *generate_Exp(Node *root);
LLVM *generate_LVal(Node *root);
LLVM *generate_PrimaryExp(Node *root);
LLVM *generate_Number(Node *root);
LLVM *generate_Character(Node *root);
LLVM *generate_UnaryExp(Node *root);
vector<LLVM *> generate_FuncRParams(Node *root, vector<Symbol *> paramSymbols);
LLVM *generate_MulExp(Node *root);
LLVM *generate_AddExp(Node *root);
LLVM *generate_RelExp(Node *root);
LLVM *generate_EqExp(Node *root);
void generate_LAndExp(Node *root, LLVM *trueLabel, LLVM *falseLabel);
void generate_LOrExp(Node *root, LLVM *trueLabel, LLVM *falseLabel);

LLVM *find_GStr(string str, int len);
void symbols_2_mid();
