void cal_symbols();
void cal_array_symbol(Symbol *symbol);
void cal_var_symbol(Symbol *symbol);
void cal_func_symbol(Symbol *symbol);

long long cal_Exp(Node *root);
long long cal_AddExp(Node *root);
long long cal_MulExp(Node *root);
long long cal_UnaryExp(Node *root);
long long cal_PrimaryExp(Node *root);
long long cal_LVal(Node *root);
Symbol *find_symbol(Token *token, bool isDeclare = false);
long long cal_Number(Node *root);
long long cal_Character(Node *root);

const map<char, int> escapeChars = {
    {'n', '\n'},
    {'t', '\t'},
    {'r', '\r'},
    {'a', '\a'},
    {'b', '\b'},
    {'f', '\f'},
    {'v', '\v'},
    {'\\', '\\'},
    {'\'', '\''},
    {'\"', '\"'},
    {'0', '\0'}};