void Optimizer::init_visited()
{
    visited.clear();
    for (BasicBlock *block : blocks)
        visited.insert({block, false});
}

void Optimizer::gen_entryBlocks()
{
    for (BasicBlock *block : blocks)
        if (block->label->midType == G_FUNC_DEF_IR)
            entryBlocks.push_back(block);
}

void Optimizer::gen_prev_next()
{
    int len = blocks.size();
    for (int i = 0; i < len; ++i)
    {
        BasicBlock *block = blocks.at(i);
        LLVM *label = block->label;
        label2block.insert({label, block});
        prevs.insert({block, set<BasicBlock *>()});
        nexts.insert({block, set<BasicBlock *>()});
    }

    for (int i = 0; i < len; ++i)
    {
        BasicBlock *block = blocks.at(i);
        LLVM *label = block->label;
        for (LLVM *llvm : block->codes)
        {
            if (llvm->midType == BR_IR)
            {
                BrLLVM *br = dynamic_cast<BrLLVM *>(llvm);
                BasicBlock *block1 = label2block.at(br->label1);
                nexts.at(block).insert(block1);
                prevs.at(block1).insert(block);
                if (br->label2 != NULL)
                {
                    BasicBlock *block2 = label2block.at(br->label2);
                    nexts.at(block).insert(block2);
                    prevs.at(block2).insert(block);
                }
            }
        }
    }
}

void Optimizer::gen_entrySet_recursive(BasicBlock *entry, BasicBlock *block)
{
    if (visited.at(block))
        return;
    visited.at(block) = true;
    entrySet.at(entry).insert(block);
    for (BasicBlock *next : nexts.at(block))
        gen_entrySet_recursive(entry, next);
}

void Optimizer::gen_entrySet()
{
    for (BasicBlock *block : entryBlocks)
    {
        init_visited();
        entrySet.insert({block, set<BasicBlock *>()});
        gen_entrySet_recursive(block, block);
    }
}

// get doms
void Optimizer::gen_doms()
{
    // 支配我的集合 domeds
    for (BasicBlock *block : blocks)
        domeds.insert({block, set<BasicBlock *>()});

    while (domChanged)
    {
        domChanged = false;
        init_visited();
        for (BasicBlock *entry : entryBlocks)
        {
            vector<BasicBlock *> queue = {entry};
            while (!queue.empty())
            {
                BasicBlock *block = queue.at(0);

                // 计算所有prev结点的domeds交集
                set<BasicBlock *> newDom(blocks.begin(), blocks.end());
                for (BasicBlock *pred : prevs.at(block))
                {
                    set<BasicBlock *> temp;
                    set_intersection(domeds.at(pred).begin(), domeds.at(pred).end(),
                                     newDom.begin(), newDom.end(),
                                     inserter(temp, temp.begin()));
                    newDom = temp;
                }
                if (prevs.at(block).empty())
                    newDom.clear();
                newDom.insert(block);
                newDom.insert(entry);

                if (newDom.size() < domeds.at(block).size())
                    DIE("gen_dom error");

                if (domeds.at(block) != newDom)
                {
                    domChanged = true;
                    domeds.at(block) = newDom;
                }

                queue.erase(queue.begin());
                for (BasicBlock *back : nexts.at(block))
                {
                    if (!visited.at(back))
                    {
                        queue.push_back(back);
                        visited.at(back) = true;
                    }
                }
            }
        }
    }
    // doms
    for (BasicBlock *block : blocks)
        doms.insert({block, set<BasicBlock *>()});

    for (auto &pair : domeds)
        for (BasicBlock *block : pair.second)
            doms.at(block).insert(pair.first);
}

void Optimizer::gen_sdoms()
{
    // sdoms
    for (auto &pair : doms)
    {
        set<BasicBlock *> temp = pair.second;
        temp.erase(pair.first);
        sdoms.insert({pair.first, temp});
    }
    // 严格支配我的集合 sdomeds
    for (BasicBlock *block : blocks)
        sdomeds.insert({block, set<BasicBlock *>()});

    for (auto &pair : sdoms)
        for (BasicBlock *block : pair.second)
            sdomeds.at(block).insert(pair.first);
}

void Optimizer::gen_idoms()
{
    // idomeds  idom
    for (BasicBlock *block : blocks)
        idoms.insert({block, set<BasicBlock *>()});
    for (auto &pair : sdomeds)
    {
        for (BasicBlock *block : pair.second)
        {
            set<BasicBlock *> temp;
            set_intersection(sdoms.at(block).begin(), sdoms.at(block).end(),
                             pair.second.begin(), pair.second.end(),
                             inserter(temp, temp.begin()));
            if (temp.empty())
            {
                idomeds.insert({pair.first, block});
                idoms.at(block).insert(pair.first);
                break;
            }
        }
    }
}

void Optimizer::gen_df()
{
    for (BasicBlock *block : blocks)
        df.insert({block, set<BasicBlock *>()});

    for (auto &pair : nexts)
    {
        BasicBlock *a = pair.first;
        for (BasicBlock *b : pair.second)
        {
            BasicBlock *x = a;
            while (sdoms.at(x).find(b) == sdoms.at(x).end())
            {
                df.at(x).insert(b);
                if (idomeds.find(x) == idomeds.end())
                    break; // 到达起点 起点没有直接支配者
                x = idomeds.at(x);
            }
        }
    }
}
void Optimizer::gen_varSet()
{
    for (BasicBlock *block : blocks)
    {
        for (LLVM *llvm : block->codes)
        {
            LLVM *def = NULL;

            if (llvm->midType == ALLOCA_IR)
                def = llvm;
            else if (llvm->midType == STORE_IR)
            {
                StoreLLVM *store = dynamic_cast<StoreLLVM *>(llvm);
                if (store->des->midType == ALLOCA_IR)
                    def = store->des;
            }

            if (def != NULL)
            {
                varSet.insert(def);
                if (varDefBlocks.find(def) == varDefBlocks.end())
                    varDefBlocks.insert({def, set<BasicBlock *>()});
                varDefBlocks.at(def).insert(block);
            }
        }
    }
}

void Optimizer::gen_phi()
{
    for (auto &pair : varDefBlocks)
    {
        LLVM *v = pair.first;
        set<BasicBlock *> W = pair.second;
        set<BasicBlock *> F;
        while (!W.empty())
        {
            BasicBlock *X = *W.begin();
            W.erase(X);
            for (BasicBlock *Y : df.at(X))
            {
                if (F.find(Y) == F.end())
                {
                    //add phi
                    LLVM *phi;
                    phi = new PhiLLVM({}); 
                    Y->codes.insert(Y->codes.begin() + 1, phi);
                    F.insert(Y);
                    if (pair.second.find(Y) == pair.second.end())
                    {
                        W.insert(Y);
                        pair.second.insert(Y); // new
                    }
                }
            }
        }
    }
}

void Optimizer::del_alloca()
{
    for (BasicBlock *block : blocks)
    {
        int len = block->codes.size();
        for (int i = 0; i < len; i++)
        {
            LLVM *llvm = block->codes.at(i);
            if (llvm->midType == ALLOCA_IR)
            {
                AllocaLLVM *alloca = dynamic_cast<AllocaLLVM *>(llvm);
                if (!isArrayType(alloca->allocaType))
                {
                    block->codes.erase(block->codes.begin() + i);
                    i--;
                    len--;
                }
            }
            else if (llvm->midType == STORE_IR)
            {
                StoreLLVM *store = dynamic_cast<StoreLLVM *>(llvm);
                if (store->des->midType == ALLOCA_IR)
                {
                    AllocaLLVM *alloca = dynamic_cast<AllocaLLVM *>(store->des);
                    /*todo*/
                }
            }
        }
    }
}

void Optimizer::optimize_mem2reg()
{
    // 每个函数的第一个基本块 入口集合
    gen_entryBlocks();
    // CFG图 prev_next
    gen_prev_next();
    // 每个函数的基本块集合
    gen_entrySet();
    // 支配图
    gen_doms();
    // 严格支配图
    gen_sdoms();
    // 直接支配图
    gen_idoms();
    // 支配边界
    gen_df();
    // 找到变量集合
    gen_varSet();
    // 插入phi
    gen_phi();
    // // 删除不必要的alloca load store
    del_alloca();
}