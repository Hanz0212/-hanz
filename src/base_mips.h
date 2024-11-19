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
                manager->addData(new GlobalDefMips(gDefLLVM->GDefType, gDefLLVM->varName, gDefLLVM->length, gDefLLVM->initValList));
                // todo 将全局变量塞到occupation
                manager->occupy(llvm, new LabelReg(gDefLLVM->varName));
            }
        }
        else
            break;
    }

    for (LLVM *llvm : strCodes)
    {
        GStrLLVM *gStrLLVM = dynamic_cast<GStrLLVM *>(llvm);
        manager->addData(new GlobalStrMips(gStrLLVM->loc, gStrLLVM->str));
    }

    for (LLVM *llvm : midCodes)
    {
        switch (llvm->midType)
        {
        case ALLOCA_IR:
        {
            AllocaLLVM *allocaLLVM = dynamic_cast<AllocaLLVM *>(llvm);
            RegPtr offset = manager->allocMem(allocaLLVM, allocaLLVM->length);
            manager->addAnnotation(new AnnotationMips("#" + offset->getStr() + "($sp) " + llvm->toString()));
            break;
        }
        case LOAD_IR:
        {
            LoadLLVM *loadLLVM = dynamic_cast<LoadLLVM *>(llvm);
            RegPtr desReg = manager->findOccupiedReg(loadLLVM->des);
            // desReg保证:全局局部数组一定返回reg，全局变量一定返回label, 置true一定返回tempReg,局部变量返回offset
            RegPtr rt = manager->allocTempReg(loadLLVM);
            manager->addCode(new LoadMips(rt, desReg, llvm->toString()));
            break;
        }
        case STORE_IR:
        {
            StoreLLVM *storeLLVM = dynamic_cast<StoreLLVM *>(llvm);
            RegPtr desReg = manager->findOccupiedReg(storeLLVM->des);
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
                    manager->addCode(new LiMips(rt, new IntermediateReg(constVal), ""));
                }
            }
            else
            {
                rt = manager->findOccupiedReg(storeLLVM->val);
            }
            manager->addCode(new StoreMips(rt, desReg, llvm->toString()));
            manager->tryReleaseReg(rt);
            break;
        }
        case GETELEMENTPTR_IR:
        {
            GetelementptrLLVM *array = dynamic_cast<GetelementptrLLVM *>(llvm);
            RegPtr base = manager->findOccupiedReg(array->src); // label / offset
            RegPtr offset = array->offset->midType == CONST_IR
                                ? new IntermediateReg(dynamic_cast<ConstLLVM *>(array->offset)->val * 4)
                                : manager->findOccupiedReg(array->offset); // const / reg
            RegPtr rt = manager->allocTempReg(array);
            if (base->getType() == LABEL)
            {
                if (offset->getType() != INTERMEDIATE)
                    manager->addCode(new ITypeMips(SLL_OP, offset, offset, new IntermediateReg(2), "#reg <<= 2"));
                manager->addCode(new LaMips(rt, base, offset, llvm->toString()));
            }
            else if (base->getType() == OFFSET)
            {
                if (offset->getType() == INTERMEDIATE)
                    manager->addCode(new ITypeMips(ADDIU_OP, rt, manager->sp, offset, "#reg = const+sp"));
                else
                {
                    manager->addCode(new ITypeMips(SLL_OP, offset, offset, new IntermediateReg(2), "#reg <<= 2"));
                    manager->addCode(new RTypeMips(ADD_OP, rt, manager->sp, offset, "#reg = reg+sp"));
                }
                manager->addCode(new LaMips(rt, base, rt, llvm->toString()));
            }
            break;
        }
        case ADD_IR:
        case SUB_IR:
        case AND_IR:
        case OR_IR:
        case MUL_IR:
        case SREM_IR:
        case SDIV_IR:
        {
            RTypeLLVM *rTypeLLVM = dynamic_cast<RTypeLLVM *>(llvm);
            RegPtr rs = manager->findOccupiedReg(rTypeLLVM->op1, true);
            RegPtr rt = manager->findOccupiedReg(rTypeLLVM->op2, true);
            RegPtr rd = manager->allocTempReg(rTypeLLVM);
            if (llvm->midType == MUL_IR || llvm->midType == SREM_IR || llvm->midType == SDIV_IR)
            {
                manager->addCode(new MDTypeMips(llvm->midType != MUL_IR, rs, rt, llvm->toString()));
                manager->addCode(new MFTypeMips(llvm->midType == SREM_IR, rd, ""));
            }
            else
            {
                manager->addCode(new RTypeMips(mid_type_2_mips_type.at(rTypeLLVM->midType), rd, rs, rt, llvm->toString()));
            }
            break;
        }
        case ICMP_IR:
        { // > < >= <= != ==
            IcmpLLVM *icmpLLVM = dynamic_cast<IcmpLLVM *>(llvm);
            RegPtr rs = manager->findOccupiedReg(icmpLLVM->op1, true);
            RegPtr rt = manager->findOccupiedReg(icmpLLVM->op2, true);
            RegPtr rd = manager->allocTempReg(icmpLLVM);
            switch (icmpLLVM->cmpType)
            {
            case LSS: // <
                manager->addCode(new RTypeMips(SLT_OP, rd, rs, rt, llvm->toString()));
                break;
            case GRE: // >
                manager->addCode(new RTypeMips(SLT_OP, rd, rt, rs, llvm->toString()));
                break;
            case GEQ: // >=
                manager->addCode(new RTypeMips(SLT_OP, rd, rs, rt, llvm->toString()));
                manager->addCode(new ITypeMips(XORI_OP, rd, rd, manager->one_inter, ""));
                break;
            case LEQ: // <=
                manager->addCode(new RTypeMips(SLT_OP, rd, rt, rs, llvm->toString()));
                manager->addCode(new ITypeMips(XORI_OP, rd, rd, manager->one_inter, ""));
                break;
            case EQL: // ==
                manager->addCode(new RTypeMips(XOR_OP, rd, rs, rt, llvm->toString()));
                manager->addCode(new ITypeMips(SLTIU_OP, rd, rd, manager->one_inter, "")); // 亦或后的值小于1相等
                break;
            case NEQ: // !=
                manager->addCode(new RTypeMips(XOR_OP, rd, rs, rt, llvm->toString()));
                manager->addCode(new RTypeMips(SLTU_OP, rd, manager->zero, rd, "")); // 亦或后的值大于0不相等
                break;
            }
            break;
        }
        case ZEXT_IR:
        {
            ZextLLVM *zextLLVM = dynamic_cast<ZextLLVM *>(llvm);
            RegPtr rs = manager->findOccupiedReg(zextLLVM->src, true);
            RegPtr rd = manager->allocTempReg(zextLLVM);
            manager->addCode(new RTypeMips(ADD_OP, rd, rs, manager->zero, llvm->toString()));
            break;
        }
        case TRUNC_IR:
        {
            TruncLLVM *truncLLVM = dynamic_cast<TruncLLVM *>(llvm);
            RegPtr rs = manager->findOccupiedReg(truncLLVM->src, true);
            RegPtr rt = manager->allocTempReg(truncLLVM);
            manager->addCode(new ITypeMips(ANDI_OP, rt, rs, manager->maxchar_inter, llvm->toString()));
            break;
        }
        case LABEL_IR:
        {
            LabelLLVM *labelLLVM = dynamic_cast<LabelLLVM *>(llvm);
            manager->addCode(new LabelMips(new LabelReg("label" + labelLLVM->returnTk.substr(1)), ""));
        }
        default:
            continue;
        }
    }
    return;
}