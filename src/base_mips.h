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
                mipsCodes.push_back(new GlobalDefMips(gDefLLVM->GDefType, gDefLLVM->varName, gDefLLVM->length, gDefLLVM->initValList));
            }
        }
        else
            break;
    }

    for (LLVM *llvm : strCodes)
    {
        GStrLLVM *gStrLLVM = dynamic_cast<GStrLLVM *>(llvm);
        mipsCodes.push_back(new GlobalStrMips(gStrLLVM->loc, gStrLLVM->str));
    }
    return;
}