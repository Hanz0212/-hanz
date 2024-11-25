#include "base.h"
// #include <variant>

// void show_symbols()
// {
//     vector<variant<VarSymbol *, ArraySymbol *, FuncSymbol *>> s;
//     for (Symbol *symbol : symbols)
//     {
//         if (isVarType(symbol->type))
//             s.push_back(dynamic_cast<VarSymbol *>(symbol));
//         else if (isArrayType(symbol->type))
//             s.push_back(dynamic_cast<ArraySymbol *>(symbol));
//         else
//             s.push_back(dynamic_cast<FuncSymbol *>(symbol));
//     }
//     int debug_here;
//     return;
// }

int main()
{
    // 将源文件全部读入字符串
    string content((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();

    // 词法分析
    char_2_token(content);
    cout << "lexer done!" << endl;
    print_tokens();
    fout1.close();

    if (!errors.empty())
    {
        int a = 10;
        while (a--)
        {
            fout4 << "damn" << endl;
        }
        return 0;
    }

    // 语法分析
    token_2_tree();
    cout << "parser done!" << endl;
    print_tree(ROOT);
    fout2.close();

    // 语义分析
    tree_2_symbols();
    cout << "symbol done!" << endl;
    print_symbols();
    fout3.close();

    // 错误输出
    print_errors();
    ferr.close();

    if (!errors.empty())
    {
        int a = 10;
        while (a--)
        {
            fout4 << "damn" << endl;
        }
        return 0;
    }
    // 计算符号表（初值，数组大小）
    cal_symbols();
    // show_symbols();

    // 中间代码生成
    symbols_2_mid();
    cout << "mid done!" << endl;

    // 中间代码优化
    optimize_mid();
    print_mid();
    fout4.close();

    // 目标代码生成
    mid_2_mips();
    cout << "mips done!" << endl;
    print_mips();
    fout5.close();

    return 0;
}
