LLVM *generate_midCode(LLVM *llvm)
{
    if (llvm->midType == G_STR)
        strCodes.push_back(llvm);
    else
        midCodes.push_back(llvm);
    return llvm;
}

// 编译单元 CompUnit → {Decl} {FuncDef} MainFuncDef
void generate_CompUnit(Node *root)
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
        generate_Decl(root->GetChildAt(i), true);
    }

    // 函数分析
    for (int i = key; i < childSize; i++)
    {
        if (root->GetChildAt(i)->type != FuncDef)
        {
            break;
        }
        generate_FuncDef(root->GetChildAt(i));
    }
    Node *mainFuncDef = root->GetLastChild();
    generate_midCode(new GDefLLVM(IntFunc, "main", vector<Symbol *>()));
    curFuncType = IntFunc;
    curFuncReturnLabel = new LabelLLVM();
    curFuncReturnVar = generate_midCode(new AllocaLLVM(Int));
    generate_Block(mainFuncDef->GetChildAt(4));
    generate_midCode(curFuncReturnLabel);
    generate_midCode(new RetLLVM(curFuncType, generate_midCode(new LoadLLVM(curFuncType, curFuncReturnVar))));
    generate_midCode(new FuncEndLLVM());
}

// 声明 Decl → ConstDecl | VarDecl
void generate_Decl(Node *root, bool isGlobal)
{
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == ConstDecl)
    {
        generate_ConstDecl(firstChild, isGlobal);
    }
    else
    {
        generate_VarDecl(firstChild, isGlobal);
    }
}

// 常量声明 ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
void generate_ConstDecl(Node *root, bool isGlobal)
{
    int childSize = root->GetChildsSize();
    for (int i = 2; i < childSize; i++)
    {
        if (root->GetChildAt(i)->type == ConstDef)
        {
            generate_ConstDef(root->GetChildAt(i), isGlobal);
        }
    }
}

// 常量定义 ConstDef → Ident [ '[' ConstExp ']' ] '=' ConstInitVal // b
void generate_ConstDef(Node *root, bool isGlobal)
{
    Token *token = root->GetFirstChild()->token;
    Symbol *symbol = find_symbol(token, true);
    if (isArrayType(symbol->type))
    {
        ArraySymbol *arraySymbol = dynamic_cast<ArraySymbol *>(symbol);
        if (isGlobal)
        {
            LLVM *llvm = generate_midCode(new GDefLLVM(symbol->type,
                                                       symbol->GetIdentName(),
                                                       arraySymbol->initVal,
                                                       arraySymbol->length));
            syb_2_llvm_insert(symbol, llvm);
        }
        else
        {
            LLVM *alloca = generate_midCode(new AllocaLLVM(symbol->type, arraySymbol->length));
            int len = arraySymbol->initVal.size();
            for (int i = 0; i < len; i++)
            {
                LLVM *array = generate_midCode(new GetelementptrLLVM(symbol->type,
                                                                     arraySymbol->length,
                                                                     alloca,
                                                                     new ConstLLVM(i)));
                generate_midCode(new StoreLLVM(symbol->type, new ConstLLVM(arraySymbol->initVal.at(i)), array));
            }
            syb_2_llvm_insert(symbol, alloca);
        }
    }
    // 局部的非数组常量不用考虑
}

// 变量声明 VarDecl → BType VarDef { ',' VarDef } ';'
void generate_VarDecl(Node *root, bool isGlobal)
{
    int childSize = root->GetChildsSize();
    for (int i = 1; i < childSize; i++)
    {
        if (root->GetChildAt(i)->type == VarDef)
        {
            generate_VarDef(root->GetChildAt(i), isGlobal);
        }
    }
}

// 变量定义 VarDef → Ident [ '[' ConstExp ']' ] | Ident [ '[' ConstExp ']' ] '=' InitVal // b
void generate_VarDef(Node *root, bool isGlobal)
{
    Token *token = root->GetFirstChild()->token;
    Symbol *symbol = find_symbol(token, true);
    if (isGlobal)
    {
        if (isArrayType(symbol->type))
        {
            ArraySymbol *arraySymbol = dynamic_cast<ArraySymbol *>(symbol);
            LLVM *llvm = generate_midCode(new GDefLLVM(symbol->type,
                                                       symbol->GetIdentName(), arraySymbol->initVal,
                                                       arraySymbol->length));
            syb_2_llvm_insert(symbol, llvm);
        }
        else
        {
            vector<int> initValList = {dynamic_cast<VarSymbol *>(symbol)->initVal};
            LLVM *llvm = generate_midCode(new GDefLLVM(symbol->type, symbol->GetIdentName(), initValList, 1));
            syb_2_llvm_insert(symbol, llvm);
        }
    }
    else
    {
        if (isVarType(symbol->type))
        {
            LLVM *alloca = generate_midCode(new AllocaLLVM(symbol->type));
            syb_2_llvm_insert(symbol, alloca);
            Node *initValNode = dynamic_cast<VarSymbol *>(symbol)->initValNode;
            if (initValNode != NULL)
            {
                Node *firstChild = initValNode->GetFirstChild();
                LLVM *exp = generate_Exp(firstChild);
                if (!isIntType(symbol->type)) // 向char赋值 trunc
                {
                    if (exp->midType != CONST_IR)
                        exp = generate_midCode(new TruncLLVM(exp));
                    else
                    {
                        int val = dynamic_cast<ConstLLVM *>(exp)->val & 0xff;
                        exp = new ConstLLVM(val);
                    }
                }
                LLVM *store = generate_midCode(new StoreLLVM(symbol->type, exp, alloca));
            }
        }
        else // array
        {
            ArraySymbol *arraySymbol = dynamic_cast<ArraySymbol *>(symbol);
            LLVM *alloca = generate_midCode(new AllocaLLVM(symbol->type, arraySymbol->length));
            syb_2_llvm_insert(symbol, alloca);
            if (arraySymbol->initValNode != NULL)
            {
                if (arraySymbol->initValNode->GetFirstChild()->token->type == STRCON)
                {
                    int cnt = arraySymbol->initVal.size();
                    for (int i = 0; i < cnt; i++)
                    {
                        char c = arraySymbol->initVal.at(i);
                        LLVM *array = generate_midCode(new GetelementptrLLVM(symbol->type,
                                                                             arraySymbol->length,
                                                                             alloca,
                                                                             new ConstLLVM(i)));
                        generate_midCode(new StoreLLVM(symbol->type, new ConstLLVM(c), array));
                    }
                    if (cnt < arraySymbol->length)
                    {
                        LLVM *array = generate_midCode(new GetelementptrLLVM(symbol->type,
                                                                             arraySymbol->length,
                                                                             alloca,
                                                                             new ConstLLVM(cnt)));
                        generate_midCode(new StoreLLVM(symbol->type, new ConstLLVM(0), array));
                    }
                }
                else
                {
                    int cnt = (arraySymbol->initValNode->GetChildsSize() - 1) / 2;
                    for (int i = 0; i < cnt; i++)
                    {
                        Node *child = arraySymbol->initValNode->GetChildAt(i * 2 + 1);
                        LLVM *array = generate_midCode(new GetelementptrLLVM(symbol->type,
                                                                             arraySymbol->length,
                                                                             alloca,
                                                                             new ConstLLVM(i)));
                        LLVM *exp = generate_Exp(child);

                        if (!isIntType(symbol->type)) // 向char赋值 trunc
                        {
                            if (exp->midType != CONST_IR)
                                exp = generate_midCode(new TruncLLVM(exp));
                            else
                            {
                                int val = dynamic_cast<ConstLLVM *>(exp)->val & 0xff;
                                exp = new ConstLLVM(val);
                            }
                        }
                        generate_midCode(new StoreLLVM(symbol->type, exp, array));
                    }
                }
            }
        }
    }
}

// 语句块 Block → '{' { BlockItem } '}'
void generate_Block(Node *root)
{
    curScope = node_2_scopeCnt.at(root); // 更新当亲的scope
    for (Node *child : root->childs)
    {
        if (child->type == BlockItem)
        {
            child = child->GetFirstChild();
            if (child->type == Decl)
            {
                generate_Decl(child, false);
            }
            else if (child->type == Stmt)
            {
                bool isjump = generate_Stmt(child);
                if (isjump)
                    break;
            }
        }
    }
    curScope = fatherScope.at(curScope);
}

LLVM *generate_stmt_LVal(Node *LVal)
{
    Symbol *symbol = find_symbol(LVal->GetFirstChild()->token);
    LLVM *des;
    if (isVarType(symbol->type))
    {
        des = syb_2_llvm.at(symbol);
    }
    else
    {
        LLVM *offset = generate_Exp(LVal->GetChildAt(2));
        if (symbol->isFuncArrayParam())
        {
            LLVM *array = generate_midCode(new LoadLLVM(symbol->type, syb_2_llvm.at(symbol)));
            des = generate_midCode(new GetelementptrLLVM(symbol->type, -1, array, offset));
        }
        else
        {
            des = generate_midCode(new GetelementptrLLVM(symbol->type,
                                                         dynamic_cast<ArraySymbol *>(symbol)->length,
                                                         syb_2_llvm.at(symbol),
                                                         offset));
        }
    }
    return des;
}

// stmt
bool generate_Stmt(Node *root)
{
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == Block)
    {
        generate_Block(firstChild);
    }
    else if (firstChild->type == LVal)
    {
        //  LVal '=' Exp ';'
        // LVal → Ident ['[' Exp ']']
        Symbol *symbol = find_symbol(firstChild->GetFirstChild()->token);
        LLVM *des = generate_stmt_LVal(firstChild);
        Node *keyChild = root->GetChildAt(2);
        LLVM *exp;

        if (keyChild->type == Exp)
        {
            exp = generate_Exp(root->GetChildAt(2));
        }
        //   %9 = call i32 @getint(i32 %7, i8* %8)
        else if (keyChild->token->value == "getint")
        {
            exp = generate_midCode(new CallLLVM(IntFunc, "getint", vector<Symbol *>(), vector<LLVM *>()));
            // generate_midCode(StoreLLVM())
        }
        else if (keyChild->token->value == "getchar")
        {
            exp = generate_midCode(new CallLLVM(IntFunc, "getchar", vector<Symbol *>(), vector<LLVM *>()));
        }
        else
        {
            cout << "in generate_stmt: gen LVal failed!!" << endl;
            exit(1);
        }
        if (!isIntType(symbol->type)) // 向char赋值 trunc
        {
            if (exp->midType != CONST_IR)
                exp = generate_midCode(new TruncLLVM(exp));
            else
            {
                int val = dynamic_cast<ConstLLVM *>(exp)->val & 0xff;
                exp = new ConstLLVM(val);
            }
        }
        generate_midCode(new StoreLLVM(symbol->type, exp, des));
    }
    // 'return' [Exp] ';'
    else if (firstChild->token->type == RETURNTK)
    {
        if (root->GetChildAt(1)->type == Exp)
        {
            brkLabels.clear();
            ctnLabels.clear();
            LLVM *exp = generate_Exp(root->GetChildAt(1));
            if (!isIntType(curFuncType)) // 向char赋值 trunc
            {
                if (exp->midType != CONST_IR)
                    exp = generate_midCode(new TruncLLVM(exp));
                else
                {
                    int val = dynamic_cast<ConstLLVM *>(exp)->val & 0xff;
                    exp = new ConstLLVM(val);
                }
            }
            generate_midCode(new StoreLLVM(curFuncType, exp, curFuncReturnVar));
        }
        generate_midCode(new BrLLVM(curFuncReturnLabel));
        return true;
    }
    else if (firstChild->type == Exp)
    {
        generate_Exp(firstChild);
    }
    //  'printf''('StringConst {','Exp}')'';'
    else if (firstChild->token->type == PRINTFTK)
    {
        string str = root->GetChildAt(2)->token->value.substr(1, root->GetChildAt(2)->token->value.size() - 2);
        vector<string> strList;
        vector<int> lenList;
        string tstr;
        int tlen = 0;
        int len = str.length();
        for (int i = 0; i < len; i++)
        {
            if (str[i] == '%' && i + 1 < len && str[i + 1] == 'd')
            {
                if (tstr != "")
                    strList.push_back(tstr);
                tstr = "";
                if (tlen != 0)
                    lenList.push_back(tlen);
                tlen = 0;
                strList.push_back("%d");
                i++;
            }
            else if (str[i] == '%' && i + 1 < len && str[i + 1] == 'c')
            {
                if (tstr != "")
                    strList.push_back(tstr);
                tstr = "";
                if (tlen != 0)
                    lenList.push_back(tlen);
                tlen = 0;
                strList.push_back("%c");
                i++;
            }
            else if (str[i] == '\\' && i + 1 < len && str[i + 1] == 'n')
            {
                tstr += "\\0A";
                i++;
                tlen++;
            }
            else
            {
                tstr += str[i];
                tlen++;
            }
        }
        if (tstr != "")
            strList.push_back(tstr);
        if (tlen != 0)
            lenList.push_back(tlen);

        vector<LLVM *> exps;
        for (Node *child : root->childs)
        {
            if (child->type == Exp)
            {
                exps.push_back(generate_Exp(child));
            }
        }

        for (int i = 0, j = 0, k = 0; i < strList.size(); i++)
        {
            string str = strList.at(i);
            if (str == "%d")
            {
                generate_midCode(new CallLLVM("putint", exps.at(j)));
                j++;
            }
            else if (str == "%c")
            {
                generate_midCode(new CallLLVM("putch", exps.at(j)));
                j++;
            }
            else
            {
                LLVM *gStr = find_GStr(str, lenList.at(k) + 1);
                generate_midCode(new CallLLVM(gStr));
                k++;
            }
        }
    }
    //  'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    else if (firstChild->token->type == IFTK)
    {
        LLVM *trueLabel = new LabelLLVM();  // if 的入口
        LLVM *falseLabel = new LabelLLVM(); // else 入口

        generate_Cond(root->GetChildAt(2), trueLabel, falseLabel); // 解析cond 真则跳到label1 否则label2
        generate_midCode(trueLabel);                               // 生成label1
        generate_Stmt(root->GetChildAt(4));                        // 解析if内的语句块

        if (root->GetChildsSize() > 6)
        {
            LLVM *endLabel = new LabelLLVM();       // 结尾
            generate_midCode(new BrLLVM(endLabel)); // if结束后跳到结尾

            generate_midCode(falseLabel);
            generate_Stmt(root->GetChildAt(6));

            generate_midCode(new BrLLVM(endLabel));
            generate_midCode(endLabel);
        }
        else
        {
            generate_midCode(new BrLLVM(falseLabel)); // if结束后跳到结尾
            generate_midCode(falseLabel);
        }
    }
    // 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt // h
    else if (firstChild->token->type == FORTK)
    {
        if (root->GetChildAt(2)->type == ForStmt)
            generate_ForStmt(root->GetChildAt(2));

        LLVM *label1 = new LabelLLVM(); // cond(若有)或stmt入口
        generate_midCode(new BrLLVM(label1));
        generate_midCode(label1);

        LLVM *labelEnd = new LabelLLVM(); // 结尾
        Node *cond = NULL;
        for (Node *child : root->childs)
            if (child->type == Cond)
                cond = child;

        if (cond) // 生成cond
        {
            LLVM *label2 = new LabelLLVM(); // cond出口 和 stmt入口
            generate_Cond(cond, label2, labelEnd);
            generate_midCode(label2);
        }

        LLVM *label3 = new LabelLLVM(); // stmt 和 forstmt2之间

        ctnLabels.push_back(label3);
        brkLabels.push_back(labelEnd);
        generate_Stmt(root->GetLastChild()); // 生成stmt
        brkLabels.pop_back();
        ctnLabels.pop_back();

        generate_midCode(new BrLLVM(label3));
        generate_midCode(label3);

        Node *forstmt2 = root->GetChildAt(root->GetChildsSize() - 3);
        if (forstmt2->type == ForStmt)
            generate_ForStmt(forstmt2);
        generate_midCode(new BrLLVM(label1));
        generate_midCode(labelEnd);
    }
    else if (firstChild->token->type == BREAKTK)
    {
        generate_midCode(new BrLLVM(brkLabels.back()));
        return true;
    }
    else if (firstChild->token->type == CONTINUETK)
    {
        generate_midCode(new BrLLVM(ctnLabels.back()));
        return true;
    }
    return false;
}

// 语句 ForStmt → LVal '=' Exp // h
void generate_ForStmt(Node *root)
{
    Node *firstChild = root->GetFirstChild();
    Symbol *symbol = find_symbol(firstChild->GetFirstChild()->token);
    LLVM *des = generate_stmt_LVal(firstChild);
    LLVM *exp = generate_Exp(root->GetChildAt(2));
    if (!isIntType(symbol->type)) // 向char赋值 trunc
    {
        if (exp->midType != CONST_IR)
            exp = generate_midCode(new TruncLLVM(exp));
        else
        {
            int val = dynamic_cast<ConstLLVM *>(exp)->val & 0xff;
            exp = new ConstLLVM(val);
        }
    }
    generate_midCode(new StoreLLVM(symbol->type, exp, des));
}

// 函数定义 FuncDef → FuncType Ident '(' [FuncFParams] ')' Block // b g
void generate_FuncDef(Node *root)
{
    Token *token = root->GetChildAt(1)->token;
    Symbol *symbol = find_symbol(token, true);
    FuncSymbol *funcSymbol = dynamic_cast<FuncSymbol *>(symbol);
    vector<Symbol *> funcFParams = funcSymbol->funcFParamSymbols;

    generate_midCode(new GDefLLVM(symbol->type, symbol->GetIdentName(), funcFParams));

    curScope = node_2_scopeCnt.at(root); // 更新当亲的scope
    curFuncType = funcSymbol->returnType;
    curFuncReturnLabel = new LabelLLVM();
    curFuncReturnVar = NULL;
    if (curFuncType != VoidFunc) // 用来存返回值
        curFuncReturnVar = generate_midCode(new AllocaLLVM(curFuncType == IntFunc ? Int : Char));

    vector<LLVM *> tempList1;
    vector<LLVM *> tempList2;
    for (int i = 0; i < funcFParams.size(); i++)
    {
        var_type type = funcFParams.at(i)->type;
        LLVM *funcFParam = new FuncFParamLLVM(type, i);
        LLVM *alloca = generate_midCode(new AllocaLLVM(funcFParam));
        syb_2_llvm_insert(funcFParams.at(i), alloca);
        tempList1.push_back(funcFParam);
        tempList2.push_back(alloca);
        // generate_midCode(new StoreLLVM(type, funcFParam, alloca));
    }
    for (int i = 0; i < funcFParams.size(); i++)
    {
        var_type type = funcFParams.at(i)->type;
        generate_midCode(new StoreLLVM(type, tempList1.at(i), tempList2.at(i)));
    }
    curScope = fatherScope.at(curScope); // 更新当亲的scope

    generate_Block(root->GetLastChild());

    generate_midCode(new BrLLVM(curFuncReturnLabel));
    generate_midCode(curFuncReturnLabel);

    if (funcSymbol->returnType == VoidFunc)
        generate_midCode(new RetLLVM());
    else
        generate_midCode(new RetLLVM(curFuncType, generate_midCode(new LoadLLVM(curFuncType, curFuncReturnVar))));
    generate_midCode(new FuncEndLLVM());
}

// 表达式 Exp → AddExp
LLVM *generate_Exp(Node *root)
{
    return generate_AddExp(root->GetFirstChild());
}

void generate_Cond(Node *root, LLVM *trueLabel, LLVM *falseLabel)
{
    generate_LOrExp(root->GetFirstChild(), trueLabel, falseLabel);
}

// 左值表达式 LVal → Ident ['[' Exp ']'] // c
LLVM *generate_LVal(Node *root)
{
    LLVM *result = NULL;
    Symbol *symbol = find_symbol(root->GetFirstChild()->token);
    if (isVarType(symbol->type))
    {
        if (isConstType(symbol->type))
        {
            return new ConstLLVM(dynamic_cast<VarSymbol *>(symbol)->initVal);
        }
        else
        {
            result = generate_midCode(new LoadLLVM(symbol->type, syb_2_llvm.at(symbol)));
            if (!isIntType(symbol->type))
                result = generate_midCode(new ZextLLVM(result));
        }
    }
    else
    {
        ArraySymbol *arraySymbol = dynamic_cast<ArraySymbol *>(symbol);
        if (root->GetChildsSize() < 2) // 数组名
        {
            LLVM *src = syb_2_llvm.at(symbol);
            if (arraySymbol->isFuncArrayParam())
            {
                src = generate_midCode(new LoadLLVM(symbol->type, src));
            }
            return generate_midCode(new GetelementptrLLVM(symbol->type, arraySymbol->length, src, new ConstLLVM(0)));
        }
        LLVM *exp = generate_Exp(root->GetChildAt(2));
        if (isConstType(symbol->type) && exp->midType == CONST_IR)
        {
            int offset = dynamic_cast<ConstLLVM *>(exp)->val;
            return new ConstLLVM(arraySymbol->getInitValAt(offset));
        }
        else
        {
            LLVM *src = syb_2_llvm.at(symbol);
            // 当LVal为函数形参且是数组时，需要两次load，否则都是一次
            if (arraySymbol->isFuncArrayParam())
            {
                src = generate_midCode(new LoadLLVM(symbol->type, src));
            }
            LLVM *array = generate_midCode(new GetelementptrLLVM(symbol->type, arraySymbol->length, src, exp));
            result = generate_midCode(new LoadLLVM(symbol->type, array));
            if (!isIntType(symbol->type))
                result = generate_midCode(new ZextLLVM(result));
        }
    }
    return result;
}

// 基本表达式 PrimaryExp → '(' Exp ')' | LVal | Number | Character
LLVM *generate_PrimaryExp(Node *root)
{
    LLVM *result = NULL;
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == LVal)
    {
        result = generate_LVal(firstChild);
    }
    else if (firstChild->type == Number)
    {
        return generate_Number(firstChild);
    }
    else if (firstChild->type == Character)
    {
        return generate_Character(firstChild);
    }
    else
    {
        result = generate_Exp(root->GetChildAt(1));
    }
    return result;
}

// 数值 Number → IntConst
LLVM *generate_Number(Node *root)
{
    return new ConstLLVM(stoll(root->GetFirstChild()->token->value));
}

// 字符 Character → CharConst
LLVM *generate_Character(Node *root)
{
    string s = root->GetFirstChild()->token->value;
    int ans;
    if (s.size() > 3)
    {
        ans = escapeChars.at(s[2]);
    }
    else
        ans = s[1];
    return new ConstLLVM((int)ans);
}

// 一元表达式 UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp // c d e
LLVM *generate_UnaryExp(Node *root)
{
    LLVM *result = NULL;
    Node *firstChild = root->GetFirstChild();
    if (firstChild->type == PrimaryExp)
    {
        result = generate_PrimaryExp(firstChild);
    }
    else if (firstChild->type == UnaryOp)
    {
        tk_type type = firstChild->GetFirstChild()->token->type;
        LLVM *unary = generate_UnaryExp(root->GetChildAt(1));
        if (type == MINU)
        {
            if (unary->midType == CONST_IR)
            {
                return new ConstLLVM(-dynamic_cast<ConstLLVM *>(unary)->val);
            }
            result = generate_midCode(new RTypeLLVM(SUB_IR, new ConstLLVM(0), unary));
        }
        else if (type == NOT)
        {
            if (unary->midType == CONST_IR)
            {
                return new ConstLLVM(dynamic_cast<ConstLLVM *>(unary)->val == 0);
            }
            result = generate_midCode(new ZextLLVM(generate_midCode(new IcmpLLVM(EQL, unary, new ConstLLVM(0)))));
        }
        else
        {
            result = unary;
        }
    }
    else
    {
        FuncSymbol *funcSymbol = dynamic_cast<FuncSymbol *>(find_symbol(firstChild->token));
        vector<LLVM *> params;
        if (root->GetChildAt(2)->type == FuncRParams)
        {
            params = generate_FuncRParams(root->GetChildAt(2), funcSymbol->funcFParamSymbols);
        }

        result = generate_midCode(new CallLLVM(funcSymbol->returnType, funcSymbol->GetIdentName(), funcSymbol->funcFParamSymbols, params));
        if (dynamic_cast<CallLLVM *>(result)->retType == CharFunc)
            result = generate_midCode(new ZextLLVM(result));
    }
    return result;
}

// 函数实参表 FuncRParams → Exp { ',' Exp }
vector<LLVM *> generate_FuncRParams(Node *root, vector<Symbol *> paramSymbols)
{
    vector<LLVM *> FuncRParams;
    int i = 0;
    for (Node *child : root->childs)
    {
        if (child->type == Exp)
        {
            LLVM *exp = generate_Exp(child);

            if (!isIntType(paramSymbols.at(i)->type) && !isArrayType(paramSymbols.at(i)->type)) // 向char赋值 trunc
            {
                if (exp->midType != CONST_IR)
                    exp = generate_midCode(new TruncLLVM(exp));
                else
                {
                    int val = dynamic_cast<ConstLLVM *>(exp)->val & 0xff;
                    exp = new ConstLLVM(val);
                }
            }
            FuncRParams.push_back(exp);
            i++;
        }
    }
    return FuncRParams;
}

// 乘除模表达式 MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
LLVM *generate_MulExp(Node *root)
{
    LLVM *result = generate_UnaryExp(root->GetFirstChild());

    for (int i = 1; i < root->childs.size(); i += 2)
    {
        tk_type op = root->GetChildAt(i)->token->type;
        Node *child = root->GetChildAt(i + 1);
        LLVM *llvm = generate_UnaryExp(child);
        if (result->midType == CONST_IR && llvm->midType == CONST_IR)
            result = new ConstLLVM(tk_cal_2op(op, dynamic_cast<ConstLLVM *>(result)->val, dynamic_cast<ConstLLVM *>(llvm)->val));
        else
            result = generate_midCode(new RTypeLLVM(tk_2_mid.at(op), result, llvm));
    }
    return result;
}

// 加减表达式 AddExp → MulExp | AddExp ('+' | '−') MulExp
LLVM *generate_AddExp(Node *root)
{
    LLVM *result = NULL;
    int sum = 0, flag = 0; // flag=0代表还没碰到过常数
    bool sumFirst = false; //
    tk_type op, init_op = Undefined;
    LLVM *llvm = generate_MulExp(root->GetFirstChild());

    if (llvm->midType == CONST_IR)
    {
        sumFirst = true;
        sum = dynamic_cast<ConstLLVM *>(llvm)->val;
        flag = 1;
    }
    else
    {
        result = llvm;
    }

    for (int i = 1; i < root->childs.size(); i += 2)
    {
        op = root->GetChildAt(i)->token->type;
        Node *child = root->GetChildAt(i + 1);
        LLVM *llvm = generate_MulExp(child);
        if (init_op == Undefined && sumFirst != (llvm->midType == CONST_IR))
            init_op = op;

        if (llvm->midType == CONST_IR)
        {
            if (flag == 0)
            {
                sum = dynamic_cast<ConstLLVM *>(llvm)->val;
                if (op == MINU)
                    sum = -sum;
                flag = 1;
            }
            else
                sum = tk_cal_2op(op, sum, dynamic_cast<ConstLLVM *>(llvm)->val);
        }
        else
        {
            if (result == NULL)
            {
                result = llvm;
                if (op == MINU)
                    result = generate_midCode(new RTypeLLVM(SUB_IR, new ConstLLVM(0), result));
            }
            else
                result = generate_midCode(new RTypeLLVM(tk_2_mid.at(op), result, llvm));
        }
    }

    if (result == NULL)
    {
        return new ConstLLVM(sum);
    }
    else if (sum != 0 || sumFirst && init_op == MINU)
    // sum不为0或者形如 0-exp
    {
        // if (sumFirst)
        //     return generate_midCode(new RTypeLLVM(tk_2_mid[init_op], new ConstLLVM(sum), result));
        // else
        return generate_midCode(new RTypeLLVM(ADD_IR, result, new ConstLLVM(sum)));
    }
    else
    {
        return result;
    }
}

// 关系表达式 RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
LLVM *generate_RelExp(Node *root)
{
    LLVM *result = NULL;
    int sum = 0, flag = 0; // flag=0代表还没碰到过常数
    bool sumFirst = false; //
    tk_type op, init_op = Undefined;
    LLVM *llvm = generate_AddExp(root->GetFirstChild());

    if (llvm->midType == CONST_IR)
    {
        sumFirst = true;
        sum = dynamic_cast<ConstLLVM *>(llvm)->val;
        flag = 1;
    }
    else
    {
        result = llvm;
    }

    for (int i = 1; i < root->childs.size(); i += 2)
    {
        op = root->GetChildAt(i)->token->type;
        Node *child = root->GetChildAt(i + 1);
        LLVM *llvm = generate_AddExp(child);
        if (init_op == Undefined && sumFirst != (llvm->midType == CONST_IR))
            init_op = op;

        if (llvm->midType == CONST_IR)
        {
            if (flag == 0)
            {
                sum = dynamic_cast<ConstLLVM *>(llvm)->val;
                flag = 1;
            }
            else
                sum = tk_cal_2op(op, sum, dynamic_cast<ConstLLVM *>(llvm)->val);
        }
        else
        {
            if (result == NULL)
                result = llvm;
            else
                result = generate_midCode(new ZextLLVM(generate_midCode(new IcmpLLVM(op, result, llvm))));
        }
    }

    if (result == NULL)
        return new ConstLLVM(sum);
    else if (init_op == Undefined)
        return result;
    else if (sumFirst)
        return generate_midCode(new ZextLLVM(generate_midCode(new IcmpLLVM(init_op, new ConstLLVM(sum), result))));
    else
        return generate_midCode(new ZextLLVM(generate_midCode(new IcmpLLVM(init_op, result, new ConstLLVM(sum)))));
}

// 相等性表达式 EqExp → RelExp | EqExp ('==' | '!=') RelExp
LLVM *generate_EqExp(Node *root)
{
    LLVM *result = NULL;
    int sum = 0, flag = 0; // flag=0代表还没碰到过常数
    bool sumFirst = false; //
    tk_type op, init_op = Undefined;
    LLVM *llvm = generate_RelExp(root->GetFirstChild());

    if (llvm->midType == CONST_IR)
    {
        sumFirst = true;
        sum = dynamic_cast<ConstLLVM *>(llvm)->val;
        flag = 1;
    }
    else
    {
        result = llvm;
    }

    for (int i = 1; i < root->childs.size(); i += 2)
    {
        op = root->GetChildAt(i)->token->type;
        Node *child = root->GetChildAt(i + 1);
        LLVM *llvm = generate_RelExp(child);
        if (init_op == Undefined && sumFirst != (llvm->midType == CONST_IR))
            init_op = op;

        if (llvm->midType == CONST_IR)
        {
            if (flag == 0)
            {
                sum = dynamic_cast<ConstLLVM *>(llvm)->val;
                flag = 1;
            }
            else
                sum = tk_cal_2op(op, sum, dynamic_cast<ConstLLVM *>(llvm)->val);
        }
        else
        {
            if (result == NULL)
                result = llvm;
            else
                result = generate_midCode(new ZextLLVM(generate_midCode(new IcmpLLVM(op, result, llvm))));
        }
    }

    if (result == NULL)
        return generate_midCode(new ZextLLVM(generate_midCode(new IcmpLLVM(NEQ, new ConstLLVM(sum), new ConstLLVM(0)))));
    else if (init_op == Undefined && root->GetChildsSize() == 1 && root->GetFirstChild()->GetChildsSize() == 1)
        return generate_midCode(new ZextLLVM(generate_midCode(new IcmpLLVM(NEQ, result, new ConstLLVM(0)))));
    else if (init_op == Undefined)
        return result;
    else if (sumFirst)
        return generate_midCode(new ZextLLVM(generate_midCode(new IcmpLLVM(init_op, new ConstLLVM(sum), result))));
    else
        return generate_midCode(new ZextLLVM(generate_midCode(new IcmpLLVM(init_op, result, new ConstLLVM(sum)))));
}

// 逻辑与表达式 LAndExp → EqExp | LAndExp '&&' EqExp /
void generate_LAndExp(Node *root, LLVM *trueLabel, LLVM *falseLabel)
{
    int len = root->childs.size();
    LLVM *tLabel;
    for (int i = 0; i < len; i += 2)
    {
        if (i + 2 < len)
        {
            tLabel = new LabelLLVM();
        }
        else
        {
            tLabel = trueLabel;
        }
        Node *child = root->GetChildAt(i);
        LLVM *cmp = generate_EqExp(child);
        generate_midCode(new BrLLVM(generate_midCode(new TruncLLVM(cmp, "i1")), tLabel, falseLabel));
        if (tLabel != trueLabel)
            generate_midCode(tLabel);
    }
    return;
}

void generate_LOrExp(Node *root, LLVM *trueLabel, LLVM *falseLabel)
{
    int len = root->childs.size();
    LLVM *tLabel;
    for (int i = 0; i < len; i += 2)
    {
        if (i + 2 < len)
        {
            tLabel = new LabelLLVM();
        }
        else
        {
            tLabel = falseLabel;
        }

        Node *child = root->GetChildAt(i);
        generate_LAndExp(child, trueLabel, tLabel);
        if (tLabel != falseLabel)
            generate_midCode(tLabel);
    }
    return;
}

LLVM *find_GStr(string str, int len)
{
    for (LLVM *llvm : strCodes)
    {
        GStrLLVM *gStrLLVM = dynamic_cast<GStrLLVM *>(llvm);
        if (gStrLLVM->str == str)
            return gStrLLVM;
    }
    return generate_midCode(new GStrLLVM(str, len));
}

void symbols_2_mid()
{
    curScope = 1;
    curFuncType = var_null;
    curStrLoc = 1;
    generate_CompUnit(ROOT);
}