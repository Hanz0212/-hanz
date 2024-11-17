enum var_type
{
    ConstChar,
    ConstInt,
    ConstCharArray,
    ConstIntArray,
    Char,
    Int,
    CharArray,
    IntArray,

    VoidFunc,
    CharFunc,
    IntFunc,
    var_null
};

bool isConstType(var_type type)
{
    return type == ConstChar || type == ConstInt || type == ConstCharArray || type == ConstIntArray;
}

bool isArrayType(var_type type)
{
    return type == ConstCharArray || type == ConstIntArray || type == CharArray || type == IntArray;
}

bool isVarType(var_type type)
{
    return type == ConstChar || type == ConstInt || type == Char || type == Int;
}

bool isFuncType(var_type type)
{
    return type == VoidFunc || type == IntFunc || type == CharFunc;
}

bool isIntType(var_type type)
{
    return type == ConstInt || type == Int || type == IntArray || type == ConstIntArray || type == IntFunc;
}

bool isMatcherType(var_type type1, var_type type2)
{
    bool isArray1 = isArrayType(type1);
    bool isArray2 = isArrayType(type2);
    return isArray1 == isArray2 && (!isArray1 || type1 == type2);
}

var_type arrayType_2_type(var_type type)
{
    switch (type)
    {
    case ConstIntArray:
        return ConstInt;
    case ConstCharArray:
        return ConstChar;
    case IntArray:
        return Int;
    case CharArray:
        return Char;
    }
    return type;
}

const map<var_type, string> var_type_2_str = {
    {ConstChar, "ConstChar"},
    {ConstInt, "ConstInt"},
    {ConstCharArray, "ConstCharArray"},
    {ConstIntArray, "ConstIntArray"},
    {Char, "Char"},
    {Int, "Int"},
    {CharArray, "CharArray"},
    {IntArray, "IntArray"},
    {VoidFunc, "VoidFunc"},
    {CharFunc, "CharFunc"},
    {IntFunc, "IntFunc"}};

const map<int, var_type> int_2_var_type = {
    // isConst isArray isInt
    {0b000, Char},
    {0b001, Int},
    {0b010, CharArray},
    {0b011, IntArray},
    {0b100, ConstChar},
    {0b101, ConstInt},
    {0b110, ConstCharArray},
    {0b111, ConstIntArray}};

const map<string, var_type> str_2_funcType = {
    {"void", VoidFunc},
    {"int", IntFunc},
    {"char", CharFunc}};

struct Symbol
{
    var_type type;
    int scope;
    Token *token;

    Symbol()
    {
        type = var_null;
        scope = -1;
        token = new Token();
    }

    Symbol(var_type type, int scope, Token *token)
    {
        this->type = type;
        this->scope = scope;
        this->token = token;
    }

    string GetIdentName()
    {
        return this->token->value;
    }

    int GetIdentRow()
    {
        return this->token->row;
    }

    virtual bool isFuncArrayParam()
    {
        return false;
    }

    friend ostream &operator<<(ostream &output, const Symbol &symbol)
    {
        output << symbol.scope << ' ' << symbol.token->value << ' ' << var_type_2_str.at(symbol.type);
        return output;
    }
};

struct FuncSymbol : public Symbol
{
    var_type returnType; // 返回值类型
    bool filled;
    vector<Symbol *> funcFParamSymbols;

    Node *returnTypeNode;

    FuncSymbol() : Symbol()
    {
        this->returnType = var_null;
        this->returnTypeNode = NULL;
    }

    FuncSymbol(var_type type, int scope, Token *token, Node *returnTypeNode)
        : Symbol(type, scope, token)
    {
        this->returnTypeNode = returnTypeNode;
        this->filled = false;
        this->returnType = var_null;
    }

    void fillSymbols(vector<Symbol *> funcFParamSymbols)
    {
        this->funcFParamSymbols = funcFParamSymbols;
        this->filled = true;
    }
};

struct ArraySymbol : public Symbol
{
    int length;
    vector<int> initVal;

    Node *lengthNode;  // 为NULL代表是函数的形参
    Node *initValNode; // 为NULL代表无初始值

    ArraySymbol() : Symbol()
    {
        this->length = -1;
        this->lengthNode = NULL;
        this->initValNode = NULL;
    }

    ArraySymbol(var_type type, int scope, Token *token,
                Node *lengthNode, Node *initValNode)
        : Symbol(type, scope, token)
    {
        this->lengthNode = lengthNode;
        this->initValNode = initValNode;

        this->length = -1;
    }

    int getInitValAt(int loc)
    {
        if (loc < this->initVal.size())
            return this->initVal[loc];
        else
            return 0;
    }

    bool isFuncArrayParam() override
    {
        return this->length == -1;
    }
};

struct VarSymbol : public Symbol
{
    int initVal;

    Node *initValNode;

    VarSymbol() : Symbol()
    {
        this->initVal = 0;
        this->initValNode = NULL;
    }

    VarSymbol(var_type type, int scope, Token *token,
              Node *initValNode)
        : Symbol(type, scope, token)
    {
        this->initValNode = initValNode;

        this->initVal = 0;
    }
};

var_type get_var_type(Node *btype, Node *def, bool isConst);
void tree_2_symbols();
void add_symbol(Symbol *symbol);

void analyze_CompUnit(Node *root);
void analyze_Decl(Node *root);
void analyze_ConstDecl(Node *root);
void analyze_ConstDef(Node *root, Node *btype);
void analyze_ConstInitVal(Node *root);
void analyze_VarDecl(Node *root);
void analyze_VarDef(Node *root, Node *btype);
void analyze_InitVal(Node *root);
void analyze_Block(Node *root);
void analyze_Stmt(Node *root);
void analyze_ForStmt(Node *root);
void analyze_FuncDef(Node *root, bool isFirst);
vector<Symbol *> analyze_FuncFParams(Node *root, bool isFirst);
Symbol *analyze_FuncFParam(Node *root, bool isFirst);
var_type analyze_Exp(Node *root);
var_type analyze_LVal(Node *root);
var_type analyze_PrimaryExp(Node *root);
var_type analyze_Number(Node *root);
var_type analyze_Character(Node *root);
var_type analyze_UnaryExp(Node *root);
vector<var_type> analyze_FuncRParams(Node *root);
var_type analyze_MulExp(Node *root);
var_type analyze_AddExp(Node *root);
var_type analyze_ConstExp(Node *root);

void tree_2_symbols();

int scopeCnt = 1; // 全局变量的scope是1，存在scope2symbols[1]中
int curScope = 1;
bool inVoidFunc = false;
int loopStack = 0;
vector<Symbol *> symbols;
set<Symbol *> caledSymbols;
map<int, vector<Symbol *>> scope2symbols;
vector<int> scopeStack;
map<int, int> fatherScope = {{1, 0}};
// 普通block则key为block的root  函数则为funcdef的root
map<Node *, int> node_2_scopeCnt; 