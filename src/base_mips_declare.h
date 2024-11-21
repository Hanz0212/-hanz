
struct RegStruct;
using RegPtr = RegStruct *;
struct IntermediateReg;
struct Mips;
struct MipsManager;
struct CodeMips;
struct DataMips;
struct AnnotationMips;

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
        string result = annotation.empty() ? "" : ("\t#\t" + annotation);
        return !annotation.empty() && annotation.at(0) == '#' ? result : result + "";
    }

    void addAnnotation(string annotation)
    {
        this->annotation += annotation;
    }
};

struct MipsManager
{
private:
    int curStack; // sp
    RegPool tempRegPool = new _RegPool();
    map<LLVM *, RegPtr> occupation;
    map<string, RegPtr> funcRegs;
    // 存全局变量 str funcNAme
    map<string, RegPtr> labelRegs;
    set<string> IOFuncNames = {"getint", "getchar", "putint", "putch", "putstr"};
    bool raBeenPushed = false;

    RegPtr getFreeTempReg(LLVM *llvm);
    void release(LLVM *llvm);
    void push(LLVM *llvm);

public:
    const int STACKSIZE = 400;
    int ALLOCCNT = 0;
    RegPtr zero = new TempReg(ZERO);
    RegPtr sp = new TempReg(SP);
    vector<RegPtr> a = {new TempReg(A0), new TempReg(A1), new TempReg(A2), new TempReg(A3)};
    RegPtr v0 = new TempReg(V0);
    RegPtr ra = new TempReg(RA);
    RegPtr zero_inter = new IntermediateReg(0);
    RegPtr one_inter = new IntermediateReg(1);
    RegPtr zero_off = new OffsetReg(0);
    RegPtr maxchar_inter = new IntermediateReg(0xff);
    vector<CodeMips *> mipsCodes;
    vector<DataMips *> mipsDatas;
    string curFuncName;

    void resetFrame(string funcName);

    bool isIOFuncName(string funcName) { return IOFuncNames.find(funcName) != IOFuncNames.end(); }
    void allocStackSpace(int paramCnt);
    void freeStackSpace();

    void pushRa();
    void popRa();
    void pushAll();
    void pushFuncFParam(LLVM *llvm, int paramCnt);
    void linkFuncFParam(LLVM *llvm, int paramCnt);

    MipsManager();
    void occupy(LLVM *llvm, RegPtr reg);
    void addCode(CodeMips *code);
    void addCode_move(RegPtr des, LLVM *llvm, string annotation);
    void addData(DataMips *data);
    RegPtr addFunc(string funcName, RegPtr func);
    RegPtr findFunc(string funcName);
    RegPtr addLabel(string labelName, RegPtr label);
    RegPtr findLabel(string funcName);
    void addAnnotation(AnnotationMips *annotation);
    RegPtr findOccupiedReg(LLVM *llvm, bool needTempReg = false);
    RegPtr allocTempReg(LLVM *llvm);
    RegPtr allocMem(LLVM *llvm, int size);
    RegPtr load(LLVM *llvm);

} g_mipsManager, *manager = &g_mipsManager;
