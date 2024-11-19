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
ofstream fout5("mips.asm");
ofstream ferr("error.txt");
ifstream fin("testfile.txt");

void DIE(string text)
{
    cout << "D I E ! ! !" << endl;
    cout << text << endl;
    exit(1);
}

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

#include "base_print.h"
#include "base_optimize_mid.h"
