MipsManager::MipsManager()
{
    for (reg_type regType : temp_reg_types)
        tempRegPool.insert({regType, new TempReg(regType)});
}

void MipsManager::addCode(Mips *code)
{
    mipsCodes.push_back(code);
}

void MipsManager::occupy(LLVM *llvm, RegPtr reg)
{
    occupation.insert({llvm, reg});
}

RegPtr MipsManager::findOccupiedReg(LLVM *llvm)
{
    if (occupation.count(llvm) == 0)
    {
        cout << "findOccupiedReg failed !!" << endl;
        exit(1);
    }
    return occupation.at(llvm);
}

RegPtr MipsManager::getFreeTempReg(LLVM *llvm)
{
    for (int i = 0; i < TEMPREGCNT; i++)
    {
        int index = (tempCount + i) % TEMPREGCNT;
        reg_type regType = temp_reg_types.at(index);
        if (tempRegPool.find(regType) != tempRegPool.end())
        {
            tempCount = (index + 1) % TEMPREGCNT;
            RegPtr reg = tempRegPool.at(regType);
            tempRegPool.erase(regType);
            return reg;
        }
    }

    int index = tempCount;
    tempCount = (tempCount + 1) % TEMPREGCNT;
    // push todo
    return NULL;
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
    curStack += size;
    return reg;
}

void MipsManager::release(LLVM *llvm)
{
    if (occupation.count(llvm) == 0)
    {
        cout << "release faild : llvm not exist !!!" << endl;
        exit(1);
    }
    RegPtr reg = occupation.at(llvm);
    tempRegPool.insert({reg->getType(), reg});
    occupation.erase(llvm);
}

void MipsManager::push(LLVM *llvm)
{
    addCode(new StoreMips(findOccupiedReg(llvm), curStack, isIntType(llvm->)));
}