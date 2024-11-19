// 删除br后面的死代码
void del_after_br_in_block()
{
    /*
    删除br后面的死代码 如
    br label %1
    br label %2
    */
    int len = midCodes.size();
    for (int i = 0; i < len; i++)
    {
        LLVM *llvm = midCodes[i];
        if (llvm->midType == BR_IR && dynamic_cast<BrLLVM *>(llvm)->isNoCond())
        {
            i++;
            llvm = midCodes[i];
            for (; i < len && llvm->midType != LABEL_IR;)
            {
                midCodes.erase(midCodes.begin() + i);
                llvm = midCodes[i];
                len--;
            }
            i--;
        }
    }
}


// 删除空块
void del_empty_block()
{
    int len = midCodes.size();
    for (int i = 0; i < len; i++)
    {
        LLVM *llvm = midCodes.at(i);
        if (llvm->midType == LABEL_IR)
        {
            int j;
            for (j = i + 1; j < len && midCodes.at(j)->midType == LABEL_IR; j++)
                ;
            if (j > i + 1)
            {        // 有连续标签
                j--; // 此时j是最后一个重复的label
                LabelLLVM *newLabel = dynamic_cast<LabelLLVM *>(midCodes.at(j));
                for (int k = i; k < j; k++)
                {
                    LabelLLVM *oldLabel = dynamic_cast<LabelLLVM *>(midCodes.at(k));
                    for (LLVM *oldPred : oldLabel->preds)
                    {
                        BrLLVM *br = dynamic_cast<BrLLVM *>(oldPred);
                        br->changeLabel(oldLabel, newLabel);
                    }
                }
                int delCnt = j - i;
                len -= delCnt;
                while (delCnt--)
                {
                    midCodes.erase(midCodes.begin() + i);
                }
            }
        }
    }
}

void optimize_mid()
{
    del_after_br_in_block();
    del_empty_block();
    // 待优化
    // 只使用lw和sw，当需要char变量时，lw出来，需要另一个char时如果恰好也在这个字节中，可以直接移位
}

