#pragma once

#include <optional>
#include <set>

#include "ir.h"

namespace pick::x86_64
{
    /*
        x86_64���W�X�^�̗񋓁B
        R8-R15��32bit�v�Z�Ŏg�p����ƁAREX���K�v�Ȃ��߁A
        32bit�^��RAX, RCX, RDX, RBX, RSI, RDI��D��I�ɔz�u����B
        64bit�^��R8-R15��D��I�Ɏg�p����B
    */
    enum struct Register
    {
        /*
            �A�L�������[�^���W�X�^�B
            ���l�n�̉��Z���߂ŁA���ߒ����Z���Ȃ�ꍇ������B
            64bit * 64bit�̌��ʂ̉���64bit���i�[����A
            128bit / 64bit�̔폜���̉���64bit���i�[����A
            ���Z�̏����i�[�����B
            �߂�l���i�[�����B
            �Ăяo����Ŕj�󂵂Ă��悢�B
            ����ȊO�͔ėp���W�X�^�Ƃ��Ďg�p�ł���B
        */
        RAX,
        /*
            �J�E���^���W�X�^�B
            �J��Ԃ��񐔂̐ݒ�Ɏg�p����B
            Windows�ł͈�Ԗڂ̐����������i�[����B
            Linux�ł͑�l�������i�[����B
            �Ăяo����Ŕj�󂵂Ă��悢�B
            ����ȊO�͔ėp���W�X�^�Ƃ��Ďg�p�ł���B
        */
        RCX,
        /*
            �f�[�^���W�X�^�B
            64bit * 64bit�̌��ʂ̏��64bit���i�[����A
            128bit / 64bit�̔폜���̏��64bit���i�[����A
            ���Z�̏�]���i�[�����B
            Windows�ł͓�Ԗڂ̐����������i�[����B
            Linux�ł͑�O�������i�[����B
            �Ăяo����Ŕj�󂵂Ă��悢�B
            ����ȊO�͔ėp���W�X�^�Ƃ��Ďg�p�ł���B
        */
        RDX,
        /*
            �x�[�X���W�X�^�B
            �Ăяo����ɂ���ĕێ������K�v������B
            x64�ł͒P�Ȃ�ėp���W�X�^�B
        */
        RBX,
        /*
            �X�^�b�N�|�C���^���W�X�^�B
            �Ăяo����ɂ���ĕێ������K�v������B
            �Ăяo����Ŕj�󂵂Ă��悢�B
            �ǂݏ������邱�Ƃ͂قڂȂ��B
        */
        RSP,
        /*
            �x�[�X�|�C���^���W�X�^�B
            �X�^�b�N�t���[���̃|�C���^�B
            �Ăяo����ɂ���ĕێ������K�v������B
            �ǂݏ������邱�Ƃ͂قڂȂ��B
        */
        RBP,
        /*
            �\�[�X�C���f�b�N�X���W�X�^�B
            �����񖽗ߌn�ŃR�s�[���������ꍇ�Ɏg�p����B
            Linux�ł͑��������i�[����B
            �Ăяo����ɂ���ĕێ������K�v������B
            ����ȊO�͒P�Ȃ�ėp���W�X�^�B
        */
        RSI,
        /*
            �f�X�e�B�l�[�V�����C���f�b�N�X���W�X�^�B
            �����񖽗ߌn�ŃR�s�[��������ꍇ�Ɏg�p����B
            Linux�ł͑��������i�[����B
            �Ăяo����ɂ���ĕێ������K�v������B
            ����ȊO�͒P�Ȃ�ėp���W�X�^�B
        */
        RDI,
        /*
            x64�g���ėp���W�X�^�B
            REX�v���t�B�b�N�X���K�v�Ȃ��߁A���ߒ���1Byte�����Ȃ�B
            Windows�ł͎O�Ԗڂ̐����������i�[����B
            Linux�ł͑�܈������i�[����B
            �Ăяo����Ŕj�󂵂Ă��悢�B
            �����n���Ɏg�p�����B
        */
        R8,
        /*
            x64�g���ėp���W�X�^�B
            REX�v���t�B�b�N�X���K�v�Ȃ��߁A���ߒ���1Byte�����Ȃ�B
            Windows�ł͎l�Ԗڂ̐����������i�[����B
            Linux�ł͑�Z�������i�[����B
            �Ăяo����Ŕj�󂵂Ă��悢�B
            �����n���Ɏg�p�����B
        */
        R9,
        /*
            x64�g���ėp���W�X�^�B
            REX�v���t�B�b�N�X���K�v�Ȃ��߁A���ߒ���1Byte�����Ȃ�B
            �Ăяo����ɂ���ĕێ������K�v������B
            �����n���Ɏg�p�����B
        */
        R10,
        /*
            x64�g���ėp���W�X�^�B
            REX�v���t�B�b�N�X���K�v�Ȃ��߁A���ߒ���1Byte�����Ȃ�B
            �����n���Ɏg�p�����B
            �Ăяo����ɂ���ĕێ������K�v������B
            Linux�V�X�e���R�[���̌Ăяo���ŕۑ�����Ȃ��B
        */
        R11,
        /*
            x64�g���ėp���W�X�^�B
            REX�v���t�B�b�N�X���K�v�Ȃ��߁A���ߒ���1Byte�����Ȃ�B
            �Ăяo����ɂ���ĕێ������K�v������B
            �����n���Ɏg�p�����B
        */
        R12,
        /*
            x64�g���ėp���W�X�^�B
            REX�v���t�B�b�N�X���K�v�Ȃ��߁A���ߒ���1Byte�����Ȃ�B
            �Ăяo����ɂ���ĕێ������K�v������B
            �����n���Ɏg�p�����B
        */
        R13,
        /*
            x64�g���ėp���W�X�^�B
            REX�v���t�B�b�N�X���K�v�Ȃ��߁A���ߒ���1Byte�����Ȃ�B
            �Ăяo����ɂ���ĕێ������K�v������B
            �����n���Ɏg�p�����B
        */
        R14,
        /*
            x64�g���ėp���W�X�^�B
            REX�v���t�B�b�N�X���K�v�Ȃ��߁A���ߒ���1Byte�����Ȃ�B
            �Ăяo����ɂ���ĕێ������K�v������B
            �����n���Ɏg�p�����B
        */
        R15,
        /*
        XMM0,
        XMM1,
        XMM2,
        XMM3,
        XMM4,
        XMM5,
        XMM6,
        XMM7,
        XMM8,
        XMM9,
        XMM10,
        XMM11,
        XMM12,
        XMM13,
        XMM14,
        XMM15
        */
    };

    enum struct RelocationType
    {
        ConstantSymbol,         // X86_64::constSymbols���̃V���{���̍Ĕz�u���
        RuntimeSymbol,          // X86_64::runtimeSymbols���̃V���{���̍Ĕz�u���
        Function,               // X86_64::routines���̊֐��̍Ĕz�u���
        Extern,
    };
    struct Relocation
    {
        uint64_t indexOfRoutine;  // ���[�`���R�[�h���̍Ĕz�u���̂���ꏊ�̃I�t�Z�b�g
        RelocationType type;
        int32_t offset;           // �z���\���̗̂v�f�ւ̃A�N�Z�X�Ȃǂ̃x�[�X�A�h���X�{�I�t�Z�b�g�A�h���X���K�v�ł���Ύg�p����
        uint64_t indexOfData;     // X86_64���̂��ꂼ��̃C���f�b�N�X
        std::string ext;          // type == Extern
    };

    enum struct Opecode
    {
        _NONE,
        ADD,
        OR,
        ADC,
        SBB,
        AND,
        SUB,
        XOR,
        CMP,
        MUL,
        IMUL,
        DIV,
        IDIV,
        IMOD,
        NEG,
        NOT,
        TEST,
        MOV,
        NOP,
        HLT,
        JMP,
        JE,
        JNE,
        JG,
        JGE,
        JL,
        JLE,
        CALL,
        LEAVE,
        RET,
        PUSH,
        POP,
    };
    enum struct RMType
    {
        Register,
        Memory,
        Address,
        Constant,
        Runtime,
    };
    struct Memory
    {
        Register base;
        std::optional<Register> index;
        int32_t scale;
        int32_t disp;
    };
    struct RM
    {
        RMType type;
        union
        {
            Register reg;
            Memory mem;
            uint64_t address;
        };
    };
    struct Operation
    {
        Opecode opecode;
        size_t size;        // �I�y�����h�̃o�C�g��
        int32_t offset;      // �z���\���̗̂v�f�ւ̃A�N�Z�X�ȂǂŎg�p
        RM op1;
        std::optional<RM> op2;
        std::optional<int32_t> imm;
        static Operation r(Opecode opecode, Register reg);
        static Operation a(Opecode opecode, Register reg);
        static Operation a(Opecode opecode, Register base, int32_t ref);
        static Operation i(Opecode opecode, int32_t imm);
        static Operation rr(Opecode opecode, Register dist, Register src, size_t size);
        static Operation ra(Opecode opecode, Register dist, Register src, size_t size);
        static Operation ra(Opecode opecode, Register dist, RMType type, uint64_t address, int32_t offset, size_t size);
        static Operation ra(Opecode opecode, Register dist, Register base, int32_t ref, size_t size);
        static Operation ra(Opecode opecode, Register dist, Register base, Register index, int scale, int32_t ref, size_t size);
        static Operation ri(Opecode opecode, Register dist, int32_t imm, size_t size);
        static Operation ar(Opecode opecode, Register dist, Register src, size_t size);
        static Operation ar(Opecode opecode, RMType type, uint64_t address, int32_t offset, Register src, size_t size);
        static Operation ar(Opecode opecode, Register base, int32_t ref, Register src, size_t size);
        static Operation ar(Opecode opecode, Register base, Register index, int scale, int32_t ref, Register src, size_t size);
        static Operation ai(Opecode opecode, Register dist, int32_t imm, size_t size);
        static Operation ai(Opecode opecode, RMType type, uint64_t address, int32_t offset, int32_t imm, size_t size);
        static Operation ai(Opecode opecode, Register base, int32_t ref, int32_t imm, size_t size);
        static Operation jcc(Opecode opecode, size_t indexOfBlock);
        static Operation call(size_t indexOfFunction);
        static Operation ret();
    };
    struct Routine
    {
        uint64_t address;   // �����J�Ŏg�p����B
        std::vector<std::vector<Operation>> ops;    // ir�̃u���b�N��؂�̖��ߗ�
        std::vector<uint8_t> code;
        std::vector<Relocation> relocs;
    };
    enum struct ConstantSymbolType
    {
        _NONE,
        Immediate,
        Relocation
    };
    // .rdata�Z�N�V�����ɔz�u�����V���{��
    struct ConstantSymbol
    {
        uint64_t address;       // exe.h�Ŏg�p
        std::string name;
        ConstantSymbolType type;
        union {
            std::vector<uint8_t> imm;
            Relocation reloc;
        };
        ConstantSymbol(const std::string& name, const std::vector<uint8_t>& imm);
        ConstantSymbol(const std::string& name, const Relocation& reloc);
        ConstantSymbol(const ConstantSymbol& con);
        ~ConstantSymbol();
        ConstantSymbol& operator=(const ConstantSymbol& con);
    private:
        void clear();
    };
    // .data�Z�N�V�����ɔz�u�����V���{��
    struct RuntimeSymbol
    {
        uint64_t address;       // exe.h�Ŏg�p
        std::string name;
        uint64_t sizeOfSymbol;
        ir::Function* init;
        Routine initRoutine;
    };
    struct X86_64
    {
        Routine invokeMain;
        std::vector<Routine> routines;
        std::set<std::string> exts;
        std::vector<ConstantSymbol> constSymbols;
        std::vector<RuntimeSymbol> runtimeSymbols;
    };
    enum struct VarType
    {
        Immediate,
        Register,
        Spill,
        Phi,
        ConstantSymbol,
        RuntimeSymbol,
        FunctionSymbol,
    };
    struct Var
    {
        VarType type;
        union
        {
            int64_t imm;
            x86_64::Register reg;
            int32_t offset;
            size_t indexOfData;
        };
    };
    struct RegState
    {
        Var* inUse = nullptr;
        bool hasBeenUsed = false;
    };
    enum struct Cond
    {
        Undef,
        Equal,
        NotEqual,
        GreaterEqual,
        GreaterThan,
        LessEqual,
        LessThan
    };
    class X86_64Compiler
    {
        ir::IntermediateRepresentation ir;
        X86_64 x86_64;
        Result<std::vector<std::vector<Operation>>, std::vector<std::string>> commonCompileRoutine(const ir::Function* fn, std::unordered_map<x86_64::Register, RegState>& regs, int32_t& offset);
        Result<_, std::vector<std::string>> compileOperation(Routine& routine, int32_t len, const std::vector<uint8_t>& prologue, const std::vector<uint8_t>& epilogue);
        Result<Routine, std::vector<std::string>> compileRoutine(const ir::Function* fn);
    public:
        X86_64Compiler(const ir::IntermediateRepresentation& ir);
        Result<X86_64, std::vector<std::string>> compile();
    };

    enum struct ImmGrp1
    {
        ADD,
        OR,
        ADC,
        SBB,
        AND,
        SUB,
        XOR,
        CMP,
    };
    // x64�ł̓������A�N�Z�X��32bit�ł����ł��Ȃ�?

    void addRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    void addRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> addRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size);
    void addRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size);
    void addRA(std::vector<uint8_t>& code, Register dist, Register base, Register index, int scale, int32_t ref, size_t size);
    // ���l���Z��32bit�܂ŁB64bit�͈�x���l��mov���Ă���v�Z�B
    void addRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    void addAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> addAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size);
    void addAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size);
    void addAR(std::vector<uint8_t>& code, Register base, Register index, int scale, int32_t ref, Register src, size_t size);
    void addAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    std::vector<size_t> addAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size);
    void addAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size);

    void subRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    void subRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> subRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size);
    void subRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size);
    void subRA(std::vector<uint8_t>& code, Register dist, Register base, Register index, int scale, int32_t ref, size_t size);
    void subRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    void subAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> subAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size);
    void subAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size);
    void subAR(std::vector<uint8_t>& code, Register base, Register index, int scale, int32_t ref, Register src, size_t size);
    void subAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    std::vector<size_t> subAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size);
    void subAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size);

    void imulRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    void imulRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> imulRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size);
    void imulRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size);
    void imulRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    void imulAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> imulAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size);
    void imulAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size);
    void imulAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    std::vector<size_t> imulAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size);
    void imulAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size);

    void idivRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    void idivRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> idivRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size);
    void idivRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size);
    void idivRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    void idivAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> idivAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size);
    void idivAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size);
    void idivAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    std::vector<size_t> idivAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size);
    void idivAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size);

    void imodRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    void imodRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> imodRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size);
    void imodRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size);
    void imodRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    void imodAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> imodAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size);
    void imodAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size);
    void imodAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    std::vector<size_t> imodAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size);
    void imodAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size);

    void cmpRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    void cmpRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> cmpRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size);
    void cmpRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size);
    void cmpRA(std::vector<uint8_t>& code, Register dist, Register base, Register index, int scale, int32_t ref, size_t size);
    void cmpRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    void cmpAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> cmpAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size);
    void cmpAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size);
    void cmpAR(std::vector<uint8_t>& code, Register base, Register index, int scale, int32_t ref, Register src, size_t size);
    void cmpAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    std::vector<size_t> cmpAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size);
    void cmpAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size);

    void movRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    void movRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> movRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size);
    void movRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size);
    void movRA(std::vector<uint8_t>& code, Register dist, Register base, Register index, int scale, int32_t ref, size_t size);
    void movRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    void movAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> movAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size);
    void movAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size);
    void movAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    std::vector<size_t> movAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size);
    void movAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size);

    void pushR(std::vector<uint8_t>& code, Register reg);
    void pushA(std::vector<uint8_t>& code, uint32_t address);
    void pushA(std::vector<uint8_t>& code, Register base, int32_t ref);
    void pushI(std::vector<uint8_t>& code, int8_t imm);
    void pushI(std::vector<uint8_t>& code, int32_t imm);

    void popR(std::vector<uint8_t>& code, Register reg);
    void popA(std::vector<uint8_t>& code, uint32_t address);
    void popA(std::vector<uint8_t>& code, Register base, int32_t ref);

    void negR(std::vector<uint8_t>& code, Register reg);
    void negA(std::vector<uint8_t>& code, Register base, int32_t ref);

    void callA(std::vector<uint8_t>& code, uint32_t address);
    void callR(std::vector<uint8_t>& code, Register reg);

    void je(std::vector<uint8_t>& code, uint32_t address);
    void jne(std::vector<uint8_t>& code, uint32_t address);
    void jg(std::vector<uint8_t>& code, uint32_t address);
    void jge(std::vector<uint8_t>& code, uint32_t address);
    void jl(std::vector<uint8_t>& code, uint32_t address);
    void jle(std::vector<uint8_t>& code, uint32_t address);

    void jcc(std::vector<uint8_t>& code, uint8_t shortCode, uint8_t nearCode, uint32_t address);

    void jmpF(std::vector<uint8_t>& code, uint32_t address);
    void jmpA(std::vector<uint8_t>& code, uint32_t address);
    void jmpR(std::vector<uint8_t>& code, Register reg);

    void leave(std::vector<uint8_t>& code);
    void ret(std::vector<uint8_t>& code);

    void opRR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register src, size_t size);
    void opRA(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register src, size_t size);
    std::vector<size_t> opRA(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, uint32_t address, size_t size);
    void opRA(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register base, int32_t ref, size_t size);
    void opRA(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register base, Register index, int scale, int32_t ref, size_t size);
    void opAR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register dist, Register src, size_t size);
    std::vector<size_t> opAR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, uint32_t address, Register src, size_t size);
    void opAR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register base, int32_t ref, Register src, size_t size);
    void opAR(std::vector<uint8_t>& code, uint8_t lCode, uint8_t eCode, Register base, Register index, int scale, int32_t ref, Register src, size_t size);
    void opImmGrp1R(std::vector<uint8_t>& code, ImmGrp1 immGrp, Register dist, int32_t imm, size_t size);
    void opImmGrp1A(std::vector<uint8_t>& code, ImmGrp1 immGrp, Register dist, int32_t imm, size_t size);
    std::vector<size_t> opImmGrp1A(std::vector<uint8_t>& code, ImmGrp1 immGrp, uint32_t address, int32_t imm, size_t size);
    void opImmGrp1A(std::vector<uint8_t>& code, ImmGrp1 immGrp, Register base, int32_t ref, int32_t imm, size_t size);
    void xidivRR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    void xidivRA(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> xidivRA(std::vector<uint8_t>& code, Register dist, uint32_t address, size_t size);
    void xidivRA(std::vector<uint8_t>& code, Register dist, Register base, int32_t ref, size_t size);
    void xidivRI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    void xidivAR(std::vector<uint8_t>& code, Register dist, Register src, size_t size);
    std::vector<size_t> xidivAR(std::vector<uint8_t>& code, uint32_t address, Register src, size_t size);
    void xidivAR(std::vector<uint8_t>& code, Register base, int32_t ref, Register src, size_t size);
    void xidivAI(std::vector<uint8_t>& code, Register dist, int32_t imm, size_t size);
    std::vector<size_t> xidivAI(std::vector<uint8_t>& code, uint32_t address, int32_t imm, size_t size);
    void xidivAI(std::vector<uint8_t>& code, Register base, int32_t ref, int32_t imm, size_t size);

    uint8_t modRM(Register dist, Register src);
    uint8_t immGrp1(ImmGrp1 immGrp);
    bool isX86Reg(Register reg);
    bool is8bitReg(Register reg);
}