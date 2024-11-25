void create_str()
{
    for (LLVM *llvm : strCodes)
    {
        GStrLLVM *gStrLLVM = dynamic_cast<GStrLLVM *>(llvm);
        manager->addData(new GlobalStrMips(gStrLLVM->loc, gStrLLVM->str));
        manager->addLabel(to_string(gStrLLVM->loc), new LabelReg("JHZSTR_" + to_string(gStrLLVM->loc)));
    }
}

// 分析函数 存入funcName labelName 全局变量 str 并且保存dataCodes
void save_global()
{
    int labelCnt = 0;
    for (int i = 0; i < midCodes.size(); i++)
    {
        LLVM *llvm = midCodes.at(i);
        if (llvm->midType == G_VAR_DEF_IR)
        {
            GDefLLVM *gDefLLVM = dynamic_cast<GDefLLVM *>(llvm);
            if (!isFuncType(gDefLLVM->GDefType))
            {
                manager->addData(new GlobalDefMips(gDefLLVM->GDefType, "JHZVAR_" + gDefLLVM->varName, gDefLLVM->length, gDefLLVM->initValList));
                manager->addLabel(gDefLLVM->varName, new LabelReg("JHZVAR_" + gDefLLVM->varName));
            }
        }
        else if (llvm->midType == G_FUNC_DEF_IR)
        {
            labelCnt = 1;
            GDefLLVM *gDefLLVM = dynamic_cast<GDefLLVM *>(llvm);
            manager->curFuncName = gDefLLVM->varName;
            manager->addFunc(gDefLLVM->varName, new LabelReg("JHZFUNC_" + gDefLLVM->varName + "_0"));
        }
        else if (llvm->midType == LABEL_IR)
        {
            LabelLLVM *labelLLVM = dynamic_cast<LabelLLVM *>(llvm);
            string str = manager->curFuncName + labelLLVM->returnTk;
            manager->addLabel(str, new LabelReg("JHZFUNC_" + manager->curFuncName + "_" + to_string(labelCnt)));
            labelCnt++;
        }
    }
}

void create_IO_code(CallLLVM *call)
{
    const map<string, int> m = {
        {"getint", 5},
        {"getchar", 12},
        {"putint", 1},
        {"putch", 11},
        {"putstr", 4}};
    if (call->funcName == "putint" || call->funcName == "putch")
        manager->addCode_move(manager->a.at(0), call->src, "");
    else if (call->funcName == "putstr")
        manager->addCode(new LaMips(manager->a.at(0), manager->findLabel(to_string(dynamic_cast<GStrLLVM *>(call->gStr)->loc)), manager->zero_inter, ""));
    manager->addCode(new LiMips(manager->v0, new IntermediateReg(m.at(call->funcName)), call->funcName));
    manager->addCode(new SyscallMips());
    if (call->funcName == "getint" || call->funcName == "getchar")
        manager->occupy(call, manager->v0);
}

void create_code()
{
    for (int i = 0; i < midCodes.size(); i++)
    {
        LLVM *llvm = midCodes.at(i);
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
            RegPtr rt = manager->allocTempReg(loadLLVM);
            RegPtr desReg = manager->findOccupiedReg(loadLLVM->des);
            if (loadLLVM->des->midType == GETELEMENTPTR_IR && desReg->getType() == OFFSET)
            {
                RegPtr tmp = manager->allocTempReg(llvm);
                manager->addCode(new LoadMips(tmp, desReg, "#load address"));
                manager->addCode(new LoadMips(rt, tmp, llvm->toString()));
            }
            else
                manager->addCode(new LoadMips(rt, desReg, llvm->toString()));
            break;
        }
        case STORE_IR:
        {
            StoreLLVM *storeLLVM = dynamic_cast<StoreLLVM *>(llvm);
            RegPtr rt;
            if (storeLLVM->val->midType == CONST_IR)
            {
                int constVal = dynamic_cast<ConstLLVM *>(storeLLVM->val)->val;
                if (constVal == 0)
                    rt = manager->zero;
                else
                {
                    rt = manager->allocTempReg(storeLLVM->val);
                    manager->addCode(new LiMips(rt, new IntermediateReg(constVal), ""));
                }
            }
            else
                rt = manager->findOccupiedReg(storeLLVM->val, true);

            RegPtr desReg = manager->findOccupiedReg(storeLLVM->des);
            if (storeLLVM->des->midType == GETELEMENTPTR_IR && desReg->getType() == OFFSET)
            {
                RegPtr tmp = manager->allocTempReg(llvm);
                manager->addCode(new LoadMips(tmp, desReg, "#load address"));
                manager->addCode(new StoreMips(rt, tmp, llvm->toString()));
            }
            else
                manager->addCode(new StoreMips(rt, desReg, llvm->toString()));
            break;
        }
        case GETELEMENTPTR_IR:
        {
            GetelementptrLLVM *array = dynamic_cast<GetelementptrLLVM *>(llvm);
            RegPtr base = manager->findOccupiedReg(array->src); // label / offset
            RegPtr offset = array->offset->midType == CONST_IR
                                ? new IntermediateReg(dynamic_cast<ConstLLVM *>(array->offset)->val << 2)
                                : manager->findOccupiedReg(array->offset, true); // const / reg
            RegPtr rt = manager->allocTempReg(array);

            // if (offset->getType() == INTERMEDIATE)
            //     manager->addCode(new LaMips(rt, new OffsetReg(offset->val), base, llvm->toString()));
            // else // offset 为 reg
            // {
            //     manager->addCode(new ITypeMips(SLL_OP, offset, offset, new IntermediateReg(2), "#reg <<= 2"));
            //     manager->addCode(new RTypeMips(ADD_OP, rt, base, offset, "#rt = offset+base"));
            //     manager->addCode(new LaMips(rt, manager->zero_off, rt, llvm->toString()));
            // }
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
            else if (in32Reg(base->getType())) // 函数形参数组
            {
                if (offset->getType() == INTERMEDIATE)
                    manager->addCode(new LaMips(rt, new OffsetReg(offset->val), base, llvm->toString()));
                else // offset 为 reg
                {
                    manager->addCode(new ITypeMips(SLL_OP, offset, offset, new IntermediateReg(2), "#reg <<= 2"));
                    manager->addCode(new RTypeMips(ADD_OP, rt, base, offset, "#rt = offset+base"));
                    manager->addCode(new LaMips(rt, manager->zero_off, rt, llvm->toString()));
                }
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
            RegPtr rd = manager->allocTempReg(rTypeLLVM);
            if (rTypeLLVM->op1->midType == CONST_IR && rTypeLLVM->op2->midType == CONST_IR)
            {
                int ans = mid_cal_2op(rTypeLLVM->midType,
                                      dynamic_cast<ConstLLVM *>(rTypeLLVM->op1)->val,
                                      dynamic_cast<ConstLLVM *>(rTypeLLVM->op2)->val);
                manager->addCode(new LiMips(rd, new IntermediateReg(ans), "#op1 op2 both const"));
                break;
            }
            if (rTypeLLVM->op1->midType != CONST_IR && rTypeLLVM->op2->midType != CONST_IR)
            {
                RegPtr rs = manager->findOccupiedReg(rTypeLLVM->op1, true);
                RegPtr rt = manager->findOccupiedReg(rTypeLLVM->op2, true);
                manager->addCode(new RTypeMips(mid_type_2_mips_type.at(rTypeLLVM->midType), rd, rs, rt, llvm->toString()));
                if (llvm->midType == MUL_IR || llvm->midType == SREM_IR || llvm->midType == SDIV_IR)
                    manager->addCode(new MFTypeMips(llvm->midType == SREM_IR, rd, ""));
                break;
            }
            RegPtr rs, rt;
            if (rTypeLLVM->op1->midType == CONST_IR)
            {
                rs = manager->allocTempReg(rTypeLLVM->op1);
                manager->addCode(new LiMips(rs, new IntermediateReg(dynamic_cast<ConstLLVM *>(rTypeLLVM->op1)->val), ""));
                rt = manager->findOccupiedReg(rTypeLLVM->op2, true);
            }
            else if (rTypeLLVM->op2->midType == CONST_IR)
            {
                rs = manager->findOccupiedReg(rTypeLLVM->op1, true);
                rt = manager->allocTempReg(rTypeLLVM->op2);
                manager->addCode(new LiMips(rt, new IntermediateReg(dynamic_cast<ConstLLVM *>(rTypeLLVM->op2)->val), ""));
            }

            if (llvm->midType == MUL_IR || llvm->midType == SREM_IR || llvm->midType == SDIV_IR)
            {
                manager->addCode(new RTypeMips(mid_type_2_mips_type.at(rTypeLLVM->midType), rd, rs, rt, llvm->toString()));
                manager->addCode(new MFTypeMips(llvm->midType == SREM_IR, rd, ""));
            }
            else
                manager->addCode(new RTypeMips(mid_type_2_mips_type.at(rTypeLLVM->midType), rd, rs, rt, llvm->toString()));

            break;
        }
        case ICMP_IR:
        { // > < >= <= != ==
            IcmpLLVM *icmpLLVM = dynamic_cast<IcmpLLVM *>(llvm);
            RegPtr rd = manager->allocTempReg(icmpLLVM);
            if (icmpLLVM->op1->midType == CONST_IR)
            {
                RegPtr reg = manager->allocTempReg(icmpLLVM->op1);
                manager->addCode(new LiMips(reg, new IntermediateReg(dynamic_cast<ConstLLVM *>(icmpLLVM->op1)->val), ""));
            }
            if (icmpLLVM->op2->midType == CONST_IR)
            {
                RegPtr reg = manager->allocTempReg(icmpLLVM->op2);
                manager->addCode(new LiMips(reg, new IntermediateReg(dynamic_cast<ConstLLVM *>(icmpLLVM->op2)->val), ""));
            }
            RegPtr rs = manager->findOccupiedReg(icmpLLVM->op1, true);
            RegPtr rt = manager->findOccupiedReg(icmpLLVM->op2, true);
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
            manager->addCode(new LabelMips(manager->findLabel(manager->curFuncName + labelLLVM->returnTk), ""));
            break;
        }
        case G_FUNC_DEF_IR:
        {
            GDefLLVM *gDefLLVM = dynamic_cast<GDefLLVM *>(llvm);
            manager->resetFrame(gDefLLVM->varName);
            // 将形参与a寄存器建立映射
            int len = gDefLLVM->funcFParams.size();
            for (int j = 0; j < len; j++)
            {
                AllocaLLVM *alloca = dynamic_cast<AllocaLLVM *>(syb_2_llvm.at(gDefLLVM->funcFParams.at(j)));
                FuncFParamLLVM *param = dynamic_cast<FuncFParamLLVM *>(alloca->funcFParam);
                if (j < 4)
                    manager->occupy(param, manager->a.at(j));
                else
                    manager->occupy(param, manager->allocFuncFParamStackSpaces());
            }

            // 申请栈空间
            manager->addCode(new ITypeMips(ADDIU_OP, manager->sp, manager->sp, manager->allocStackSpace(), "申请栈空间"));
            if (manager->curFuncName != "main")
                manager->pushRa();
            break;
        }
        case FUNC_END_IR:
        {
            break;
        }
        case CALL_IR:
        {
            CallLLVM *callLLVM = dynamic_cast<CallLLVM *>(llvm);
            if (manager->isIOFuncName(callLLVM->funcName))
            {
                create_IO_code(callLLVM);
                break;
            }

            // 传参到a寄存器
            for (int j = 0; j < callLLVM->params.size(); j++)
            {
                if (j < 4)
                    manager->addCode_move(manager->a.at(j), callLLVM->params.at(j), "#save a" + to_string(j));
                else
                {
                    if (callLLVM->params.at(j)->midType == CONST_IR)
                    {
                        RegPtr reg = manager->allocTempReg(callLLVM->params.at(j));
                        manager->addCode_move(reg, callLLVM->params.at(j), "");
                        manager->addCode(new StoreMips(reg, new OffsetReg(-4 * (j - 3)), "#save a" + to_string(j)));
                    }
                    else
                    {
                        RegPtr reg = manager->findOccupiedReg(callLLVM->params.at(j), true);
                        manager->addCode(new StoreMips(reg, new OffsetReg(-4 * (j - 3)), "#save a" + to_string(j)));
                    }
                }
            }
            manager->pushAll(); // 腾出所有寄存器
            manager->addCode(new JTypeMips(JAL_OP, manager->findFunc(callLLVM->funcName), llvm->toString()));
            manager->addCode(new NopMips());
            // 若有返回值 寄存器存储并建立映射
            if (callLLVM->retType != VoidFunc)
            {
                RegPtr retVal = manager->allocTempReg(callLLVM);
                manager->addCode(new ITypeMips(ADDIU_OP, retVal, manager->v0, manager->zero_inter, "#get ret val"));
            }
            break;
        }
        case RET_IR:
        {
            RetLLVM *ret = dynamic_cast<RetLLVM *>(llvm);
            manager->correctStackSpace();

            if (manager->curFuncName != "main")
            {
                if (ret->retType != VoidFunc)
                { // 保存返回值
                    manager->addCode(new ITypeMips(ADDIU_OP, manager->v0, manager->findOccupiedReg(ret->src),
                                                   manager->zero_inter, "#save return value"));
                }

                manager->popRa();
                manager->addCode(new ITypeMips(ADDIU_OP, manager->sp, manager->sp,
                                               new IntermediateReg(manager->getSuitStackSpace()), "恢复栈空间"));
                manager->addCode(new JRMips());
                manager->addCode(new NopMips());
            }
            break;
        }
        case BR_IR:
        {
            BrLLVM *br = dynamic_cast<BrLLVM *>(llvm);
            RegPtr label1 = manager->findLabel(manager->curFuncName + br->label1Tk);

            if (br->isNoCond())
            {
                manager->addCode(new JTypeMips(J_OP, label1, ""));
                manager->addCode(new NopMips());
                break;
            }

            RegPtr label2 = manager->findLabel(manager->curFuncName + br->label2Tk);
            RegPtr op1 = manager->findOccupiedReg(br->cmp);
            manager->addCode(new BTypeMips(BGTZ_OP, op1, NULL, label1, ""));
            manager->addCode(new NopMips());
            manager->addCode(new JTypeMips(J_OP, label2, ""));
            manager->addCode(new NopMips());
            break;
        }
        default:
            continue;
        }
    }
}

void mid_2_mips()
{
    save_global();
    create_str();
    manager->addCode(new JTypeMips(J_OP, manager->findFunc("main"), "START"));
    manager->addCode(new NopMips());
    create_code();
    manager->addCode(new LiMips(manager->v0, new IntermediateReg(10), "END"));
    manager->addCode(new SyscallMips());
    return;
}