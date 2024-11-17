
// 声明 Decl → ConstDecl | VarDecl
Node *parse_Decl()
{
    Node *root = new Node(Decl);
    if (FIRST.at(VarDecl).count(tokens.at(loc)->type) != 0)
        root->AddChild(parse_VarDecl()); // VarDecl
    else
        root->AddChild(parse_ConstDecl()); // ConstDecl
    return root;
}

// 常量声明 ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';' // i
Node *parse_ConstDecl()
{
    Node *root = new Node(ConstDecl);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // const
    root->AddChild(parse_BType());                        // BType
    root->AddChild(parse_ConstDef());                     // ConstDef
    while (tokens.at(loc)->type == COMMA)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // ,
        root->AddChild(parse_ConstDef());                     // ConstDef
    }
    error_i(root); // ;
    return root;
}

// 基本类型 BType → 'int' | 'char'
Node *parse_BType()
{
    Node *root = new Node(BType);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // int | char
    return root;
}

// 常量定义 ConstDef → Ident [ '[' ConstExp ']' ] '=' ConstInitVal // k
Node *parse_ConstDef()
{
    Node *root = new Node(ConstDef);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // Ident
    if (tokens.at(loc)->type == LBRACK)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // [
        root->AddChild(parse_ConstExp());                     // ConstExp
        error_k(root);                                        // ]
    }
    if (tokens.at(loc)->type == ASSIGN)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // =
        root->AddChild(parse_ConstInitVal());                 // ConstInitVal
    }
    return root;
}

// 常量初值 ConstInitVal → ConstExp | '{' [ ConstExp { ',' ConstExp } ] '}' | StringConst
Node *parse_ConstInitVal()
{
    Node *root = new Node(ConstInitVal);
    if (tokens.at(loc)->type == LBRACE)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // {
        if (tokens.at(loc)->type != RBRACE)
        {
            root->AddChild(parse_ConstExp()); // ConstExp
            while (tokens.at(loc)->type == COMMA)
            {
                root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // ,
                root->AddChild(parse_ConstExp());                     // ConstExp
            }
        }
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // }
    }
    else if (tokens.at(loc)->type == STRCON)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // StringConst
    }
    else
    {
        root->AddChild(parse_ConstExp()); // ConstExp
    }
    return root;
}

// 变量声明 VarDecl → BType VarDef { ',' VarDef } ';' // i
Node *parse_VarDecl()
{
    Node *root = new Node(VarDecl);
    root->AddChild(parse_BType());  // BType
    root->AddChild(parse_VarDef()); // VarDef
    while (tokens.at(loc)->type == COMMA)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // ,
        root->AddChild(parse_VarDef());                       // VarDef
    }
    error_i(root); // ;
    return root;
}

// 变量定义 VarDef → Ident [ '[' ConstExp ']' ] | Ident [ '[' ConstExp ']' ] '=' InitVal // k
Node *parse_VarDef()
{
    Node *root = new Node(VarDef);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // Ident
    if (tokens.at(loc)->type == LBRACK)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // [
        root->AddChild(parse_ConstExp());                     // ConstExp
        error_k(root);                                        // ]
    }
    if (tokens.at(loc)->type == ASSIGN)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // =
        root->AddChild(parse_InitVal());                      // InitVal
    }
    return root;
}

// 变量初值 InitVal → Exp | '{' [ Exp { ',' Exp } ] '}' | StringConst
Node *parse_InitVal()
{
    Node *root = new Node(InitVal);
    if (tokens.at(loc)->type == LBRACE)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // {
        if (tokens.at(loc)->type != RBRACE)
        {
            root->AddChild(parse_Exp()); // Exp
            while (tokens.at(loc)->type == COMMA)
            {
                root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // ,
                root->AddChild(parse_Exp());                          // Exp
            }
        }
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // }
    }
    else if (tokens.at(loc)->type == STRCON)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // StringConst
    }
    else
    {
        root->AddChild(parse_Exp()); // Exp
    }
    return root;
}

// 函数定义 FuncDef → FuncType Ident '(' [FuncFParams] ')' Block // j
Node *parse_FuncDef()
{
    Node *root = new Node(FuncDef);
    root->AddChild(parse_FuncType());                     // FuncType
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // Ident
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // (
    if (tokens.at(loc)->type == INTTK || tokens.at(loc)->type == CHARTK)
        root->AddChild(parse_FuncFParams()); // FuncFParams
    error_j(root);                           // )
    root->AddChild(parse_Block());           // Block
    return root;
}

// 主函数定义 MainFuncDef → 'int' 'main' '(' ')' Block // j
Node *parse_MainFuncDef()
{
    Node *root = new Node(MainFuncDef);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // int
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // main
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // (
    error_j(root);                                        // )
    root->AddChild(parse_Block());                        // Block
    return root;
}

// 函数类型 FuncType → 'void' | 'int' | 'char'
Node *parse_FuncType()
{
    Node *root = new Node(FuncType);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // void | int | char
    return root;
}

// 函数形参表 FuncFParams → FuncFParam { ',' FuncFParam }
Node *parse_FuncFParams()
{
    Node *root = new Node(FuncFParams);
    root->AddChild(parse_FuncFParam()); // FuncFParam
    while (tokens.at(loc)->type == COMMA)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // ,
        root->AddChild(parse_FuncFParam());                   // FuncFParam
    }
    return root;
}

// 函数形参 FuncFParam → BType Ident ['[' ']'] // k
Node *parse_FuncFParam()
{
    Node *root = new Node(FuncFParam);
    root->AddChild(parse_BType());                        // BType
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // Ident
    if (tokens.at(loc)->type == LBRACK)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // [
        error_k(root);                                        // ]
    }
    return root;
}

// 语句块 Block → '{' { BlockItem } '}'
Node *parse_Block()
{
    Node *root = new Node(Block);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // {
    while (tokens.at(loc)->type != RBRACE)
    {
        root->AddChild(parse_BlockItem()); // BlockItem
    }
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // }
    return root;
}

// 语句块项 BlockItem → Decl | Stmt
Node *parse_BlockItem()
{
    Node *root = new Node(BlockItem);
    if (tokens.at(loc)->type == CONSTTK || tokens.at(loc)->type == INTTK || tokens.at(loc)->type == CHARTK)
    {
        root->AddChild(parse_Decl()); // Decl
    }
    else
    {
        root->AddChild(parse_Stmt()); // Stmt
    }
    return root;
}

Node *parse_Stmt()
{
    Node *root = new Node(Stmt);
    // 'if' '(' Cond ')' Stmt [ 'else' Stmt ] // j
    if (tokens.at(loc)->type == IFTK)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // if
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // (
        root->AddChild(parse_Cond());                         // Cond
        error_j(root);                                        // )
        root->AddChild(parse_Stmt());                         // Stmt
        if (tokens.at(loc)->type == ELSETK)
        {
            root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // else
            root->AddChild(parse_Stmt());                         // Stmt
        }
    }
    // 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt
    else if (tokens.at(loc)->type == FORTK)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // for
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // (
        if (tokens.at(loc)->type != SEMICN)
            root->AddChild(parse_ForStmt());                  // ForStmt
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // ;
        if (tokens.at(loc)->type != SEMICN)
            root->AddChild(parse_Cond());                     // Cond
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // ;
        if (tokens.at(loc)->type != RPARENT)
            root->AddChild(parse_ForStmt());                  // ForStmt
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // )
        root->AddChild(parse_Stmt());                         // Stmt
    }
    // 'break' ';' | 'continue' ';' // i
    else if (tokens.at(loc)->type == BREAKTK || tokens.at(loc)->type == CONTINUETK)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // break | continue
        error_i(root);                                        // ;
    }
    // 'return' [Exp] ';' // i
    else if (tokens.at(loc)->type == RETURNTK)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // return
        if (FIRST.at(Exp).count(tokens.at(loc)->type) != 0)
            root->AddChild(parse_Exp()); // Exp
        error_i(root);                   // ;
    }
    // 'printf''('StringConst {','Exp}')'';' // i j
    else if (tokens.at(loc)->type == PRINTFTK)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // printf
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // (
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // StringConst
        while (tokens.at(loc)->type == COMMA)
        {
            root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // ,
            root->AddChild(parse_Exp());                          // Exp
        }
        error_j(root); // )
        error_i(root); // ;
    }
    else if (tokens.at(loc)->type == LBRACE)
    {                                  // Block
        root->AddChild(parse_Block()); // Block
    }
    else // 开始区分 Exp 和 LVal =
    {
        bool isExp = true; // 初始默认为 Exp, 首个 token 为 Ident 才有机会逆转
        if (tokens.at(loc)->type == IDENFR)
        { // 尝试阶段开始
            int save_loc = loc;
            parse_LVal(); // LVal
            isExp = tokens.at(loc)->type != ASSIGN;
            loc = save_loc;
        } // 尝试阶段结束
          // [Exp] ';' // i
        if (isExp)
        {
            if (FIRST.at(Exp).count(tokens.at(loc)->type) != 0)
                root->AddChild(parse_Exp()); // Exp
        }
        else
        {
            root->AddChild(parse_LVal());                         // LVal
            root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // =
            // LVal '=' 'getint''('')'';' | LVal '=' 'getchar''('')'';' // i j
            if (tokens.at(loc)->type == GETINTTK || tokens.at(loc)->type == GETCHARTK)
            {
                root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // getint | getchar
                root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // (
                error_j(root);                                        // )
            }
            //  LVal '=' Exp ';' // i
            else
            {
                root->AddChild(parse_Exp()); // Exp
            }
        }
        error_i(root); // ;
    }
    return root;
}

// 语句 ForStmt → LVal '=' Exp
Node *parse_ForStmt()
{
    Node *root = new Node(ForStmt);
    root->AddChild(parse_LVal());                         // LVal
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // =
    root->AddChild(parse_Exp());                          // Exp
    return root;
}

// 表达式 Exp → AddExp
Node *parse_Exp()
{
    Node *root = new Node(Exp);
    root->AddChild(parse_AddExp()); // AddExp
    return root;
}

// 条件表达式 Cond → LOrExp
Node *parse_Cond()
{
    Node *root = new Node(Cond);
    root->AddChild(parse_LOrExp()); // LOrExp
    return root;
}

// 左值表达式 LVal → Ident ['[' Exp ']'] // k
Node *parse_LVal()
{
    Node *root = new Node(LVal);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // Ident
    if (tokens.at(loc)->type == LBRACK)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // [
        root->AddChild(parse_Exp());                          // Exp
        error_k(root);                                        // ]
    }
    return root;
}

// 基本表达式 PrimaryExp → '(' Exp ')' | LVal | Number | Character// j
Node *parse_PrimaryExp()
{
    Node *root = new Node(PrimaryExp);
    if (tokens.at(loc)->type == INTCON)
    {
        root->AddChild(parse_Number()); // Number
    }
    else if (tokens.at(loc)->type == CHRCON)
    {
        root->AddChild(parse_Character()); // Character
    }
    else if (tokens.at(loc)->type == IDENFR)
    {
        root->AddChild(parse_LVal()); // LVal
    }
    else
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // (
        root->AddChild(parse_Exp());                          // Exp
        error_j(root);                                        // )
    }
    return root;
}

// 数值 Number → IntConst
Node *parse_Number()
{
    Node *root = new Node(Number);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // IntConst
    return root;
}

// 字符 Character → CharConst
Node *parse_Character()
{
    Node *root = new Node(Character);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // CharConst
    return root;
}

// 一元表达式 UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp // j
Node *parse_UnaryExp()
{
    Node *root = new Node(UnaryExp);
    if (FIRST.at(UnaryOp).count(tokens.at(loc)->type) != 0)
    {
        root->AddChild(parse_UnaryOp());  // UnaryOp
        root->AddChild(parse_UnaryExp()); // UnaryExp
    }
    else if (tokens.at(loc)->type == IDENFR && tokens.at(loc + 1)->type == LPARENT)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // Ident
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // (
        if (FIRST.at(Exp).count(tokens.at(loc)->type) != 0)
            root->AddChild(parse_FuncRParams()); // FuncRParams
        error_j(root);                           // )
    }
    else
    {
        root->AddChild(parse_PrimaryExp()); // PrimaryExp
    }
    return root;
}

// 单目运算符 UnaryOp → '+' | '−' | '!' 注：'!'仅出现在条件表达式中
Node *parse_UnaryOp()
{
    Node *root = new Node(UnaryOp);
    root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // + | - | !
    return root;
}

// 函数实参表 FuncRParams → Exp { ',' Exp }
Node *parse_FuncRParams()
{
    Node *root = new Node(FuncRParams);
    root->AddChild(parse_Exp()); // Exp
    while (tokens.at(loc)->type == COMMA)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // ,
        root->AddChild(parse_Exp());                          // Exp
    }
    return root;
}

void rebuild_tree(Node *root, syn_type type)
{
    return;
    if (root->GetChildsSize() <= 1)
        return; // 不需重建
    Node *p = root;
    do
    {
        p->AddFisrtChild(root->PopChild());            // XXXExp
        p->AddFisrtChild(root->PopChild());            // 'X' | 'X' | 'X' | ....
        p->AddFisrtChild(new Node(type, new Token())); // XXXExp
        p = p->GetFirstChild();
    } while (root->GetChildsSize() > 4);
    p->AddFisrtChild(root->PopChild()); // XXXExp
}

// 乘除模表达式 MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
Node *parse_MulExp()
{
    Node *root = new Node(MulExp);
    root->AddChild(parse_UnaryExp()); // UnaryExp
    while (tokens.at(loc)->type == MULT || tokens.at(loc)->type == DIV || tokens.at(loc)->type == MOD)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // '*' | '/' | '%'
        root->AddChild(parse_UnaryExp());                     // UnaryExp
    }
    rebuild_tree(root, MulExp);
    return root;
}

// 加减表达式 AddExp → MulExp | AddExp ('+' | '−') MulExp
Node *parse_AddExp()
{
    Node *root = new Node(AddExp);
    root->AddChild(parse_MulExp()); // MulExp
    while (tokens.at(loc)->type == PLUS || tokens.at(loc)->type == MINU)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // '+' | '-'
        root->AddChild(parse_MulExp());                       // MulExp
    }
    rebuild_tree(root, AddExp);
    return root;
}

// 关系表达式 RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
Node *parse_RelExp()
{
    Node *root = new Node(RelExp);
    root->AddChild(parse_AddExp()); // AddExp
    while (tokens.at(loc)->type == LSS || tokens.at(loc)->type == GRE || tokens.at(loc)->type == LEQ || tokens.at(loc)->type == GEQ)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // '<' | '>' | '<=' | '>='
        root->AddChild(parse_AddExp());                       // AddExp
    }
    rebuild_tree(root, RelExp);
    return root;
}

// 相等性表达式 EqExp → RelExp | EqExp ('==' | '!=') RelExp
Node *parse_EqExp()
{
    Node *root = new Node(EqExp);
    root->AddChild(parse_RelExp()); // RelExp
    while (tokens.at(loc)->type == EQL || tokens.at(loc)->type == NEQ)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // '==' | '!='
        root->AddChild(parse_RelExp());                       // RelExp
    }
    rebuild_tree(root, EqExp);
    return root;
}

// 逻辑与表达式 LAndExp → EqExp | LAndExp '&&' EqExp
Node *parse_LAndExp()
{
    Node *root = new Node(LAndExp);
    root->AddChild(parse_EqExp()); // EqExp
    while (tokens.at(loc)->type == AND)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // '&&'
        root->AddChild(parse_EqExp());                        // EqExp
    }
    rebuild_tree(root, LAndExp);
    return root;
}

// 逻辑或表达式 LOrExp → LAndExp | LOrExp '||' LAndExp
Node *parse_LOrExp()
{
    Node *root = new Node(LOrExp);
    root->AddChild(parse_LAndExp()); // LAndExp
    while (tokens.at(loc)->type == OR)
    {
        root->AddChild(new Node(TERMINAL, tokens.at(loc++))); // '||'
        root->AddChild(parse_LAndExp());                      // LAndExp
    }
    rebuild_tree(root, LOrExp);
    return root;
}

// 常量表达式 ConstExp → AddExp 注：使用的 Ident 必须是常量
Node *parse_ConstExp()
{
    Node *root = new Node(ConstExp);
    root->AddChild(parse_AddExp()); // AddExp
    return root;
}

void token_2_tree()
{
    ROOT = new Node(CompUnit);
    while (tokens.at(loc + 2)->type != LPARENT)
        ROOT->AddChild(parse_Decl());
    while (tokens.at(loc + 1)->type != MAINTK)
        ROOT->AddChild(parse_FuncDef());
    ROOT->AddChild(parse_MainFuncDef());
}
