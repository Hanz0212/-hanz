struct Optimizer
{
    vector<BasicBlock *> blocks;                   // 基本块集合
    set<LLVM *> varSet;                            // 变量集合
    map<LLVM *, set<BasicBlock *>> varDefBlocks;   // 变量定义的基本块集合
    vector<BasicBlock *> entryBlocks;              // 每个函数的第一个基本块
    map<BasicBlock *, set<BasicBlock *>> entrySet; // 每个函数基本块集合
    map<LLVM *, BasicBlock *> label2block;
    map<BasicBlock *, set<BasicBlock *>> prevs;
    map<BasicBlock *, set<BasicBlock *>> nexts;
    map<BasicBlock *, set<BasicBlock *>> domeds;  // 被支配者视角
    map<BasicBlock *, set<BasicBlock *>> doms;    // 支配者视角
    map<BasicBlock *, set<BasicBlock *>> sdomeds; // 被严格支配者视角
    map<BasicBlock *, set<BasicBlock *>> sdoms;   // 严格支配者视角
    map<BasicBlock *, BasicBlock *> idomeds;      // 被直接支配者视角
    map<BasicBlock *, set<BasicBlock *>> idoms;   // 直接支配者视角
    map<BasicBlock *, set<BasicBlock *>> df;      // 支配边界

    Optimizer(vector<BasicBlock *> blocks) : blocks(blocks) {}

    void optimize_mid();
    void del_dead_block();

    map<BasicBlock *, bool> visited;
    void init_visited();
    void gen_entryBlocks();
    void gen_prev_next();
    void gen_entrySet_recursive(BasicBlock *entry, BasicBlock *block);
    void gen_entrySet();

    bool domChanged = true;
    void gen_varSet();
    void gen_doms();
    void gen_sdoms();
    void gen_idoms();
    void gen_df();
    void gen_phi();
    void del_alloca();
    void optimize_mem2reg();
};