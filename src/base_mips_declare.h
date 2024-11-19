
struct RegStruct;
using RegPtr = RegStruct *;
struct OffsetReg;
struct Mips;
struct MipsManager;

struct Mips
{
    mips_type mipsType;
    Mips(mips_type mipsType)
    {
        this->mipsType = mipsType;
    }

    virtual string toString()
    {
        cout << "struct hasnt override toString : " << mips_type_2_str.at(mipsType) << endl;
        exit(1);
    }
};

struct MipsManager
{
private:
    int curStack;      // sp
    int tempCount = 0; // 临时寄存器分配号，每次申请都+1 mod n
    int TEMPREGCNT = temp_reg_types.size();
    map<reg_type, RegPtr> tempRegPool;
    map<LLVM *, RegPtr> occupation;


public:
    RegPtr zero = new RegStruct(ZERO);
    RegPtr sp = new RegStruct(SP);
    vector<Mips *> mipsCodes;

    MipsManager();
    void addCode(Mips *code);
    void occupy(LLVM *llvm, RegPtr reg);
    RegPtr findOccupiedReg(LLVM *llvm);
    RegPtr getFreeTempReg(LLVM *llvm);
    RegPtr allocTempReg(LLVM *llvm);
    RegPtr allocMem(LLVM *llvm, int size);
    void release(LLVM *llvm);
    void push(LLVM *llvm);

} g_mipsManager, *manager = &g_mipsManager;

