
struct RegStruct;
using RegPtr = RegStruct *;
struct IntermediateReg;
struct Mips;
struct MipsManager;
struct CodeMips;
struct DataMips;

struct Mips
{
    mips_type mipsType;
    string annotation;

    Mips(mips_type mipsType, string annotation)
    {
        this->mipsType = mipsType;
        this->annotation = annotation;
    }

    virtual string toString()
    {
        return annotation.empty() ? "" : "\t#\t" + annotation;
    }

    void addAnnotation(string annotation)
    {
        this->annotation += annotation;
    }
};

struct MipsManager
{
private:
    int curStack;      // sp
    RegPool tempRegPool = new _RegPool();
    map<LLVM *, RegPtr> occupation;

    RegPtr getFreeTempReg(LLVM *llvm);
    void release(LLVM *llvm);
    void push(LLVM *llvm);

public:
    RegPtr zero = new TempReg(ZERO);
    RegPtr sp = new TempReg(SP);
    RegPtr zero_inter = new IntermediateReg(0);
    RegPtr one_inter = new IntermediateReg(1);
    RegPtr zero_off = new OffsetReg(0);
    RegPtr maxchar_inter = new IntermediateReg(0xff);
    vector<CodeMips *> mipsCodes;
    vector<DataMips *> mipsDatas;

    MipsManager();
    void occupy(LLVM *llvm, RegPtr reg);
    void tryReleaseReg(RegPtr reg);
    void addCode(CodeMips *code);
    void addData(DataMips *data);
    RegPtr findOccupiedReg(LLVM *llvm, bool needTempReg = false);
    RegPtr allocTempReg(LLVM *llvm);
    RegPtr allocMem(LLVM *llvm, int size);
    RegPtr load(LLVM *llvm);

} g_mipsManager, *manager = &g_mipsManager;
