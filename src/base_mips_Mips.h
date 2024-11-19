MipsManager::MipsManager()
{
}

void MipsManager::addCode(CodeMips *code)
{
    mipsCodes.push_back(code);
}

void MipsManager::addData(DataMips *data)
{
    mipsDatas.push_back(data);
}

void MipsManager::occupy(LLVM *llvm, RegPtr reg)
{
    tempRegPool->tryOccupy(reg->getType());
    occupation.insert({llvm, reg});
}

// 保证:局部数组变量一定返回offset，全局数组一定返回label,置true一定返回tempReg
RegPtr MipsManager::findOccupiedReg(LLVM *llvm, bool needTempReg)
{
    if (occupation.count(llvm) == 0)
    {
        DIE("findOccupiedReg failed !!\nmidType: " + mid_type_2_str.at(llvm->midType));
    }

    RegPtr reg = occupation.at(llvm);
    if (needTempReg && !in32Reg(reg->getType()))
        reg = load(llvm);

    if (llvm->midType == ALLOCA_IR && reg->getType() != OFFSET &&
        isArrayType(dynamic_cast<AllocaLLVM *>(llvm)->allocaType))
    {
        DIE("in findOccupiedReg: isLocalArray but not return inter");
    }
    if (llvm->midType == G_VAR_DEF_IR && reg->getType() != LABEL &&
        isArrayType(dynamic_cast<GDefLLVM *>(llvm)->GDefType))
    {
        DIE("in findOccupiedReg: isGlobalArray but not return label");
    }
    if (needTempReg && !in32Reg(reg->getType()))
    {
        DIE("in findOccupiedReg: needTempReg but not return tempReg");
    }

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
    for (const auto &pair : occupation)
    {
        if (pair.second->getType() == regType)
        {
            tllvm = pair.first;
            break;
        }
    }
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
// 若regType是寄存器，则release，否则无事发生
void MipsManager::tryReleaseReg(RegPtr reg)
{
    tempRegPool->tryRelease(reg);
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
    addCode(new StoreMips(findOccupiedReg(llvm), manager->sp, curStack_inter, "push"));
    release(llvm);
    occupation.insert({llvm, curStack_inter});
    curStack += 4;
}

RegPtr MipsManager::load(LLVM *llvm)
{
    RegPtr reg = getFreeTempReg(llvm);
    RegPtr offset = occupation.at(llvm);
    addCode(new LoadMips(reg, sp, offset, "reload"));
    occupy(llvm, reg);
    return reg;
}