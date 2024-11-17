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

// void del_after_ret_in_block()
// {
//     /*
//     删除ret后面的死代码 如
//     ret i32 1
//     br label %7
//     */
//     int len = midCodes.size();
//     for (int i = 0; i < len; i++)
//     {
//         LLVM *llvm = midCodes[i];
//         if (llvm->midType == RET_IR)
//         {
//             i++;
//             llvm = midCodes[i];
//             for (; i < len && llvm->midType != LABEL_IR; )
//             {
//                 midCodes.erase(midCodes.begin() + i);
//                 llvm = midCodes[i];
//                 len--;
//             }
//             i--;
//         }
//     }
// }

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
}



/*

// 乘除模表达式 MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
LLVM *generate_MulExp(Node *root)
{
    LLVM *result = NULL;
    int sum = 1, flag = 0;
    bool sumFirst = false; // 默认最后sum是op1
    tk_type op, init_op = Undefined;
    LLVM *llvm = generate_UnaryExp(root->GetFirstChild());

    if (llvm->midType == CONST_IR)
    {
        sumFirst = true;
        sum = dynamic_cast<ConstLLVM *>(llvm)->val;
        flag = 1;
    }
    else
    {
        result = llvm;
    }

    for (int i = 1; i < root->childs.size(); i += 2)
    {
        op = root->GetChildAt(i)->token->type;
        Node *child = root->GetChildAt(i + 1);
        LLVM *llvm = generate_UnaryExp(child);
        if (init_op == Undefined && sumFirst != (llvm->midType == CONST_IR))
            init_op = op;

        if (llvm->midType == CONST_IR)
        {
            if (flag == 0)
            {
                sum = dynamic_cast<ConstLLVM *>(llvm)->val;
                flag = 1;
            }
            else
                sum = cal_2op(op, sum, dynamic_cast<ConstLLVM *>(llvm)->val);
        }
        else
        {
            if (result == NULL)
                result = llvm;
            else
                result = generate_midCode(new RTypeLLVM(tk_2_mid[op], result, llvm));
        }
    }

    if (result == NULL)
    {
        return new ConstLLVM(sum);
    }
    else if (sum == 1 && (init_op == MULT || init_op == DIV) || init_op == Undefined)
    {
        return result;
    }
    else
    {
        if (sumFirst)
            return generate_midCode(new RTypeLLVM(tk_2_mid[init_op], new ConstLLVM(sum), result));
        else
            return generate_midCode(new RTypeLLVM(tk_2_mid[init_op], result, new ConstLLVM(sum)));
    }
}
*/