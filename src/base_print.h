void print_tokens()
{
    for (Token *token : tokens)
        fout1 << *token << endl;
}

void print_tree(Node *root)
{
    if (root->childs.empty())
        fout2 << *root->token << endl;
    else
    {
        for (Node *child : root->childs)
            print_tree(child);
        if (root->type != BlockItem && root->type != Decl && root->type != BType)
            fout2 << '<' << syn_type_2_str.at(root->type) << '>' << endl;
    }
}

void print_symbols()
{
    for (auto &pair : scope2symbols)
    {
        for (Symbol *symbol : pair.second)
        {
            fout3 << *symbol << endl;
        }
    }
}

void pre_print_mid()
{
    int cnt = 0;
    for (LLVM *llvm : midCodes)
    {
        mid_type type = llvm->midType;
        if (type == G_FUNC_DEF_IR)
        {
            cnt = dynamic_cast<GDefLLVM *>(llvm)->funcFParams.size() + 1;
        }
        else
        {
            if (llvm->hasReturn)
            {
                llvm->returnTk = "%" + to_string(cnt);
                cnt++;
            }
            llvm->refillNum();
        }
    }
}

void print_mid()
{
    pre_print_mid();
    pre_print_mid();

    fout4 << "declare i32 @getint()          ; 读取一个整数" << endl;
    fout4 << "declare i32 @getchar()     ; 读取一个字符" << endl;
    fout4 << "declare void @putint(i32)      ; 输出一个整数" << endl;
    fout4 << "declare void @putch(i32)       ; 输出一个字符" << endl;
    fout4 << "declare void @putstr(i8*)      ; 输出字符串" << endl;

    fout4 << endl;
    for (LLVM *llvm : strCodes)
    {
        fout4 << llvm->toString() << endl;
    }
    if (!strCodes.empty())
        fout4 << endl;

    int cnt = 1;
    bool inFunc = false;
    for (LLVM *llvm : midCodes)
    {
        mid_type type = llvm->midType;
        if (type == FUNC_END_IR)
        {
            inFunc = false;
            fout4 << "}" << endl;
            continue;
        }
        else if (type == LABEL_IR)
        {
            fout4 << endl
                  << llvm->toString() << endl;
            continue;
        }

        if (inFunc)
            fout4 << "  ";
        if (type == G_FUNC_DEF_IR)
        {
            inFunc = true;
            cnt = dynamic_cast<GDefLLVM *>(llvm)->funcFParams.size() + 1;
            fout4 << endl
                  << llvm->toString() << " {\n";
        }
        else
        {
            fout4 << llvm->toString() << endl;
        }
    }
}

void print_errors()
{
    for (const auto &e : errors)
    {
        ferr << e.first << " " << e.second << endl;
    }
}