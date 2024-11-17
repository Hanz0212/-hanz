int cal_2op(tk_type type, int op1, int op2)
{
    switch (type)
    {
    case MULT:
        return op1 * op2;
    case DIV:
        return op1 / op2;
    case MOD:
        return op1 % op2;
    case PLUS:
        return op1 + op2;
    case MINU:
        return op1 - op2;
    case GEQ:
        return op1 >= op2;
    case LEQ:
        return op1 <= op2;
    case EQL:
        return op1 == op2;
    case NEQ:
        return op1 != op2;
    case LSS:
        return op1 < op2;
    case GRE:
        return op1 > op2;
    default:
        cout << "in cal_2op: unknwon op type!: " << tk_type_2_str.at(type) << endl;
        exit(1);
    }
}


void char_2_token(const string &input)
{
    int len = input.size();
    int row = 1;
    for (int i = 0; i < len; i++)
    {
        // 识别换行符
        if (input[i] == '\n')
        {
            row++;
            continue;
        }
        // 跳过空白符
        if (isspace(input[i]))
            continue;
        // 跳过单行注释
        if (input[i] == '/' && i + 1 < len && input[i + 1] == '/')
        {
            while (i < len && input[i] != '\n')
                i++;
            i--;
            continue;
        }
        // 跳过多行注释
        if (input[i] == '/' && i + 1 < len && input[i + 1] == '*')
        {
            i += 2;
            while (i + 1 < len && (input[i] != '*' || input[i + 1] != '/'))
            {
                if (input[i] == '\n')
                    row++;
                i++;
            }
            i++;
            continue;
        }
        string value;
        if (isalpha(input[i]) || input[i] == '_')
        { // 保留字或者标识符
            for (; isalnum(input[i]) || input[i] == '_'; i++)
                value += input[i];
            bool isKeyWord = false;
            for (const auto &pair : keyword_2_tk_type)
            {
                if (value == pair.first)
                {
                    tokens.push_back(new Token(pair.second, value, row));
                    isKeyWord = true;
                    break;
                }
            }
            if (!isKeyWord)
                tokens.push_back(new Token(IDENFR, value, row));
            i--;
        }
        else if (isdigit(input[i]))
        { // 数值常量
            for (; isdigit(input[i]); i++)
                value += input[i];
            tokens.push_back(new Token(INTCON, value, row));
            i--;
        }
        else if (input[i] == '\'')
        { // 字符常量
            value += '\'';
            if (input[i + 1] == '\\')
            {
                value += '\\';
                i++;
            }
            value += input[i + 1];
            i += 2;
            value += '\'';
            tokens.push_back(new Token(CHRCON, value, row));
        }
        else if (input[i] == '\"')
        { // 字符串常量
            value += '\"';
            for (i++; isascii(input[i]) && input[i] != '\"'; i++)
                value += input[i];
            value += '\"';
            tokens.push_back(new Token(STRCON, value, row));
        }
        else if (strchr("+-*/%;,()[]{}", input[i]))
        { // 单字符操作符
            value += input[i];
            tokens.push_back(new Token(sop_2_tk_type.at(input[i]), value, row));
        }
        else if (input[i] == '&' || input[i] == '|')
        { // 双字符操作符
            value += input[i];
            error_a(input, i, value, row);
        }
        else if (strchr("><!=", input[i]))
        { // 单双均可操作符
            value += input[i];
            if (input[i + 1] == '=')
            {
                value += '=';
                tokens.push_back(new Token(dop_2_tk_type.at(input[i]), value, row));
                i++;
            }
            else
            {
                tokens.push_back(new Token(sop_2_tk_type.at(input[i]), value, row));
            }
        }
        else
        {
            for (Token *token : tokens)
            {
                cout << tk_type_2_str.at(token->type) << ' ' << token->value << endl;
            }
            cout << "impossible!!!" << endl;
            cout << input[i] << endl;
            exit(1);
        }
    }
}