void mid_2_mips()
{
    for (LLVM *llvm : midCodes)
    {
        if (llvm->midType == G_VAR_DEF_IR)
        {
            GDefLLVM *gDefLLVM = dynamic_cast<GDefLLVM *>(llvm);
            if (isFuncType(gDefLLVM->GDefType))
            {
            }
            else
            {
                manager->addCode(new GlobalDefMips(gDefLLVM->GDefType, gDefLLVM->varName, gDefLLVM->length, gDefLLVM->initValList));
            }
        }
        else
            break;
    }

    for (LLVM *llvm : strCodes)
    {
        GStrLLVM *gStrLLVM = dynamic_cast<GStrLLVM *>(llvm);
        manager->addCode(new GlobalStrMips(gStrLLVM->loc, gStrLLVM->str));
    }

    for (LLVM *llvm : midCodes)
    {
        switch (llvm->midType)
        {
        case ALLOCA_IR:
        {
            AllocaLLVM *allocaLLVM = dynamic_cast<AllocaLLVM *>(llvm);
            manager->allocMem(allocaLLVM, allocaLLVM->length * 4);
            break;
        }
        case LOAD_IR:
        {
            LoadLLVM *loadLLVM = dynamic_cast<LoadLLVM *>(llvm);
            RegPtr rs = manager->findOccupiedReg(loadLLVM->des);
            RegPtr rt = manager->allocTempReg(loadLLVM);
            manager->addCode(new LoadMips(rt, rs->offset, isIntType(loadLLVM->loadType)));
            break;
        }
        case STORE_IR:
        {
            StoreLLVM *storeLLVM = dynamic_cast<StoreLLVM *>(llvm);
            RegPtr rs = manager->findOccupiedReg(storeLLVM->des);
            RegPtr rt;
            if (storeLLVM->val->midType == CONST_IR)
            {
                int constVal = dynamic_cast<ConstLLVM *>(storeLLVM->val)->val;
                if (constVal == 0)
                {
                    rt = manager->zero;
                }
                else
                {
                    rt = manager->allocTempReg(storeLLVM->val);
                    manager->addCode(new LiMips(rt, constVal));
                }
            }
            else
            {
                rt = manager->findOccupiedReg(storeLLVM->val);
            }
            manager->addCode(new StoreMips(rt, rs->offset, isIntType(storeLLVM->storeType)));
            break;
        }
        case ADD_IR:
        case SUB_IR:
        case AND_IR:
        case OR_IR:
        {
            RTypeLLVM *rTypeLLVM = dynamic_cast<RTypeLLVM *>(llvm);
            RegPtr rs = manager->findOccupiedReg(rTypeLLVM->op1);
            RegPtr rt = manager->findOccupiedReg(rTypeLLVM->op2);
            RegPtr rd = manager->allocTempReg(rTypeLLVM);
            manager->addCode(new RTypeMips(mid_type_2_mips_type.at(rTypeLLVM->midType), rs, rt, rd));
            break;
        }
        case MUL_IR:
        case SREM_IR:
        case SDIV_IR:
        {
            MDTypeMips *mDTypeMips = dynamic_cast<MDTypeMips *>(llvm);
            // manager->addCode(new MDTypeMips(mid_type_2_mips_type.at(llvm->midType), T0, T1));
            // if (llvm->midType == MUL_IR || llvm->midType == SDIV_IR)
            //     manager->addCode(new MFTypeMips(T0, false));
            // else
            //     manager->addCode(new MFTypeMips(T0, true));
            break;
        }
        case ICMP_IR:
        {
            break;
        }
        case LABEL_IR:
        {
            LabelLLVM *labelLLVM = dynamic_cast<LabelLLVM *>(llvm);
            manager->addCode(new LabelMips(labelLLVM->returnTk));
        }
        default:
            continue;
        }
    }
    return;
}