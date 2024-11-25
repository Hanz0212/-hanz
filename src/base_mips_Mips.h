MipsManager::MipsManager()
{
}

void MipsManager::addCode(CodeMips *code)
{
    mipsCodes.push_back(code);
}

// 将一个数字/寄存器/offset 的值给到另一个寄存器
void MipsManager::addCode_move(RegPtr des, LLVM *llvm, string annotation)
{
    RegPtr src;
    if (llvm->midType == CONST_IR)
        src = new IntermediateReg(dynamic_cast<ConstLLVM *>(llvm)->val);
    else
        src = findOccupiedReg(llvm, true);

    if (!in32Reg(des->getType()) || src->getType() != INTERMEDIATE && !in32Reg(src->getType()))
        DIE("eror type in addCode_move: <" + reg_type_2_str(des->getType()) + "," + reg_type_2_str(src->getType()) + ">");

    if (in32Reg(src->getType()))
        mipsCodes.push_back(new ITypeMips(ADDIU_OP, des, src, zero_inter, annotation));
    else if (src->getType() == INTERMEDIATE)
        mipsCodes.push_back(new LiMips(des, src, annotation));
}

void MipsManager::addData(DataMips *data)
{
    mipsDatas.push_back(data);
}

RegPtr MipsManager::addFunc(string funcName, RegPtr func)
{
    funcRegs.insert({funcName, func});
    return func;
}

RegPtr MipsManager::findFunc(string funcName)
{
    if (funcRegs.find(funcName) == funcRegs.end())
        DIE("error funcName cant find func! :" + funcName);
    return funcRegs.at(funcName);
}

RegPtr MipsManager::addLabel(string labelName, RegPtr label)
{
    labelRegs.insert({labelName, label});
    return label;
}

// manager->curFuncName + labelLLVM->returnTk
RegPtr MipsManager::findLabel(string labelName)
{
    if (labelRegs.find(labelName) == labelRegs.end())
        DIE("error funcName cant find label! :" + labelName);
    return labelRegs.at(labelName);
}

void MipsManager::addAnnotation(AnnotationMips *annotation)
{
    mipsCodes.push_back(annotation);
}

// 建立映射，若为临时寄存器，从pool删除
void MipsManager::occupy(LLVM *llvm, RegPtr reg)
{
    tempRegPool->tryOccupy(reg->getType());
    // 之前建立过和offset的映射 被push后又load出来，需要删除之前映射，再和寄存器建立映射
    if (occupation.find(llvm) != occupation.end())
        occupation.erase(llvm);
    occupation.insert({llvm, reg});
}

RegPtr MipsManager::findOccupiedReg(LLVM *llvm, bool needTempReg)
{
    if (llvm->midType == G_VAR_DEF_IR)
        return findLabel(dynamic_cast<GDefLLVM *>(llvm)->varName);

    if (occupation.count(llvm) == 0)
    {
        DIE("findOccupiedReg failed !!\nmidType: " + mid_type_2_str.at(llvm->midType));
    }

    RegPtr reg = occupation.at(llvm);
    if (needTempReg && !in32Reg(reg->getType()))
    {
        cout << "in load" << endl;
        reg = load(llvm);
    }

    if (reg->getType() == INTERMEDIATE)
        DIE("cant find intermediate in occupation!!");
    return reg;
}

// 获取一个空闲临时寄存器 若没有则push 强行获取（但不occupy）
RegPtr MipsManager::getFreeTempReg(LLVM *llvm)
{
    for (int i = 0; i < tempRegPool->POOLSIZE; i++)
    {
        reg_type regType = tempRegPool->getRegTypeAt(i);
        if (tempRegPool->isFree(regType))
        {
            tempRegPool->updateRegType(i + 1);
            return tempRegPool->getReg(regType);
        }
    }

    reg_type regType = tempRegPool->getRegTypeAt(0);
    tempRegPool->updateRegType();
    LLVM *tllvm;
    bool flag = false;
    for (const auto &pair : occupation)
    {
        if (pair.second->getType() == regType)
        {
            tllvm = pair.first;
            flag = true;
            break;
        }
    }
    if (!flag)
        DIE("in get freeTempReg: reg not in regpool and not in occupation");
    push(tllvm);
    return tempRegPool->getReg(regType);
}

RegPtr MipsManager::allocTempReg(LLVM *llvm)
{
    RegPtr reg = getFreeTempReg(llvm);
    occupy(llvm, reg);
    return reg;
}

RegPtr MipsManager::allocMem(LLVM *llvm, int size)
{
    RegPtr reg = new OffsetReg(curStack);
    occupy(llvm, reg);
    curStack += size * 4;
    return reg;
}

// 释放llvm对应的reg，解除配对，恢复寄存器池
void MipsManager::release(LLVM *llvm)
{
    if (occupation.count(llvm) == 0)
    {
        DIE("release faild : llvm not exist !!!");
    }
    RegPtr reg = occupation.at(llvm);
    tempRegPool->tryRelease(reg);
    occupation.erase(llvm);
}

// 将寄存器对应的llvm压栈，释放寄存器
void MipsManager::push(LLVM *llvm)
{
    RegPtr curStack_inter = new OffsetReg(curStack);
    addCode(new StoreMips(findOccupiedReg(llvm), curStack_inter, "#push"));
    release(llvm);
    occupation.insert({llvm, curStack_inter});
    curStack += 4;
}

RegPtr MipsManager::load(LLVM *llvm)
{
    RegPtr offset = occupation.at(llvm);
    RegPtr reg = getFreeTempReg(llvm);
    addCode(new LoadMips(reg, offset, "#reload"));
    occupy(llvm, reg);
    return reg;
}

RegPtr MipsManager::allocStackSpace()
{
    if (!stackSpaceIsCorrect)
        DIE("havnt correct stack space!");
    stackSpace = new IntermediateReg(-400);
    stackSpaceIsCorrect = false;

    // 0留给ra
    curStack = 4;
    return stackSpace;
}

void MipsManager::correctStackSpace()
{
    if (stackSpaceIsCorrect)
        DIE("stack space already correct!");
    stackSpaceIsCorrect = true;
    stackSpace->changeValTo(-curStack - 8);
    for (int i = 0; i < funfFParamStackSpaces.size(); i++)
        funfFParamStackSpaces.at(i)->changeValTo(curStack - 4 * (i + 1) + 8);
    funfFParamStackSpaces.clear();
}

void MipsManager::pushRa()
{
    raBeenPushed = true;
    addCode(new StoreMips(manager->ra, manager->zero_off, "#push ra"));
}

void MipsManager::popRa()
{
    if (raBeenPushed)
    {
        addCode(new LoadMips(manager->ra, manager->zero_off, "#pop ra"));
        raBeenPushed = false;
    }
}

void MipsManager::pushAll()
{
    set<LLVM *> pushSet;
    for (auto &pair : occupation)
    {
        if (tempRegPool->inRegTypes(pair.second->getType()))
        {
            pushSet.insert(pair.first);
        }
    }

    for (LLVM *llvm : pushSet)
        push(llvm);
}

void MipsManager::resetFrame(string funcName)
{
    manager->curFuncName = funcName;
    manager->addCode(new LabelMips(manager->findFunc(funcName), "")); // name
    // 重置寄存器池
    tempRegPool = new _RegPool();
    // 释放occupation（reg offset intermediate）
    occupation.clear();
}
