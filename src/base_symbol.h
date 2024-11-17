var_type get_var_type(Node *btype, Node *def, bool isConst)
{
    bool isArray = def->GetChildsSize() > 1 && def->GetChildAt(1)->token->type == LBRACK;
    bool isInt = btype->GetFirstChild()->token->type == INTTK;
    int combined = (isConst << 2) | (isArray << 1) | isInt;
    return int_2_var_type.at(combined);
}

void add_symbol(Symbol *symbol)
{
    symbols.push_back(symbol);
    if (scope2symbols.find(symbol->scope) == scope2symbols.end())
    {
        scope2symbols.insert({symbol->scope, vector<Symbol *>()});
    }
    scope2symbols.at(symbol->scope).push_back(symbol);
}

// 编译单元 CompUnit → {Decl} {FuncDef} MainFuncDef
void analyze_CompUnit(Node *root)
{
    int childSize = root->GetChildsSize();
    int key;
    for (int i = 0; i < childSize; i++)
    {
        if (root->GetChildAt(i)->type != Decl)
        {
            key = i;
            break;
        }
        analyze_Decl(root->GetChildAt(i));
    }
    for (int i = key; i < childSize; i++)
    {
        if (root->GetChildAt(i)->type != FuncDef)
        {
            break;
        }
        analyze_FuncDef(root->GetChildAt(i), true);
    }
    for (int i = key; i < childSize; i++)
    {
        if (root->GetChildAt(i)->type != FuncDef)
        {
            break;
        }
        analyze_FuncDef(root->GetChildAt(i), false);
    }
    Node *mainFuncDef = root->GetLastChild();
    // main函数需要有返回值
    error_g(IntFunc, mainFuncDef->GetChildAt(4));
    analyze_Block(mainFuncDef->GetChildAt(4));
}

// 声明 Decl → ConstDecl | VarDecl
void analyze_Decl(Node *root)
{
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == ConstDecl)
    {
        analyze_ConstDecl(firstChild);
    }
    else
    {
        analyze_VarDecl(firstChild);
    }
}

// 常量声明 ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
void analyze_ConstDecl(Node *root)
{
    Node *btype = root->GetChildAt(1);
    int childSize = root->GetChildsSize();
    for (int i = 2; i < childSize; i++)
    {
        if (root->GetChildAt(i)->type == ConstDef)
        {
            analyze_ConstDef(root->GetChildAt(i), btype);
        }
    }
}

// 常量定义 ConstDef → Ident [ '[' ConstExp ']' ] '=' ConstInitVal // b
void analyze_ConstDef(Node *root, Node *btype)
{
    var_type type = get_var_type(btype, root, true);
    Token *token = root->GetFirstChild()->token;
    Node *constExp = NULL;
    Node *initVal = NULL;
    // 常量名重定义
    if (error_b(token->value, token->row))
        return;

    if (root->GetChildAt(1)->token->type == LBRACK)
    {
        analyze_ConstExp(root->GetChildAt(2));
        constExp = root->GetChildAt(2);
    }
    if (root->GetLastChild()->type == ConstInitVal)
    {
        analyze_ConstInitVal(root->GetLastChild());
        initVal = root->GetLastChild();
    }

    if (isArrayType(type))
    {
        add_symbol(new ArraySymbol(type, scopeStack.back(), token, constExp, initVal));
    }
    else
    {
        add_symbol(new VarSymbol(type, scopeStack.back(), token, initVal));
    }
}

// 常量初值 ConstInitVal → ConstExp | '{' [ ConstExp { ',' ConstExp } ] '}' | StringConst
void analyze_ConstInitVal(Node *root)
{
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == ConstExp)
    {
        analyze_ConstExp(firstChild);
    }
    else if (firstChild->token->type == LBRACE)
    {
        for (Node *child : root->childs)
        {
            if (child->type == ConstExp)
            {
                analyze_ConstExp(child);
            }
        }
    }
    else if (firstChild->token->type == STRCON)
    {
        /*NOTHING*/
        ;
    }
}

// 变量声明 VarDecl → BType VarDef { ',' VarDef } ';'
void analyze_VarDecl(Node *root)
{
    Node *btype = root->GetChildAt(0);
    int childSize = root->GetChildsSize();
    for (int i = 1; i < childSize; i++)
    {
        if (root->GetChildAt(i)->type == VarDef)
        {
            analyze_VarDef(root->GetChildAt(i), btype);
        }
    }
}

// 变量定义 VarDef → Ident [ '[' ConstExp ']' ] | Ident [ '[' ConstExp ']' ] '=' InitVal // b
void analyze_VarDef(Node *root, Node *btype)
{

    var_type type = get_var_type(btype, root, false);
    Token *token = root->GetFirstChild()->token;
    Node *constExp = NULL;
    Node *initVal = NULL;
    // 变量名重定义
    if (error_b(token->value, token->row))
        return;

    if (root->GetChildsSize() > 1 && root->GetChildsSize() > 1 && root->GetChildAt(1)->token->type == LBRACK)
    {
        analyze_ConstExp(root->GetChildAt(2));
        constExp = root->GetChildAt(2);
    }
    if (root->GetLastChild()->type == InitVal)
    {
        analyze_InitVal(root->GetLastChild());
        initVal = root->GetLastChild();
    }

    if (isArrayType(type))
    {
        add_symbol(new ArraySymbol(type, scopeStack.back(), token, constExp, initVal));
    }
    else
    {
        add_symbol(new VarSymbol(type, scopeStack.back(), token, initVal));
    }
}

// 变量初值 InitVal → Exp | '{' [ Exp { ',' Exp } ] '}' | StringConst
void analyze_InitVal(Node *root)
{
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == Exp)
    {
        analyze_Exp(firstChild);
    }
    else if (firstChild->token->type == LBRACE)
    {
        for (Node *child : root->childs)
        {
            if (child->type == Exp)
            {
                analyze_ConstExp(child);
            }
        }
    }
    else if (firstChild->token->type == STRCON)
    {
        /*NOTHING*/
        ;
    }
}

// 语句块 Block → '{' { BlockItem } '}'
void analyze_Block(Node *root)
{
    ++scopeCnt;
    fatherScope.insert({scopeCnt, curScope}); // 新进入的scope的父亲为当前的scope
    curScope = scopeCnt;                      // 更新当亲的scope
    scopeStack.push_back(scopeCnt);
    node_2_scopeCnt.insert({root, scopeCnt});
    for (Node *child : root->childs)
    {
        if (child->type == BlockItem)
        {
            child = child->GetFirstChild();
            if (child->type == Decl)
            {
                analyze_Decl(child);
            }
            else if (child->type == Stmt)
            {
                analyze_Stmt(child);
            }
        }
    }
    scopeStack.pop_back();
    curScope = fatherScope.at(curScope); // 退出scope时将当前scope恢复为当前scope的fatherScope
}

// stmt
void analyze_Stmt(Node *root)
{
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == Block)
    {
        analyze_Block(firstChild);
    }
    else if (firstChild->token->type == IFTK)
    {
        analyze_Stmt(root->GetChildAt(4));
        if (root->GetChildsSize() > 6)
        {
            analyze_Stmt(root->GetChildAt(6));
        }
    }
    else if (firstChild->token->type == FORTK)
    {
        for (Node *child : root->childs)
        {
            if (child->type == ForStmt)
            {
                analyze_ForStmt(child);
            }
        }
        loopStack++;
        analyze_Stmt(root->GetLastChild());
        loopStack--;
    }
    else if (firstChild->token->type == BREAKTK)
    {
        error_m(firstChild->token->row);
    }
    else if (firstChild->token->type == CONTINUETK)
    {
        error_m(firstChild->token->row);
    }
    else if (firstChild->token->type == RETURNTK)
    {
        error_f(firstChild->token->row, root->GetChildAt(1)->type == Exp);
    }
    else if (firstChild->token->type == PRINTFTK)
    {
        string str = root->GetChildAt(2)->token->value;
        int expCnt = 0;
        for (Node *child : root->childs)
        {
            if (child->type == Exp)
            {
                analyze_Exp(child);
                expCnt++;
            }
        }
        // printf占位符和传参数量不符
        error_l(str, expCnt, firstChild->token->row);
    }
    else if (firstChild->type == LVal)
    {
        var_type type = analyze_LVal(firstChild);
        // stmt中的LVal不能为常量
        error_h(type, firstChild->GetFirstChild()->token->row);
        if (root->GetChildAt(2)->type == Exp)
        {
            analyze_Exp(root->GetChildAt(2));
        }
    }
    else if (firstChild->type == Exp)
    {
        analyze_Exp(firstChild);
    }
}

// 语句 ForStmt → LVal '=' Exp // h
void analyze_ForStmt(Node *root)
{
    var_type type = analyze_LVal(root->GetFirstChild());
    // for 语句中的LVal不能为常量
    error_h(type, root->GetFirstChild()->GetFirstChild()->token->row);
    analyze_Exp(root->GetChildAt(2));
}

// 函数定义 FuncDef → FuncType Ident '(' [FuncFParams] ')' Block // b g
void analyze_FuncDef(Node *root, bool isFirst)
{
    var_type type = str_2_funcType.at(root->GetFirstChild()->GetFirstChild()->token->value);
    Token *token = root->GetChildAt(1)->token;
    Node *returnTypeNode = root->GetFirstChild();
    if (isFirst)
    {
        // 函数名重定义
        if (!error_b(token->value, token->row))
        {
            Symbol *symbol = new FuncSymbol(type, 1, token, returnTypeNode);
            add_symbol(symbol);
            // cout << var_type_2_str[symbol->type] << endl;
        }
        // 有返回值的函数的block最后一句不是return
        error_g(type, root->GetLastChild());
    }
    else
    {
        vector<Symbol *> funcFParamSymbols;
        Symbol *symbol = find_symbol(token, true);
        // cout << var_type_2_str[symbol->type] << endl;
        inVoidFunc = type == VoidFunc;
        { // block
            ++scopeCnt;
            fatherScope.insert({scopeCnt, curScope});
            curScope = scopeCnt;
            scopeStack.push_back(scopeCnt);
            node_2_scopeCnt.insert({root, scopeCnt});
            node_2_scopeCnt.insert({root->GetLastChild(), scopeCnt});

            if (root->GetChildAt(3)->type == FuncFParams)
            {
                funcFParamSymbols = analyze_FuncFParams(root->GetChildAt(3), false);
            }

            if (isFuncType(symbol->type) && dynamic_cast<FuncSymbol *>(symbol)->filled == false)
                dynamic_cast<FuncSymbol *>(symbol)->fillSymbols(funcFParamSymbols);

            root = root->GetLastChild();
            for (Node *child : root->childs)
            {
                if (child->type == BlockItem)
                {
                    child = child->GetFirstChild();
                    if (child->type == Decl)
                    {
                        analyze_Decl(child);
                    }
                    else if (child->type == Stmt)
                    {
                        analyze_Stmt(child);
                    }
                }
            }
            scopeStack.pop_back();
            curScope = fatherScope.at(curScope); // 退出scope时将当前scope恢复为当前scope的fatherScope
        }
        inVoidFunc = false;

        // cout << symbol->funcFParamSymbols.size() << endl;
    }
}

// 函数形参表 FuncFParams → FuncFParam { ',' FuncFParam }
vector<Symbol *> analyze_FuncFParams(Node *root, bool isFirst)
{
    vector<Symbol *> funcFParamSymbols;
    for (Node *child : root->childs)
    {
        if (child->type == FuncFParam)
        {
            Symbol *symbol = analyze_FuncFParam(child, isFirst);
            funcFParamSymbols.push_back(symbol);
        }
    }
    return funcFParamSymbols;
}

// 函数形参 FuncFParam → BType Ident ['[' ']'] // b
Symbol *analyze_FuncFParam(Node *root, bool isFirst)
{
    Token *token = root->GetChildAt(1)->token;
    Symbol *symbol = new Symbol();
    if (!isFirst)
    {
        // 函数形参重定义
        error_b(token->value, token->row);

        bool isArray = root->GetChildsSize() > 2;
        bool isInt = root->GetFirstChild()->GetFirstChild()->token->type == INTTK;
        int combined = (isArray << 1) | isInt;
        var_type type = int_2_var_type.at(combined);
        if (isArrayType(type))
        {
            symbol = new ArraySymbol(type, scopeStack.back(), token, NULL, NULL);
            add_symbol(symbol);
        }
        else
        {
            symbol = new VarSymbol(type, scopeStack.back(), token, NULL);
            add_symbol(symbol);
        }
    }
    return symbol;
}

// 表达式 Exp → AddExp
var_type analyze_Exp(Node *root)
{
    return analyze_AddExp(root->GetFirstChild());
}

// 左值表达式 LVal → Ident ['[' Exp ']'] // c
var_type analyze_LVal(Node *root)
{
    // 左值中有未定义的变量名
    Symbol *symbol = error_c(root->GetFirstChild()->token);
    if (symbol != NULL && root->GetChildsSize() > 1)
    {
        analyze_Exp(root->GetChildAt(2));
        return arrayType_2_type(symbol->type);
    }
    return symbol == NULL ? var_null : symbol->type;
}

// 基本表达式 PrimaryExp → '(' Exp ')' | LVal | Number | Character
var_type analyze_PrimaryExp(Node *root)
{
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == LVal)
    {
        return analyze_LVal(firstChild);
    }
    else if (firstChild->type == Number)
    {
        return analyze_Number(firstChild);
    }
    else if (firstChild->type == Character)
    {
        return analyze_Character(firstChild);
    }
    else if (firstChild->token->type == LPARENT)
    {
        return analyze_Exp(root->GetChildAt(1));
    }

    return var_null;
}

// 数值 Number → IntConst
var_type analyze_Number(Node *root)
{
    return Int;
}

// 字符 Character → CharConst
var_type analyze_Character(Node *root)
{
    return Char;
}

// 一元表达式 UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp // c d e
var_type analyze_UnaryExp(Node *root)
{
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == PrimaryExp)
    {
        return analyze_PrimaryExp(firstChild);
    }
    else if (firstChild->type == UnaryOp)
    {
        return analyze_UnaryExp(root->GetChildAt(1));
    }
    else if (firstChild->token->type == IDENFR)
    {
        // 右值中有未定义的变量名
        Symbol *symbol = error_c(firstChild->token);
        if (symbol != NULL)
        { // 仅当函数名有意义时再判断参数问题
            vector<var_type> funcRParamTypes;
            if (root->GetChildsSize() > 2)
            {
                funcRParamTypes = analyze_FuncRParams(root->GetChildAt(2));
            }
            // 函数参数个数不匹配
            if (error_d(symbol, funcRParamTypes, firstChild->token->row))
            {
                // 函数参数类型不匹配
                error_e(symbol, funcRParamTypes, firstChild->token->row);
            }
        }
        // 只要表达式中有函数 其返回值一定为int或char 所以直接返回int
        return Int;
    }
    return var_null;
}

// 函数实参表 FuncRParams → Exp { ',' Exp }
vector<var_type> analyze_FuncRParams(Node *root)
{
    vector<var_type> FuncRParamTypes;
    for (Node *child : root->childs)
    {
        if (child->type == Exp)
        {
            FuncRParamTypes.push_back(analyze_Exp(child));
        }
    }
    return FuncRParamTypes;
}

// 乘除模表达式 MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
var_type analyze_MulExp(Node *root)
{
    var_type type;
    for (Node *child : root->childs)
    {
        if (child->type == UnaryExp)
        {
            type = analyze_UnaryExp(child);
        }
    }
    return type;
}

// 加减表达式 AddExp → MulExp | AddExp ('+' | '−') MulExp
var_type analyze_AddExp(Node *root)
{
    var_type type;
    for (Node *child : root->childs)
    {
        if (child->type == MulExp)
        {
            type = analyze_MulExp(child);
        }
    }
    return type;
}

// 常量表达式 ConstExp → AddExp 注：使用的 Ident 必须是常量
var_type analyze_ConstExp(Node *root)
{
    return analyze_AddExp(root->GetFirstChild());
}

void tree_2_symbols()
{
    curScope = 1;
    scopeCnt = 1;
    scopeStack.push_back(1);
    analyze_CompUnit(ROOT);
}
