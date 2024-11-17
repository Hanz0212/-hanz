enum tk_type
{
    IDENFR,
    ELSETK,
    VOIDTK,
    SEMICN,
    INTCON,
    NOT,
    MULT,
    COMMA,
    STRCON,
    AND,
    DIV,
    LPARENT, // (
    CHRCON,
    OR,
    MOD,
    RPARENT, // )
    MAINTK,
    FORTK,
    LSS,
    LBRACK, // [
    CONSTTK,
    GETINTTK,
    LEQ,
    RBRACK, // ]
    INTTK,
    GETCHARTK,
    GRE,
    LBRACE, // }
    CHARTK,
    PRINTFTK,
    GEQ,
    RBRACE, // {
    BREAKTK,
    RETURNTK,
    EQL,
    CONTINUETK,
    PLUS,
    NEQ,
    IFTK,
    MINU,
    ASSIGN,
    Undefined
};

const map<tk_type, string> tk_type_2_str = {
    {IDENFR, "IDENFR"},
    {ELSETK, "ELSETK"},
    {VOIDTK, "VOIDTK"},
    {SEMICN, "SEMICN"},
    {INTCON, "INTCON"},
    {NOT, "NOT"},
    {MULT, "MULT"},
    {COMMA, "COMMA"},
    {STRCON, "STRCON"},
    {AND, "AND"},
    {DIV, "DIV"},
    {LPARENT, "LPARENT"},
    {CHRCON, "CHRCON"},
    {OR, "OR"},
    {MOD, "MOD"},
    {RPARENT, "RPARENT"},
    {MAINTK, "MAINTK"},
    {FORTK, "FORTK"},
    {LSS, "LSS"},
    {LBRACK, "LBRACK"},
    {CONSTTK, "CONSTTK"},
    {GETINTTK, "GETINTTK"},
    {LEQ, "LEQ"},
    {RBRACK, "RBRACK"},
    {INTTK, "INTTK"},
    {GETCHARTK, "GETCHARTK"},
    {GRE, "GRE"},
    {LBRACE, "LBRACE"},
    {CHARTK, "CHARTK"},
    {PRINTFTK, "PRINTFTK"},
    {GEQ, "GEQ"},
    {RBRACE, "RBRACE"},
    {BREAKTK, "BREAKTK"},
    {RETURNTK, "RETURNTK"},
    {EQL, "EQL"},
    {CONTINUETK, "CONTINUETK"},
    {PLUS, "PLUS"},
    {NEQ, "NEQ"},
    {IFTK, "IFTK"},
    {MINU, "MINU"},
    {ASSIGN, "ASSIGN"},
    {Undefined, "Undefined"}};

struct Token
{
    tk_type type;
    string value;
    int row;

    Token()
    {
        this->type = Undefined;
        this->value = "";
        this->row = 0;
    }

    Token(tk_type type, string value, int row)
    {
        this->type = type;
        this->value = value;
        this->row = row;
    }

    friend ostream &operator<<(ostream &output, const Token &token)
    {
        output << tk_type_2_str.at(token.type) << ' ' << token.value;
        return output;
    }
};
// "+-*/%;,()[]{}";
const map<char, tk_type> sop_2_tk_type = {
    {'+', PLUS},
    {'-', MINU},
    {'*', MULT},
    {'/', DIV},
    {'%', MOD},
    {';', SEMICN},
    {',', COMMA},
    {'(', LPARENT},
    {')', RPARENT},
    {'[', LBRACK},
    {']', RBRACK},
    {'{', LBRACE},
    {'}', RBRACE},
    {'=', ASSIGN},
    {'<', LSS},
    {'>', GRE},
    {'!', NOT}};

const map<char, tk_type> dop_2_tk_type = {
    {'=', EQL},
    {'<', LEQ},
    {'>', GEQ},
    {'!', NEQ},
    {'&', AND},
    {'|', OR}};

const map<string, tk_type> keyword_2_tk_type = {
    {"main", MAINTK},
    {"const", CONSTTK},
    {"int", INTTK},
    {"char", CHARTK},
    {"break", BREAKTK},
    {"continue", CONTINUETK},
    {"if", IFTK},
    {"else", ELSETK},
    {"for", FORTK},
    {"getint", GETINTTK},
    {"getchar", GETCHARTK},
    {"printf", PRINTFTK},
    {"return", RETURNTK},
    {"void", VOIDTK}};

vector<Token *> tokens;
int cal_2op(tk_type type, int op1, int op2);
