#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <set>
// #include <variant>
#include <optional>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <memory>
using namespace std;
int DEBUG = 0;
int DEBUG2 = 0;
ofstream fout1("lexer.txt");
ofstream fout2("parser.txt");
ofstream fout3("symbol.txt");
// ofstream fout4("mid.ll");
ofstream fout4("llvm_ir.txt");
// ofstream fout5("mips.asm");
ofstream fout5("mips.txt");
ofstream fout6("llvm_ir_opt.txt");
ofstream ferr("error.txt");
ifstream fin("testfile.txt");

void DIE(string text);

#include "base_lexer_declare.h"
#include "base_syntax_declare.h"
#include "base_symbol_declare.h"
#include "base_cal_declare.h"
#include "base_mid_declare.h"
#include "base_mips_enum.h"
#include "base_mips_Reg.h"
#include "base_mips_declare.h"
#include "base_mips_DataMips.h"
#include "base_mips_CodeMips.h"
#include "base_mips_Mips.h"
#include "base_error_declare.h"

#include "base_error.h"
#include "base_lexer.h"
#include "base_syntax.h"
#include "base_symbol.h"
#include "base_cal.h"
#include "base_mid.h"
#include "base_mips.h"

#include "base_block.h"
#include "base_optimize_declare.h"
#include "base_optimize_ssa.h"
#include "base_optimize_mid.h"
#include "base_print.h"

bool inDie = false;
void DIE(string text)
{
    RegPtr pp = NULL;
    pp->getStr();
    if (inDie)
    {
        cout << "D I E AGAIN! ! !" << endl;
        cout << text << endl;
        exit(1);
    }
    inDie = true;
    cout << "D I E ! ! !" << endl;
    cout << text << endl;
    print_mips();
    exit(1);
}