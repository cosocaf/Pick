#ifndef PICKC_WINDOWS_X64_ROUTINE_H_
#define PICKC_WINDOWS_X64_ROUTINE_H_

#include "utils/binary_vec.h"
#include "bundler/function.h"
#include "type.h"

namespace pickc::windows::x64
{
  enum struct Register
  {
    /*
      アキュムレータレジスタ。
      即値系の演算命令で、命令長が短くなる場合がある。
      64bit * 64bitの結果の下位64bitが格納され、
      128bit / 64bitの被除数の下位64bitが格納され、
      除算の商が格納される。
      戻り値が格納される。
      呼び出し先で破壊してもよい。
      それ以外は汎用レジスタとして使用できる。
    */
    RAX,
    /*
      カウンタレジスタ。
      繰り返し回数の設定に使用する。
      Windowsでは一番目の整数引数を格納する。
      Linuxでは第四引数を格納する。
      呼び出し先で破壊してもよい。
      それ以外は汎用レジスタとして使用できる。
    */
    RCX,
    /*
      データレジスタ。
      64bit * 64bitの結果の上位64bitが格納され、
      128bit / 64bitの被除数の上位64bitが格納され、
      除算の剰余が格納される。
      Windowsでは二番目の整数引数を格納する。
      Linuxでは第三引数を格納する。
      呼び出し先で破壊してもよい。
      それ以外は汎用レジスタとして使用できる。
    */
    RDX,
    /*
      ベースレジスタ。
      呼び出し先によって保持される必要がある。
      x64では単なる汎用レジスタ。
    */
    RBX,
    /*
      スタックポインタレジスタ。
      呼び出し先によって保持される必要がある。
      呼び出し先で破壊してもよい。
      読み書きすることはほぼない。
    */
    RSP,
    /*
      ベースポインタレジスタ。
      スタックフレームのポインタ。
      呼び出し先によって保持される必要がある。
      読み書きすることはほぼない。
    */
    RBP,
    /*
      ソースインデックスレジスタ。
      文字列命令系でコピー元を示す場合に使用する。
      Linuxでは第二引数を格納する。
      呼び出し先によって保持される必要がある。
      それ以外は単なる汎用レジスタ。
    */
    RSI,
    /*
      デスティネーションインデックスレジスタ。
      文字列命令系でコピー先を示す場合に使用する。
      Linuxでは第一引数を格納する。
      呼び出し先によって保持される必要がある。
      それ以外は単なる汎用レジスタ。
    */
    RDI,
    /*
      x64拡張汎用レジスタ。
      REXプレフィックスが必要なため、命令長が1Byte長くなる。
      Windowsでは三番目の整数引数を格納する。
      Linuxでは第五引数を格納する。
      呼び出し先で破壊してもよい。
      引数渡しに使用される。
    */
    R8,
    /*
      x64拡張汎用レジスタ。
      REXプレフィックスが必要なため、命令長が1Byte長くなる。
      Windowsでは四番目の整数引数を格納する。
      Linuxでは第六引数を格納する。
      呼び出し先で破壊してもよい。
      引数渡しに使用される。
    */
    R9,
    /*
      x64拡張汎用レジスタ。
      REXプレフィックスが必要なため、命令長が1Byte長くなる。
      呼び出し先によって保持される必要がある。
      引数渡しに使用される。
    */
    R10,
    /*
      x64拡張汎用レジスタ。
      REXプレフィックスが必要なため、命令長が1Byte長くなる。
      引数渡しに使用される。
      呼び出し先によって保持される必要がある。
      Linuxシステムコールの呼び出しで保存されない。
    */
    R11,
    /*
      x64拡張汎用レジスタ。
      REXプレフィックスが必要なため、命令長が1Byte長くなる。
      呼び出し先によって保持される必要がある。
      引数渡しに使用される。
    */
    R12,
    /*
      x64拡張汎用レジスタ。
      REXプレフィックスが必要なため、命令長が1Byte長くなる。
      呼び出し先によって保持される必要がある。
      引数渡しに使用される。
    */
    R13,
    /*
      x64拡張汎用レジスタ。
      REXプレフィックスが必要なため、命令長が1Byte長くなる。
      呼び出し先によって保持される必要がある。
      引数渡しに使用される。
    */
    R14,
    /*
      x64拡張汎用レジスタ。
      REXプレフィックスが必要なため、命令長が1Byte長くなる。
      呼び出し先によって保持される必要がある。
      引数渡しに使用される。
    */
    R15,
    // XMM0,
    // XMM1,
    // XMM2,
    // XMM3,
    // XMM4,
    // XMM5,
    // XMM6,
    // XMM7,
    // XMM8,
    // XMM9,
    // XMM10,
    // XMM11,
    // XMM12,
    // XMM13,
    // XMM14,
    // XMM15
  };
  enum struct OperationSize
  {
    Byte,   // 8bit命令  たいていは専用の命令になる。
    Word,   // 16bit命令 66プレフィックスがつく
    DWord,  // 32bit命令
    QWord,  // 64bit命令 rex.wプレフィックスがつく
  };
  struct WindowsX64;
  struct Routine;
  class Operation
  {
  protected:
    OperationSize size;
    // rexオペランド
    static constexpr uint8_t REX = 0b0100'0000;
    // 64bitオペランド
    static constexpr uint8_t REXW = 0b0000'1000;
    // ModR/Mのregフィールド拡張
    static constexpr uint8_t REXR = 0b0000'0100;
    // SIBのindexフィールド拡張
    static constexpr uint8_t REXX = 0b0000'0010;
    // ModR/Mのr/mまたはSIBのbaseまたはオペコードのreg各フィールド拡張
    static constexpr uint8_t REXB = 0b0000'0001;
    static uint8_t modRM(Register reg, Register rm);
    // value <= INT8_MAX && value >= INT8_MIN
    static bool in8bit(int64_t value);
  public:
    Operation(OperationSize size);
    virtual ~Operation();
    // 機械語を出力する。
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) = 0;
  };
  enum struct RelocationType
  {
    _NONE,
    Symbol,
    Function,
    Text,
    JmpTo,
    Extern,
  };
  struct Relocation
  {
    RelocationType type;
    union {
      pcir::SymbolSection* symbol;
      pcir::FunctionSection* fn;
      pcir::TextSection* text;
      size_t jmpTo;
      std::string ext;
    };
    explicit Relocation(pcir::SymbolSection* symbol);
    explicit Relocation(pcir::FunctionSection* fn);
    explicit Relocation(pcir::TextSection* text);
    explicit Relocation(size_t jmpTo);
    explicit Relocation(const std::string& ext);
    Relocation(const Relocation& reloc);
    Relocation& operator=(const Relocation& reloc);
    ~Relocation();
  private:
    void clear();
  };

  enum struct OperandType
  {
    _NONE,
    Register,
    Immediate,
    Memory,
    Relocation
  };
  struct Memory
  {
    Option<Register> base;
    Option<Register> scale;
    uint8_t index: 2;
    int32_t disp;
    // routine_compilerが用いるこのメモリの使用するバイト数。
    // 機械語には関係ない。
    size_t numBytes;
    // routine::baseDeffによるdispの修正が必要ならtrue
    bool needAddressFix;
    Memory(Register base, size_t numBytes, bool needAddressFix);
    Memory(Register base, int32_t disp, size_t numBytes, bool needAddressFix);
    Memory(Register base, Register scale, uint8_t index, int32_t disp, size_t numBytes, bool needAdressFix);
  };
  struct Operand
  {
    OperandType type;
    union {
      Register reg;
      int64_t imm;
      Memory memory;
      Relocation reloc;
    };
    Operand();
    explicit Operand(Register reg);
    explicit Operand(int64_t imm);
    explicit Operand(const Memory& memory);
    explicit Operand(const Relocation& reloc);
    Operand(const Operand& operand);
    ~Operand();
    Operand& operator=(const Operand& operand);
  private:
    void clear();
  };

  class BinaryOperation : public Operation
  {
  protected:
    Operand dist;
    Operand src;
    // ADD, OR, ADC, SBB, AND, SUB, XOR, CMP用テンプレート
    BinaryVec binaryOpTemplate(Routine* routine, uint8_t ebgb, uint8_t evgv, uint8_t gbeb, uint8_t gvev, uint8_t alib, uint8_t axiz, uint8_t immop);
  public:
    BinaryOperation(OperationSize size, Operand dist, Operand src);
  };
  class AddOperation : public BinaryOperation
  {
  public:
    using BinaryOperation::BinaryOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class SubOperation : public BinaryOperation
  {
  public:
    using BinaryOperation::BinaryOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class XorOperation : public BinaryOperation
  {
  public:
    using BinaryOperation::BinaryOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class CmpOperation : public BinaryOperation
  {
  public:
    using BinaryOperation::BinaryOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };

  /**
   * Mul, IMul, Div, IDivは左辺RAX固定
   * dist = RAX op R/M を行う。
   * また、IMulに限って最大3つのオペランドを取る形式が存在するが、
   * 現状は実装を見送る。
  */
  class MulDivOperation : public Operation
  {
  protected:
    Operand right;
    BinaryVec mulDivTemplate(Routine* routine, uint8_t op);
  public:
    MulDivOperation(OperationSize size, Operand right);
  };
  class MulOperation : public MulDivOperation
  {
  public:
    using MulDivOperation::MulDivOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class IMulOperation : public MulDivOperation
  {
  public:
    using MulDivOperation::MulDivOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class DivOperation : public MulDivOperation
  {
  public:
    using MulDivOperation::MulDivOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class IDivOperation : public MulDivOperation
  {
  public:
    using MulDivOperation::MulDivOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };

  class MovOperation : public BinaryOperation
  {
  public:
    using BinaryOperation::BinaryOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };

  class NegOperation : public Operation
  {
    Operand operand;
  public:
    NegOperation(OperationSize size, Operand operand);
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  // ALを符号拡張してAXに設定する。
  class CWDOperation : public Operation
  {
  public:
    CWDOperation();
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  // AXを符号拡張してEAXに設定する。
  class CDQOperation : public Operation
  {
  public:
    CDQOperation();
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  // EAXを符号拡張してRAXに設定
  class CQOOperation : public Operation
  {
  public:
    CQOOperation();
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };

  class LeaOperation : public Operation
  {
    Register dist;
    Memory src;
  public:
    LeaOperation(OperationSize size, Register dist, Memory src);
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };

  struct Routine;
  class CallOperation : public Operation
  {
    Routine* from;
    Operand fn;
  public:
    CallOperation(Routine* from, Operand fn);
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };

  // 無条件ジャンプ
  class JmpOperation : public Operation
  {
  protected:
    // bundler上のインデックスであり、x64のインデックスでないことに注意。
    size_t to;
  public:
    JmpOperation(size_t to);
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class JeOperation : public JmpOperation
  {
  public:
    using JmpOperation::JmpOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class JneOperation : public JmpOperation
  {
  public:
    using JmpOperation::JmpOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class JgOperation : public JmpOperation
  {
  public:
    using JmpOperation::JmpOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class JgeOperation : public JmpOperation
  {
  public:
    using JmpOperation::JmpOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class JlOperation : public JmpOperation
  {
  public:
    using JmpOperation::JmpOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class JleOperation : public JmpOperation
  {
  public:
    using JmpOperation::JmpOperation;
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class ExtJmpOperation : public Operation
  {
    std::string ext;
  public:
    ExtJmpOperation(const std::string& ext);
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };

  class PushOperation : public Operation
  {
    Operand value;
  public:
    explicit PushOperation(Operand value);
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class PopOperation : public Operation
  {
    Operand dist;
  public:
    explicit PopOperation(Operand dist);
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };

  class LeaveOperation : public Operation
  {
  public:
    LeaveOperation();
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };
  class RetOperation : public Operation
  {
  public:
    RetOperation();
    virtual BinaryVec bin(WindowsX64& x64, Routine* routine) override;
  };

  enum struct RelocationPosition
  {
    Relative,
    Absolute
  };

  struct RelocationInfo
  {
    // 再配置情報を含むルーチン
    Routine* routine;
    // 再配置する要素
    Relocation reloc;
    // 再配置が必要な命令
    Operation* op;
    // 再配置をするアドレス
    size_t index;
    // 書き換えサイズ
    OperationSize size;
    // 書き換える値が相対値であるか絶対値であるか。
    RelocationPosition pos;
    RelocationInfo(Routine* routine, const Relocation& reloc, Operation* op, size_t index, OperationSize size, RelocationPosition pos);
  };

  struct Routine
  {
    bundler::Function* fn;
    std::vector<Operation*> code;
    BinaryVec nativeCode;
    // exeファイル内のこのルーチンが配置されるアドレス。
    uint64_t address;
    // codeごとのnativeCode内の開始アドレス
    // これとaddressを組み合わせて再配置を行う。
    std::unordered_map<Operation*, size_t> codeIndexes;
    // bundle命令とOperationのアドレス対応表
    std::vector<size_t> bundleIndexes;
    // 非volatileなレジスタを保存したり、RSPをアラインメントしたりした時のスタックのずれ
    int32_t baseDiff;
  };

  struct WindowsX64
  {
    // mainを呼び出すプログラム全体の初期化用関数。
    // ここでグローバル変数の初期化を行う。
    Routine* invokeMain;
    // シンボルと、それに対応するシンボルが配置されるアドレス。
    std::map<pcir::SymbolSection*, uint64_t> symbols;
    // テキストと、それに対応する文字列が配置されるアドレス。
    std::map<std::string, uint64_t> texts;
    // 関数セクションと、ネイティブ関数
    std::map<pcir::FunctionSection*, Routine*> routines;
    // extern関数の名前対応表
    std::map<pcir::FunctionSection*, std::string> externs;
    // 型テーブル
    TypeTable typeTable;
    // 再配置テーブル
    std::unordered_set<RelocationInfo*> relocs;
  };
}

#endif // PICKC_WINDOWS_X64_ROUTINE_H_