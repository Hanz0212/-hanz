bool myAssert(bool b, int row, char errType)
{
    if (!b)
    {
        errors.insert(pair<int, char>(row, errType));
    }
    return b;
}

void error_a(const string &input, int &i, string &value, int row)
{
    if (!myAssert(input[i + 1] == input[i], row, 'a'))
        tokens.push_back(new Token(dop_2_tk_type.at(input[i]), value, row));
    else
    {
        value += input[i];
        tokens.push_back(new Token(dop_2_tk_type.at(input[i]), value, row));
        i++;
    }
}

// 名字重定义
// 若非重定义 执行add_symbol 否则 error
bool error_b(string name, int row)
{
    if (scope2symbols.find(scopeStack.back()) == scope2symbols.end())
        return false;
    for (Symbol *definedSymbol : scope2symbols.at(scopeStack.back()))
    {
        if (!myAssert(definedSymbol->GetIdentName() != name, row, 'b'))
        {
            return true;
        }
    }
    return false;
}

// 未定义的名字
// 若有定义 返回Symbol 否则返回NULL
Symbol *error_c(Token *token)
{
    Symbol *symbol;
    for (auto iter = scopeStack.rbegin(); iter != scopeStack.rend(); ++iter)
    {
        int scope = *iter;
        if (scope2symbols.find(scope) == scope2symbols.end())
            continue;
        for (Symbol *definedSymbol : scope2symbols.at(scope))
        {
            if (definedSymbol->GetIdentName() == token->value)
            {
                return definedSymbol;
            }
        }
    }
    myAssert(false, token->row, 'c');
    return NULL;
}

// 函数参数个数不匹配
bool error_d(Symbol *symbol, vector<var_type> funcRParamTypes, int row)
{
    vector<Symbol *> funcFParamSymbols = dynamic_cast<FuncSymbol *>(symbol)->funcFParamSymbols;
    int paramCnt = funcFParamSymbols.size();
    int realCnt = funcRParamTypes.size();
    return myAssert(realCnt == paramCnt, row, 'd');
}

// 函数参数类型不匹配 // 已经保证数量相同
void error_e(Symbol *symbol, vector<var_type> funcRParamTypes, int row)
{
    vector<Symbol *> funcFParamSymbols = dynamic_cast<FuncSymbol *>(symbol)->funcFParamSymbols;
    for (int i = 0; i < funcFParamSymbols.size(); i++)
    {
        if (!isMatcherType(funcFParamSymbols.at(i)->type, funcRParamTypes.at(i)))
        {
            myAssert(false, row, 'e');
            return;
        }
    }
}

void error_f(int row, bool hasExp)
{
    myAssert(!hasExp && inVoidFunc || !inVoidFunc, row, 'f');
}

// 有返回值的函数缺少return语句
void error_g(var_type type, Node *block)
{
    if (type == VoidFunc)
        return;
    int row = block->GetLastChild()->token->row;
    Node *child = block->GetChildAt(block->GetChildsSize() - 2);
    myAssert(child->type == BlockItem && child->GetFirstChild()->type == Stmt &&
                 child->GetFirstChild()->GetFirstChild()->token->type == RETURNTK,
             row, 'g');
}

// 不能改变常量的值
void error_h(var_type type, int row)
{
    myAssert(!isConstType(type), row, 'h');
}

void error_i(Node *root)
{
    if (myAssert(tokens.at(loc)->type == SEMICN, tokens.at(loc - 1)->row, 'i'))
        root->AddChild(new Node(TERMINAL, tokens.at(loc++)));                                // ;
    else                                                                                     // 报错 i
        root->AddChild(new Node(TERMINAL, new Token(SEMICN, ";", tokens.at(loc - 1)->row))); // 制造一个分号，行号和上一个非终结符相同
}

void error_j(Node *root)
{
    if (myAssert(tokens.at(loc)->type == RPARENT, tokens.at(loc - 1)->row, 'j'))
        root->AddChild(new Node(TERMINAL, tokens.at(loc++)));                                 // )
    else                                                                                      // 报错 j
        root->AddChild(new Node(TERMINAL, new Token(RPARENT, ")", tokens.at(loc - 1)->row))); // 制造一个右小括号，行号和上一个非终结符相同
}

void error_k(Node *root)
{
    if (myAssert(tokens.at(loc)->type == RBRACK, tokens.at(loc - 1)->row, 'k'))
        root->AddChild(new Node(TERMINAL, tokens.at(loc++)));                                // ]
    else                                                                                     // 报错 k
        root->AddChild(new Node(TERMINAL, new Token(RBRACK, "]", tokens.at(loc - 1)->row))); // 制造一个右中括号，行号和上一个非终结符相同
}

void error_l(string str, int expCnt, int row)
{
    int cnt = 0;
    for (int i = 0; i < str.length() - 1; i++)
    {
        if (str[i] == '%')
        {
            if (str[i + 1] == 'd' || str[i + 1] == 'c')
            {
                cnt++;
                i++;
            }
        }
    }
    myAssert(cnt == expCnt, row, 'l');
}

// 在非循环块中使用break和continue语句
void error_m(int row)
{
    myAssert(loopStack > 0, row, 'm');
}