struct BasicBlock
{
    LLVM *label;
    vector<LLVM *> codes;
    string labelName;

    BasicBlock(LLVM *label, vector<LLVM *> codes)
    {
        this->label = label;
        this->codes = codes;
        if (dynamic_cast<LabelLLVM *>(label))
            this->labelName = dynamic_cast<LabelLLVM *>(label)->returnTk;
        else if (dynamic_cast<GDefLLVM *>(label))
            this->labelName = dynamic_cast<GDefLLVM *>(label)->varName;
        else
            DIE("in basicblock wrong type: <" + mid_type_2_str.at(label->midType) + ">");
    }
};

vector<BasicBlock *> blocks;

void getBasicBlocks()
{
    int len = midCodes.size();
    vector<LLVM *> codes;
    LLVM *label = NULL;
    for (int i = 0; i < len; i++)
    {
        LLVM *llvm = midCodes.at(i);
        if (llvm->midType == G_FUNC_DEF_IR || llvm->midType == LABEL_IR)
        {
            if (label != NULL)
                blocks.push_back(new BasicBlock(label, codes));
            label = llvm;
            codes.clear();
        }
        codes.push_back(llvm);
    }
    if (label != NULL)
        blocks.push_back(new BasicBlock(label, codes));
}