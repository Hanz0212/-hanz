
void cal_symbols()
{
    // 只填充Const类型和func类型的符号表！！！！！
    for (Symbol *symbol : symbols)
    {
        curScope = symbol->scope;
        if (isArrayType(symbol->type))
        {
            cal_array_symbol(symbol);
        }
        else if (isVarType(symbol->type))
        {
            cal_var_symbol(symbol);
        }
        else if (isFuncType(symbol->type))
        {
            cal_func_symbol(symbol);
        }
        caledSymbols.insert(symbol);
    }
    caledSymbols.clear();
}

void cal_array_symbol(Symbol *symbol)
{
    ArraySymbol *arraySymbol = dynamic_cast<ArraySymbol *>(symbol);
    if (arraySymbol->lengthNode != NULL)
    {
        arraySymbol->length = cal_Exp(arraySymbol->lengthNode);
    }
    bool isConst = isConstType(symbol->type);
    if (arraySymbol->initValNode != NULL && (isConst ||
                                             symbol->scope == 1 ||
                                             arraySymbol->initValNode->GetFirstChild()->token->type == STRCON))
    {
        Node *firstChild = arraySymbol->initValNode->GetFirstChild();
        if (firstChild->token->type == STRCON)
        {
            string str = firstChild->token->value.substr(1, firstChild->token->value.length() - 2);
            int len = str.length();
            for (int i = 0; i < len; i++)
            {
                if (str.at(i) == '\\' && i + 1 < len && str.at(i + 1) == 'n')
                {
                    arraySymbol->initVal.push_back('\n');
                    i++;
                }
                else if (str.at(i) == '\\' && i + 1 < len && str.at(i + 1) == '0')
                {
                    arraySymbol->initVal.push_back('\0');
                    i++;
                }
                else
                    arraySymbol->initVal.push_back(int(str.at(i)));
            }
        }
        else
        {
            for (Node *child : arraySymbol->initValNode->childs)
            {
                if (child->type == ConstExp || child->type == Exp)
                {
                    Node *add = child->GetFirstChild();
                    Node *mul = add->GetFirstChild();
                    Node *unary = mul->GetFirstChild();
                    Node *pri = unary->GetFirstChild();
                    if (pri->type == PrimaryExp || pri->type == UnaryOp)
                        arraySymbol->initVal.push_back(cal_Exp(child));
                }
            }
        }
    }
}

void cal_var_symbol(Symbol *symbol)
{
    // 全局变量的初值表达式也必须是常量表达式 ConstExp
    // 只能解析全局变量或者const变量
    if (symbol->scope != 1 && !isConstType(symbol->type))
        return;
    VarSymbol *varStruct = dynamic_cast<VarSymbol *>(symbol);
    if (varStruct->initValNode != NULL)
    {
        varStruct->initVal = cal_Exp(varStruct->initValNode->GetFirstChild());
    }
}

void cal_func_symbol(Symbol *symbol)
{
    FuncSymbol *funcStruct = dynamic_cast<FuncSymbol *>(symbol);
    switch (funcStruct->returnTypeNode->GetFirstChild()->token->type)
    {
    case INTTK:
        funcStruct->returnType = IntFunc;
        break;
    case CHARTK:
        funcStruct->returnType = CharFunc;
        break;
    case VOIDTK:
        funcStruct->returnType = VoidFunc;
        break;
    }
}

long long cal_Exp(Node *root)
{
    long long ans = cal_AddExp(root->GetFirstChild());
    return ans;
}

long long cal_AddExp(Node *root)
{
    int result = 0, len = root->childs.size();
    tk_type op = PLUS;
    for (int i = 0; i < len; i += 2)
    {
        Node *child = root->GetChildAt(i);
        result = cal_2op(op, result, cal_MulExp(child));
        if (i + 1 < len)
            op = root->GetChildAt(i + 1)->token->type;
    }
    return result;
}

long long cal_MulExp(Node *root)
{
    int result = 1, len = root->childs.size();
    tk_type op = MULT;
    for (int i = 0; i < len; i += 2)
    {
        Node *child = root->GetChildAt(i);
        result = cal_2op(op, result, cal_UnaryExp(child));
        if (i + 1 < len)
            op = root->GetChildAt(i + 1)->token->type;
    }
    return result;
}

long long cal_UnaryExp(Node *root)
{

    // 保证了constExp不包括函数调用元素
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == PrimaryExp)
    {
        return cal_PrimaryExp(firstChild);
    }
    else if (firstChild->type == UnaryOp)
    {
        tk_type op = firstChild->GetFirstChild()->token->type;
        int result = cal_UnaryExp(root->GetChildAt(1));
        return op == PLUS ? result : -result;
    }
    else
    {
        cout << "constExp cant contain func()" << endl;
        exit(1);
    }
    return 0;
}

long long cal_PrimaryExp(Node *root)
{
    Node *firstChild = root->GetFirstChild();
    switch (firstChild->type)
    {
    case Character:
        return cal_Character(firstChild);
    case Number:
        return cal_Number(firstChild);
    case LVal:
        return cal_LVal(firstChild);
    default:
        return cal_Exp(root->GetChildAt(1));
    }
}

long long cal_Character(Node *root)
{
    string s = root->GetFirstChild()->token->value;
    if (s.size() > 3)
    {
        return escapeChars.at(s[2]);
    }
    else
        return s[1];
}

long long cal_Number(Node *root)
{
    return stoll(root->GetFirstChild()->token->value);
}

// 进入这个函数前一定要先设置好curScope
long long cal_LVal(Node *root)
{
    Symbol *symbol = find_symbol(root->GetFirstChild()->token);
    if (isArrayType(symbol->type))
    {
        ArraySymbol *arraySymbol = dynamic_cast<ArraySymbol *>(symbol);
        int offset = cal_Exp(root->GetChildAt(2));
        return arraySymbol->getInitValAt(offset);
    }

    VarSymbol *varStruct = dynamic_cast<VarSymbol *>(symbol);
    return varStruct->initVal;
}

// 进入这个函数前一定要先设置好curScope
Symbol *find_symbol(Token *token, bool isDeclare)
{
    int varRow = token->row;
    string varName = token->value;
    for (int scope = curScope; scope != 0; scope = fatherScope.at(scope))
    {
        if (scope2symbols.find(scope) == scope2symbols.end())
            continue;
        for (Symbol *symbol : scope2symbols.at(scope))
        {
            if (symbol->token->value == varName)
            {
                if (symbol->token->row == varRow)
                {
                    // 如果想找的符号的行号和symbol中的行号相同，有两种情况：
                    // 1. 现在正generate到decl，相同是必然的
                    // 2. 现在不是decl，是在将token作为右值分析，而赋值操作和token的声明位置恰好在同一行
                    // 对于第一种情况，直接返回即可。对于第二种情况，只有当声明位置在赋值之前才能返回
                    // 可以用symbol是否位于syb_2_llvm中来判断是否已经分析过赋值
                    if (isDeclare || syb_2_llvm.find(symbol) != syb_2_llvm.end() || caledSymbols.count(symbol) != 0)
                    {
                        return symbol;
                    }
                    else
                        break;
                }
                else if (symbol->token->row < varRow)
                {
                    return symbol;
                }
            }
        }
    }
    cout << "cant find var in symbols!: " << *token << endl;
    exit(1);
    return NULL;
}
