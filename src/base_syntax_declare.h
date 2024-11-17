enum syn_type
{
    CompUnit,
    Decl,
    ConstDecl,
    BType,
    ConstDef,
    ConstInitVal,
    VarDecl,
    VarDef,
    InitVal,
    FuncDef,
    MainFuncDef,
    FuncType,
    FuncFParams,
    FuncFParam,
    Block,
    BlockItem,
    Stmt,
    ForStmt,
    Exp,
    Cond,
    LVal,
    PrimaryExp,
    Number,
    Character,
    UnaryExp,
    UnaryOp,
    FuncRParams,
    MulExp,
    AddExp,
    RelExp,
    EqExp,
    LAndExp,
    LOrExp,
    ConstExp,
    TERMINAL
};

const map<syn_type, string> syn_type_2_str = {
    {CompUnit, "CompUnit"},
    {Decl, "Decl"},
    {ConstDecl, "ConstDecl"},
    {BType, "BType"},
    {ConstDef, "ConstDef"},
    {ConstInitVal, "ConstInitVal"},
    {VarDecl, "VarDecl"},
    {VarDef, "VarDef"},
    {InitVal, "InitVal"},
    {FuncDef, "FuncDef"},
    {MainFuncDef, "MainFuncDef"},
    {FuncType, "FuncType"},
    {FuncFParams, "FuncFParams"},
    {FuncFParam, "FuncFParam"},
    {Block, "Block"},
    {BlockItem, "BlockItem"},
    {Stmt, "Stmt"},
    {ForStmt, "ForStmt"},
    {Exp, "Exp"},
    {Cond, "Cond"},
    {LVal, "LVal"},
    {PrimaryExp, "PrimaryExp"},
    {Number, "Number"},
    {Character, "Character"},
    {UnaryExp, "UnaryExp"},
    {UnaryOp, "UnaryOp"},
    {FuncRParams, "FuncRParams"},
    {MulExp, "MulExp"},
    {AddExp, "AddExp"},
    {RelExp, "RelExp"},
    {EqExp, "EqExp"},
    {LAndExp, "LAndExp"},
    {LOrExp, "LOrExp"},
    {ConstExp, "ConstExp"},
    {TERMINAL, "TERMINAL"}
};

set<tk_type> ExpFisrt = {INTCON, CHRCON, IDENFR, LPARENT, PLUS, MINU, NOT};
set<tk_type> InitValFisrt = {INTCON, CHRCON, IDENFR, LPARENT, PLUS, MINU, NOT, STRCON, LBRACE};
const map<syn_type, set<tk_type>> FIRST = {
    {CompUnit, {VOIDTK, INTTK, CHARTK, CONSTTK}},
    {Decl, {CONSTTK, INTTK, CHARTK}},
    {ConstDecl, {CONSTTK}},
    {BType, {INTTK, CHARTK}},
    {ConstDef, {CONSTTK}},
    {ConstInitVal, InitValFisrt},
    {VarDecl, {INTTK, CHARTK}},
    {VarDef, {IDENFR}},
    {InitVal, InitValFisrt},
    {FuncDef, {VOIDTK, INTTK, CHARTK}},
    {MainFuncDef, {INTTK}},
    {FuncType, {VOIDTK, INTTK, CHARTK}},
    {FuncFParams, {INTTK, CHARTK}},
    {FuncFParam, {INTTK, CHARTK}},
    {Block, {LBRACE}},
    {BlockItem, {}},
    {Stmt, {}},
    {ForStmt, {IDENFR}},
    {Exp, ExpFisrt},
    {Cond, ExpFisrt},
    {LVal, {IDENFR}},
    {PrimaryExp, {INTCON, CHRCON, IDENFR, LPARENT}},
    {Number, {INTCON}},
    {Character, {CHRCON}},
    {UnaryExp, ExpFisrt},
    {UnaryOp, {PLUS, MINU, NOT}},
    {FuncRParams, ExpFisrt},
    {MulExp, ExpFisrt},
    {AddExp, ExpFisrt},
    {RelExp, ExpFisrt},
    {EqExp, ExpFisrt},
    {LAndExp, ExpFisrt},
    {LOrExp, ExpFisrt},
    {ConstExp, ExpFisrt},
    {TERMINAL, {}}
};

struct Node
{
    syn_type type;
    vector<Node *> childs;
    Token *token;
    
    Node(syn_type type) {
        this->type = type;
        // cout << "START PARSE " << syn_type_2_str[type] << endl;
        this->token = new Token();
    }

    Node(syn_type type, Token *token) {
        this->type = type;
        this->token = token;
    }

    Node *GetFirstChild() {
        return this->childs.front();
    }

    Node *GetLastChild() {
        return this->childs.back();
    }

    Node *GetChildAt(int x) {
        if (x > this->childs.size())
        {
            cout << "out of rang:" << x << endl;
            exit(1);
        }
        return this->childs.at(x);
    }

    int GetChildsSize() {
        return this->childs.size();
    }

    void AddChild(Node *child) {
        this->childs.push_back(child);
    }

    void AddFisrtChild(Node *child) {
        this->childs.insert(this->childs.begin(), child);
    }


    Node *PopChild() {
        Node *temp = this->childs.back();
        this->childs.pop_back();
        return temp;
    }
};


void rebuild_tree(Node *root, syn_type type);

Node *parse_Decl();
Node *parse_ConstDecl();
Node *parse_BType();
Node *parse_ConstDef();
Node *parse_ConstInitVal();
Node *parse_VarDecl();
Node *parse_VarDef();
Node *parse_InitVal();
Node *parse_FuncDef();
Node *parse_MainFuncDef();
Node *parse_FuncType();
Node *parse_FuncFParams();
Node *parse_FuncFParam();
Node *parse_Block();
Node *parse_BlockItem();
Node *parse_Stmt();
Node *parse_ForStmt();
Node *parse_Exp();
Node *parse_Cond();
Node *parse_LVal();
Node *parse_PrimaryExp();
Node *parse_Number();
Node *parse_Character();
Node *parse_UnaryExp();
Node *parse_UnaryOp();
Node *parse_FuncRParams();
Node *parse_MulExp();
Node *parse_AddExp();
Node *parse_RelExp();
Node *parse_EqExp();
Node *parse_LAndExp();
Node *parse_LOrExp();
Node *parse_ConstExp();

int loc = 0;
Node *ROOT;